#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <random>
#include <memory>
#include <algorithm>
#include <iostream>
namespace py = pybind11;
#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)
std::mt19937 mt_for_action(0); // 行動選択用の乱数生成器を初期化
using std::cerr;
using std::cout;
using std::endl;

class State : public std::enable_shared_from_this<State>
{
public:
    std::shared_ptr<State> parent_ = nullptr;
    double evaluated_score_ = 0; // 探索上で評価したスコア
    int last_action_ = -1;       // 直前に選択した行動

    virtual ~State() {}
    virtual std::shared_ptr<State> clone() const = 0;
    virtual void advance(const int action) = 0;
    virtual std::vector<int> legalActionsCpp() = 0;

    // ゲームの終了判定
    virtual bool isDone() = 0;

    // 探索用の盤面評価をする
    virtual void evaluateScore() = 0;

    virtual void setEvaluateScore(double evaluated_score)
    {
        this->evaluated_score_ = evaluated_score;
    }

    std::shared_ptr<State> cloneAdvanced(int action)
    {
        auto clone = this->clone();
        auto actions = clone->legalActionsCpp();
        clone->advance(action);
        clone->parent_ = shared_from_this();
        clone->last_action_ = action;
        return clone;
    }
};

class PyState : public State
{
public:
    /* Inherit the constructors */
    using State::State;
    PyState(const State &state) : State(state) {}
    /* Trampoline (need one for each virtual function) */
    std::shared_ptr<State> clone() const override
    {
        auto self = py::cast(this);
        auto cloned = self.attr("clone")();

        auto keep_python_state_alive = std::make_shared<py::object>(cloned);
        auto ptr = cloned.cast<PyState *>();

        // aliasing shared_ptr: points to `A_trampoline* ptr` but refcounts the Python object
        return std::shared_ptr<State>(keep_python_state_alive, ptr);
    }

    void advance(int action) override
    {
        PYBIND11_OVERRIDE_PURE(/* Return type */ void, /* Parent class */ State, /* Name of function */ advance, /* args */ action);
    }
    std::vector<int> legalActionsCpp() override
    {
        PYBIND11_OVERRIDE_PURE(/* Return type */ std::vector<int>, /* Parent class */ State, /* Name of function */ legalActionsCpp);
    }
    bool isDone() override
    {
        PYBIND11_OVERRIDE_PURE(/* Return type */ bool, /* Parent class */ State, /* Name of function */ isDone);
    }

    void evaluateScore() override
    {
        PYBIND11_OVERRIDE_PURE(/* Return type */ void, /* Parent class */ State, /* Name of function */ evaluateScore);
    }
    void setEvaluateScore(double evaluated_score) override
    {
        PYBIND11_OVERRIDE(/* Return type */ void, /* Parent class */ State, /* Name of function */ setEvaluateScore, /* args */ evaluated_score);
    }
};

// ランダムに行動を決定する
std::vector<int> randomAction(std::shared_ptr<State> state)
{
    using namespace std;
    cout << "C: randomAction " << __LINE__ << endl;
    while (!state->isDone())
    {
        auto legal_actions = state->legalActionsCpp();

        int action = legal_actions[mt_for_action() % (legal_actions.size())];
        cout << "C: randomAction " << __LINE__ << " action" << action << endl;
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

PYBIND11_MODULE(_thun_search, m)
{
    py::bind_vector<std::vector<int>>(m, "VectorInt");

    py::class_<State, PyState, std::shared_ptr<State>>(m, "State")
        .def(py::init<>())
        .def(py::init<const State &>())
        .def("isDone", &State::isDone)
        .def("evaluateScore", &State::evaluateScore)
        .def("setEvaluateScore", &State::setEvaluateScore)
        .def("advance", &State::advance)
        .def("cloneAdvanced", &State::cloneAdvanced)
        .def("clone", &State::clone)
        .def("legalActionsCpp", &State::legalActionsCpp);

    m.def("randomAction", &randomAction);

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
