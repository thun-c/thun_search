import sys
from typing import List, Callable
from copy import deepcopy
from ._thunsearch import *
_thun = _thunsearch
__version__ = _thun.__version__


def must(func_obj):
    func_obj.__must__ = True
    return func_obj


def should(func_obj):
    func_obj.__should__ = True
    return func_obj


def can(func_obj):
    func_obj.__can__ = True
    return func_obj


def _get_labeled_functions(obj, label: str):
    return {k for k, v in obj.__dict__.items() if hasattr(v, label)}


def _get_functions(obj):
    return {k for k, v in obj.__dict__.items() if type(v).__name__ == "function"}


class BaseState(_thun.State):
    """Abstract Class for Beam Search

    If this class is inherited 
    and virtual functions are implemented appropriately,
    time series information-based search algorithms
    such as beam search can be applied.

    """

    def __new__(cls, *args, **kwargs):
        must_functions = (_get_labeled_functions(__class__, "__must__"))
        sub_functions = (_get_functions(cls))

        not_implemented_musts = must_functions-sub_functions
        if len(not_implemented_musts) > 0:
            joined_musts = " , ".join([k for k in list(not_implemented_musts)])
            raise NotImplementedError(
                f"must functions are not implemented. [{joined_musts}] ")
        return super(__class__, cls).__new__(cls, *args, **kwargs)

    def _legal_actions(self) -> _thun.VectorInt:
        return _thun.VectorInt(self.legal_actions())

    def __init_subclass__(cls, /,  **kwargs):
        super().__init_subclass__(**kwargs)
        cls.sub_cls = cls

    @classmethod
    def get_not_implemented_should_methods(cls):
        base_functions = _get_labeled_functions(__class__, "__should__")
        sub_functions = _get_functions(cls)
        return base_functions-sub_functions

    @classmethod
    def get_not_implemented_can_methods(cls):
        base_functions = _get_labeled_functions(__class__, "__can__")
        sub_functions = _get_functions(cls)
        return base_functions-sub_functions

    @must
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

    @must
    def legal_actions(self) -> List[int]:
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @must
    def is_done(self):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @must
    def is_dead(self):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @must
    def evaluate_score(self) -> float:
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @should
    def __str__(self):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @can
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
