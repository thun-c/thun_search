import sys
from abc import abstractmethod
from typing import List, Callable
from copy import deepcopy
from ._thunsearch import *
_thun = _thunsearch
__version__ = _thun.__version__


class BaseState(_thun.State):
    """Abstract Class for Beam Search

    If this class is inherited 
    and virtual functions are implemented appropriately,
    time series information-based search algorithms
    such as beam search can be applied.

    Why neither A nor B is used?


    """

    def advance(self, action: int) -> None:
        """Advance state by action

        Note
        ----------
        Implementation is required for the following functions
        - show_game
        - play_game
        - randomAction
        - beamSearchAction

        Parameters
        ----------
        action: int


        Returns
        -------
        None
        """
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
    def evaluate_score(self) -> float:
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    def __init_subclass__(cls, /,  **kwargs):
        super().__init_subclass__(**kwargs)
        cls.sub_cls = cls

    def clone(self):
        """Clone object that inherit BaseClass

        clone instance as deepcopy

        Returns
        -------
        SubClass
            cloned instance
        """
        # sub_cls is Class that inherit BaseClass
        cloned = self.sub_cls.__new__(self.sub_cls)
        # clone C++ state
        _thun.State.__init__(cloned, self)
        # clone Python state
        cloned.__dict__ = {key: deepcopy(value)
                           for key, value in self.__dict__.items()}
        return cloned

    @abstractmethod
    def __str__(self):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")


def beam_search_action(state: BaseState, beam_width: int):
    return _thun.beamSearchAction(state, beam_width)


def clone_inherited_instance(sub_class: type, instance: object) -> object:
    """Clone object that inherit BaseClass

    clone instance as deepcopy

    Parameters
    ----------
    sub_class: type
        Type of instance.
        This is not a true BaseState, but rather an inherited BaseState.
    instance : SubClass
        instance of SubClass."SubClass" is specified in the first argument.

    Returns
    -------
    SubClass
        cloned instance
    """
    cloned = sub_class.__new__(sub_class)
    # clone C++ state
    _thun.State.__init__(cloned, instance)
    # clone Python state
    cloned.__dict__ = {key: deepcopy(value)
                       for key, value in instance.__dict__.items()}
    return cloned


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
