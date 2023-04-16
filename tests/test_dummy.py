import thun_search as thun
import pandas
from copy import copy, deepcopy
import random


class Cat(thun.Animal):
    def __init__(self) -> None:
        print("Cat.__init__")
        thun.Animal.__init__(self)
        self.voice = "meow! "
        self.turn_ = 0

    def advance(self, action):
        self.turn_ += 1

    def go(self, n_times):
        return self.voice * n_times

    def clone(self):
        # print("clone")
        # create a new object without initializing it
        cloned = Cat.__new__(Cat)
        # print("deb1")
        # print("deb1.5", cloned)
        # clone C++ state
        thun.Animal.__init__(cloned, self)
        # print("deb2")
        # print("deb2.5", cloned)
        # clone Python state
        cloned.__dict__.update(self.__dict__)
        # print("deb3")
        # print("deb3.5", cloned)
        return cloned


if __name__ == "__main__":
    print("thun\__version__", thun.__version__)
    print([key for key in thun.__dict__.keys() if "__" != key[:2]])
    cat = Cat()
    cat.voice = "org_mew! "
    print(thun.call_go(cat))
    cat2 = cat.clone()
    cat2.voice = "cat2 "
    print("cat2.turn_", cat2.turn_)
    print(cat2.go(2))
    # cat.cloneAdvancedVoid(0)
    cat3 = cat2.cloneAdvanced(0)
    print("cat3.turn_", cat3.turn_)
    print(cat3.go(2))
