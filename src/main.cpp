#include <pybind11/pybind11.h>
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

class Animal : public std::enable_shared_from_this<Animal>
{
public:
    std::shared_ptr<Animal> parent_ = nullptr;
    virtual ~Animal() {}
    virtual std::string go(int n_times) = 0;
    virtual std::shared_ptr<Animal> clone() const = 0;
    virtual void advance(const int action) = 0;

    std::shared_ptr<Animal> cloneAdvanced(int action)
    {
        auto clone = this->clone();
        clone->advance(action);
        clone->parent_ = shared_from_this();
        return clone;
    }

    void cloneAdvancedVoid(int action)
    {
        cout << "cloneAdvancedVoid" << __LINE__ << endl;
        auto clone = this->clone();
        cout << "cloneAdvancedVoid" << __LINE__ << endl;
        clone->advance(action);
        cout << "cloneAdvancedVoid" << __LINE__ << endl;
        // clone->parent_ = std::shared_ptr<Animal>(this);
        // cout << "cloneAdvancedVoid" << __LINE__ << endl;
    }
};

std::string call_go(std::shared_ptr<Animal> animal)
{
    return animal->go(3);
}

class PyAnimal : public Animal
{
public:
    /* Inherit the constructors */
    using Animal::Animal;
    PyAnimal(const Animal &animal) : Animal(animal) {}
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
    std::shared_ptr<Animal> clone() const override
    {
        auto self = py::cast(this);
        auto cloned = self.attr("clone")();

        auto keep_python_state_alive = std::make_shared<py::object>(cloned);
        auto ptr = cloned.cast<PyAnimal *>();

        // aliasing shared_ptr: points to `A_trampoline* ptr` but refcounts the Python object
        return std::shared_ptr<Animal>(keep_python_state_alive, ptr);
    }

    void advance(int action) override
    {
        cout << "PyAnimal" << __LINE__ << endl;
        PYBIND11_OVERRIDE_PURE(/* Return type */ void, /* Parent class */ Animal, /* Name of function */ advance, /* args */ action);
    }
};

PYBIND11_MODULE(thun_search, m)
{
    py::class_<Animal, PyAnimal, std::shared_ptr<Animal>>(m, "Animal")
        .def(py::init<>())
        .def(py::init<const Animal &>())
        .def("go", &Animal::go)
        .def("advance", &Animal::advance)
        .def("cloneAdvanced", &Animal::cloneAdvanced)
        .def("cloneAdvancedVoid", &Animal::cloneAdvancedVoid)
        .def("clone", &Animal::clone);

    m.def("call_go", &call_go);

#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
