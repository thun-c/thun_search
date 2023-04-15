#include <pybind11/pybind11.h>
#include <random>
#include <memory>
#include <algorithm>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)
std::mt19937 mt_for_action(0); // 行動選択用の乱数生成器を初期化

int add(int i, int j)
{
    return i + j;
}

class Animal
{
public:
    std::shared_ptr<Animal> parent = nullptr;
    virtual ~Animal() {}
    virtual std::string go(int n_times) = 0;
    // virtual std::string go(int n_times) { return "not orverrided!!!!"; }
};

// std::string call_go(Animal *animal)
std::string call_go(std::shared_ptr<Animal> animal)
{
    return animal->go(3);
}

class PyAnimal : public Animal
{
public:
    /* Inherit the constructors */
    using Animal::Animal;

    /* Trampoline (need one for each virtual function) */
    std::string go(int n_times) override
    {
        PYBIND11_OVERRIDE_PURE(
            std::string, /* Return type */
            Animal,      /* Parent class */
            go,          /* Name of function in C++ (must match Python name) */
            n_times      /* Argument(s) */
        );
    }
};

class State
{
private:
public:
    std::shared_ptr<State> parent_ = nullptr;
    double evaluated_score_ = 0; // 探索上で評価したスコア
    int last_action_ = -1;       // 直前に選択した行動
    virtual ~State() {}

    // ゲームの終了判定
    virtual bool isDone() = 0;

    // 探索用の盤面評価をする
    virtual void evaluateScore() = 0;

    virtual void setEvaluateScore(double evaluated_score)
    {
        this->evaluated_score_ = evaluated_score;
    }

    // 指定したactionでゲームを1ターン進める
    virtual void advance(const int action) = 0;

    // 現在の状況でプレイヤーが可能な行動を全て取得する
    virtual std::vector<int> legalActions() = 0;

    // インスタンスをコピーする。
    virtual std::shared_ptr<State> clone() = 0;
};

// 探索時のソート用に評価を比較する
bool operator<(const State &state_1, const State &state_2)
{
    return state_1.evaluated_score_ < state_2.evaluated_score_;
}
bool operator<(const std::shared_ptr<State> &state_1, const std::shared_ptr<State> &state_2)
{
    return state_1->evaluated_score_ < state_2->evaluated_score_;
}

class PyState : public State
{
public:
    /* Inherit the constructors */
    using State::State;

    bool isDone() override
    {
        PYBIND11_OVERRIDE_PURE(/* Return type */ bool, /* Parent class */ State, /* Name of function */ isDone);
    }

    void evaluateScore() override
    {
        PYBIND11_OVERRIDE_PURE(/* Return type */ void, /* Parent class */ State, /* Name of function */ evaluateScore);
    }

    void advance(int action) override
    {
        PYBIND11_OVERRIDE_PURE(/* Return type */ void, /* Parent class */ State, /* Name of function */ advance, /* args */ action);
    }

    std::vector<int> legalActions() override
    {
        PYBIND11_OVERRIDE_PURE(/* Return type */ std::vector<int>, /* Parent class */ State, /* Name of function */ legalActions);
    }

    void setEvaluateScore(double evaluated_score) override
    {
        PYBIND11_OVERRIDE(/* Return type */ void, /* Parent class */ State, /* Name of function */ setEvaluateScore, /* args */ evaluated_score);
    }

    std::shared_ptr<State> clone() override
    {
        PYBIND11_OVERRIDE_PURE(/* Return type */ std::shared_ptr<State>, /* Parent class */ State, /* Name of function */ clone);
    }
};

double getEvaluatedScore(State *state)
{
    return state->evaluated_score_;
}

// これはpythonに渡さない
std::shared_ptr<State> cloneAdvanced(std::shared_ptr<State> state, int action)
{
    auto clone = state->clone();
    clone->advance(action);
    clone->parent_ = state;
    clone->last_action_ = action;
    return clone;
}

// ランダムに行動を決定する
std::vector<int> randomAction(std::shared_ptr<State> state)
{
    while (!state->isDone())
    {
        auto legal_actions = state->legalActions();
        int action = legal_actions[mt_for_action() % (legal_actions.size())];
        state = cloneAdvanced(state, action);
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
namespace py = pybind11;

PYBIND11_MODULE(thun_search, m)
{
    m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------

        .. currentmodule:: thun_search

        .. autosummary::
           :toctree: _generate

           add
           subtract
    )pbdoc";

    m.def("add", &add, R"pbdoc(
        Add two numbers

        Some other explanation about the add function.
    )pbdoc");

    m.def(
        "subtract", [](int i, int j)
        { return i - j; },
        R"pbdoc(
        Subtract two numbers

        Some other explanation about the subtract function.
    )pbdoc");

    py::class_<Animal, PyAnimal, std::shared_ptr<Animal>>(m, "Animal")
        .def(py::init<>())
        .def("go", &Animal::go);

    m.def("call_go", &call_go);

    py::class_<State, PyState>(m, "State")
        .def(py::init<>())
        .def("isDone", &State::isDone)
        .def("evaluateScore", &State::evaluateScore)
        .def("advance", &State::advance)
        .def("legalActions", &State::legalActions)
        .def("setEvaluateScore", &State::setEvaluateScore)
        .def("clone", &State::clone);

    m.def("getEvaluatedScore", &getEvaluatedScore);
    m.def("randomAction", &randomAction);

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
