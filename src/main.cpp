#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <random>
#include <memory>
#include <algorithm>
#include <iostream>
#include <queue>
namespace py = pybind11;
#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)
std::mt19937 mt_for_action(0); // 行動選択用の乱数生成器を初期化
using std::cerr;
using std::cout;
using std::endl;

class ContextualState : public std::enable_shared_from_this<ContextualState>
{
public:
    std::shared_ptr<ContextualState> parent_ = nullptr;
    double evaluated_score_ = 0; // 探索上で評価したスコア
    int last_action_ = -1;       // 直前に選択した行動

    virtual ~ContextualState() {}
    virtual std::shared_ptr<ContextualState> clone() const = 0;
    virtual void advance(const int action) = 0;
    virtual std::vector<int> _legal_actions() = 0;

    // ゲームの終了判定
    virtual bool is_done() = 0;

    virtual bool is_dead() = 0;

    // 探索用の盤面評価をする
    virtual double evaluate_score() = 0;

    std::shared_ptr<ContextualState> cloneAdvanced(int action)
    {
        auto clone = this->clone();
        auto actions = clone->_legal_actions();
        clone->advance(action);
        clone->parent_ = shared_from_this();
        clone->last_action_ = action;
        return clone;
    }
};

// 探索時のソート用に評価を比較する
bool operator<(const ContextualState &state_1, const ContextualState &state_2)
{
    return state_1.evaluated_score_ < state_2.evaluated_score_;
}
bool operator<(const std::shared_ptr<ContextualState> &state_1, const std::shared_ptr<ContextualState> &state_2)
{
    return state_1->evaluated_score_ < state_2->evaluated_score_;
}

class PyContextualState : public ContextualState
{
public:
    /* Inherit the constructors */
    using ContextualState::ContextualState;
    PyContextualState(const ContextualState &state) : ContextualState(state) {}
    /* Trampoline (need one for each virtual function) */
    std::shared_ptr<ContextualState> clone() const override
    {
        auto self = py::cast(this);
        auto cloned = self.attr("clone")();

        auto keep_python_state_alive = std::make_shared<py::object>(cloned);
        auto ptr = cloned.cast<PyContextualState *>();

        // aliasing shared_ptr: points to `A_trampoline* ptr` but refcounts the Python object
        return std::shared_ptr<ContextualState>(keep_python_state_alive, ptr);
    }

    void advance(int action) override
    {
        PYBIND11_OVERRIDE_PURE(/* Return type */ void, /* Parent class */ ContextualState, /* Name of function */ advance, /* args */ action);
    }
    std::vector<int> _legal_actions() override
    {
        PYBIND11_OVERRIDE_PURE(/* Return type */ std::vector<int>, /* Parent class */ ContextualState, /* Name of function */ _legal_actions);
    }

    bool is_dead() override
    {
        PYBIND11_OVERRIDE_PURE(/* Return type */ bool, /* Parent class */ ContextualState, /* Name of function */ is_dead);
    }

    bool is_done() override
    {
        PYBIND11_OVERRIDE_PURE(/* Return type */ bool, /* Parent class */ ContextualState, /* Name of function */ is_done);
    }

    double evaluate_score() override
    {
        PYBIND11_OVERRIDE_PURE(/* Return type */ double, /* Parent class */ ContextualState, /* Name of function */ evaluate_score);
    }
};

// ランダムに行動を決定する
std::vector<int> randomAction(std::shared_ptr<ContextualState> state)
{
    using namespace std;
    while (!state->is_done() && !state->is_dead())
    {
        auto legal_actions = state->_legal_actions();

        int action = legal_actions[mt_for_action() % (legal_actions.size())];
        state = state->cloneAdvanced(action);
    }
    std::vector<int> actions{};
    while (state->parent_ != nullptr)
    {
        actions.emplace_back(state->last_action_);
        state = state->parent_;
    }
    std::reverse(actions.begin(), actions.end());
    return actions;
}

// ビーム幅を指定してビームサーチで行動を決定する
std::vector<int> beamSearchAction(std::shared_ptr<ContextualState> state, const int beam_width)
{
    using ContextualStatePtr = std::shared_ptr<ContextualState>;
    std::priority_queue<ContextualStatePtr, std::vector<ContextualStatePtr>, std::greater<ContextualStatePtr>> now_beam;
    std::shared_ptr<ContextualState> best_state = nullptr;

    now_beam.emplace(state);
    for (int t = 0;; t++)
    {
        std::priority_queue<ContextualStatePtr, std::vector<ContextualStatePtr>, std::greater<ContextualStatePtr>> next_beam;

        for (int i = 0; i < beam_width; i++)
        {
            if (now_beam.empty())
                break;
            std::shared_ptr<ContextualState> now_state = now_beam.top();
            // cout << "t " << t << "\tnow_score:" << now_state->evaluated_score_ << endl;

            now_beam.pop();
            auto legal_actions = now_state->_legal_actions();
            for (const auto &action : legal_actions)
            {
                auto next_state = now_state->cloneAdvanced(action);
                if (next_state->is_dead())
                {
                    continue;
                }
                next_state->evaluated_score_ = next_state->evaluate_score();

                if (next_beam.size() >= beam_width && next_beam.top()->evaluated_score_ >= next_state->evaluated_score_)
                {
                    continue;
                }

                assert(next_state->parent_ != nullptr);

                if (next_state->is_done())
                {
                    if (best_state == nullptr || next_state > best_state)
                    {
                        best_state = next_state;
                    }
                    continue;
                }

                // cout << "next_state " << next_state->evaluated_score_ << endl;
                next_beam.emplace(next_state);
                if (next_beam.size() > beam_width)
                {
                    next_beam.pop();
                }
            }
        }

        if (best_state != nullptr)
        {
            break;
        }
        now_beam = next_beam;
    }

    std::vector<int> actions{};
    while (best_state->parent_ != nullptr)
    {
        actions.emplace_back(best_state->last_action_);
        best_state = best_state->parent_;
    }
    std::reverse(actions.begin(), actions.end());
    return actions;
}

PYBIND11_MODULE(_thunsearch, m)
{
    py::bind_vector<std::vector<int>>(m, "VectorInt");

    py::class_<ContextualState, PyContextualState, std::shared_ptr<ContextualState>>(m, "ContextualState")
        .def(py::init<>())
        .def(py::init<const ContextualState &>())
        .def("is_done", &ContextualState::is_done)
        .def("is_dead", &ContextualState::is_dead)
        .def("evaluate_score", &ContextualState::evaluate_score)
        .def("advance", &ContextualState::advance)
        .def("cloneAdvanced", &ContextualState::cloneAdvanced)
        .def("clone", &ContextualState::clone)
        .def("_legal_actions", &ContextualState::_legal_actions);

    m.def("randomAction", &randomAction, R"mydelimiter(
        get futuer actions by random

        Parameters
        ----------

        action: int
    )mydelimiter");
    m.def("beamSearchAction", &beamSearchAction);

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
