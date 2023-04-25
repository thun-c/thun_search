import sys
from abc import abstractmethod
from typing import List, Callable
from copy import deepcopy
from ._thunsearch import *
_thun = _thunsearch
__version__ = _thun.__version__


def clone_child(child, instance):

    cloned = child.__new__(child)
    # clone C++ state
    _thun.State.__init__(cloned, instance)
    # clone Python state
    cloned.__dict__ = {key: deepcopy(value)
                       for key, value in instance.__dict__.items()}
    return cloned


def beam_py_function(beam_width) -> Callable:
    return lambda state: _thun.beamSearchAction(state, beam_width)


class BaseState(_thun.State):
    @abstractmethod
    def advance(self, action):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @abstractmethod
    def legal_actions(self) -> List[int]:
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    def _legal_actions(self) -> _thun.VectorInt:
        return _thun.VectorInt(self.legal_actions())

    @abstractmethod
    def is_done(self):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @abstractmethod
    def is_dead(self):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @abstractmethod
    def evaluate_score(self):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @abstractmethod
    def clone(self):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented.\n"
            f"It is recommended to write "
            f"\"return clone_child(__class__, self)\"")

    @abstractmethod
    def __str__(self):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")


def show_game(state: BaseState, actions: List[int]):
    state = state.clone()
    line = "#"*30
    print(line)
    print(state)
    for action in actions:
        state.advance(action)
        print(line)
        print(state)
    print(line)


def play_game(state: BaseState, ai: Callable):
    show_game(state, ai(state))
