import sys
import random
from copy import copy, deepcopy
import pandas
# import thun_search._thun_search as thun
import thun_search as thun
from abc import ABCMeta, abstractmethod
from typing import List
print([key for key in thun.__dict__.keys()])
print([key for key in thun._thun_search.__dict__.keys()])
# print([key for key in thun.__dict__.keys() if "__" != key[:2]])
# print([key for key in thun._thun_search.__dict__.keys() if "__" != key[:2]])


def clone_child(child, instance):

    cloned = child.__new__(child)
    # clone C++ state
    thun.State.__init__(cloned, instance)
    # clone Python state
    cloned.__dict__ = {key: deepcopy(value)
                       for key, value in instance.__dict__.items()}
    print(type(child))
    print(type(cloned))
    return cloned


class BaseState(thun.State):
    @abstractmethod
    def advance(self, action):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @abstractmethod
    def legalActionsPy(self) -> List[int]:
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    def legalActions(self) -> thun.VectorInt:
        return thun.VectorInt(self.legalActionsPy())

    @abstractmethod
    def advance(self, action):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @abstractmethod
    def clone(self):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")


class Cat(BaseState):
    def __init__(self) -> None:
        print("Cat.__init__")
        thun.State.__init__(self)
        self.turn_ = 0
        self.ar_ = []
        self.ar2_ = []

    def advance(self, action: List[int]):
        self.turn_ += 1
        self.ar_.append(self.turn_)
        self.ar2_.append([self.turn_*10, self.turn_*20])

    def legalActionsPy(self) -> List[int]:
        actions = [20, 30]
        actions.append(10)
        return actions

    def clone(self):
        return clone_child(__class__, self)


def print_dict(name, val):
    ret_dict = {key: value for key, value in val.__dict__.items()
                if "__" != key[:2]}
    print(name, ret_dict)


if __name__ == "__main__":
    print("thun\__version__", thun.__version__)
    print([key for key in thun.__dict__.keys() if "__" != key[:2]])

    print("actions", [
        key for key in thun.VectorInt.__dict__.keys() if "__" != key[:2]])
    print("type(Cat) ", type(Cat))
    cat = Cat()
    cat2 = cat.clone()
    print("cat2.turn_", cat2.turn_)
    # cat.cloneAdvancedVoid(0)
    # cat3 = cat2.clone()
    cat3 = cat2.cloneAdvanced(0)
    print("cat3.turn_", cat3.turn_)
    print_dict("cat", cat)
    print_dict("cat2", cat2)
    print_dict("cat3", cat3)
    cat4 = cat3.clone()
    cat4.ar2_[0][1] = 100
    print_dict("cat3", cat3)
    print_dict("cat4", cat4)
