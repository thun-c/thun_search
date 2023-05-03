import sys
from typing import List, Callable, Set
from copy import deepcopy
from ._thunsearch import *
_thun = _thunsearch
__version__ = _thun.__version__


def must(func_obj):
    """Decorator for function that must be implemented

    If the function decorated by this decorator is not implemented,
    the instance cannot be created.
    """
    func_obj.__must__ = True
    return func_obj


def should(func_obj):
    """Decorator for function that sholud be implemented

    If the function decorated by this decorator is not implemented,
    an instance can be created but some functions cannot be executed.
    """
    func_obj.__should__ = True
    return func_obj


def can(func_obj):
    """Decorator for function that can be overrided

    The function qualified by this decorator is
    subject to the implementation on the
    base class side unless overridden on the subclass side.
    """
    func_obj.__can__ = True
    return func_obj


def _get_labeled_functions(obj, label: str) -> Set[str]:
    """
    Gets the functions with the specified label.
    """
    return {k for k, v in obj.__dict__.items() if hasattr(v, label)}


def _get_functions(obj):
    """
    Gets the functions.
    """
    return {k for k, v in obj.__dict__.items() if type(v).__name__ == "function"}


class BaseContextualState(_thun.ContextualState):
    """Abstract Class for Beam Search

    If this class is inherited
    and virtual functions are implemented appropriately,
    time series information-based search algorithms
    such as beam search can be applied.

    """

    def __new__(cls, *args, **kwargs):
        """Create a instance while checking whether functions labeled "must" are implemented.
        """
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
    def get_not_implemented_must_methods(cls) -> Set[str]:
        base_functions = _get_labeled_functions(__class__, "__must__")
        sub_functions = _get_functions(cls)
        return base_functions-sub_functions

    @classmethod
    def get_not_implemented_should_methods(cls) -> Set[str]:
        base_functions = _get_labeled_functions(__class__, "__should__")
        sub_functions = _get_functions(cls)
        return base_functions-sub_functions

    @classmethod
    def get_not_implemented_can_methods(cls) -> Set[str]:
        base_functions = _get_labeled_functions(__class__, "__can__")
        sub_functions = _get_functions(cls)
        return base_functions-sub_functions

    @must
    def advance(self, action: int) -> None:
        """Advance state by action

        Label
        ----------
        "must": Must be implemented.

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
        """Get legal actions

        Label
        ----------
        "must": Must be implemented.

        Parameters
        ----------
        None


        Returns
        -------
        List[int]
            actions
        """
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @must
    def is_done(self) -> bool:
        """Check task is done

        Label
        ----------
        "must": Must be implemented.

        Parameters
        ----------
        None


        Returns
        -------
        bool
            Whether the task ended successfully
        """
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @must
    def evaluate_score(self) -> float:
        """evaluate score and return

        Label
        ----------
        "must": Must be implemented.

        Parameters
        ----------
        None


        Returns
        -------
        float
            evaluated_score
        """
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @can
    def is_dead(self) -> bool:
        """Check to see if the task ended badly

        If not overridden by a subclass, always returns False.

        Label
        ----------
        "can": Can be overided.

        Parameters
        ----------
        None


        Returns
        -------
        bool
            Whether the task ended up being an anomaly
        """
        return False

    @can
    def __str__(self) -> str:
        """Convert to string

        If not overridden by a subclass,
        just connect all member variables as str.

        Label
        ----------
        "can": Can be overided.

        Returns
        -------
        str
            state information
        """
        ret_s = ""
        for key, value in self.__dict__.items():
            ret_s += f"{key}:"
            if hasattr(value, "__str__"):
                ret_s += f"{value}"
            ret_s += "\n"
        return ret_s

    @can
    def clone(self):
        """Clone object that inherit BaseClass

        If not overridden by a subclass,
        clone instance as deepcopy

        Label
        ----------
        "can": Can be overided.

        Returns
        -------
        SubClass
            cloned instance
        """
        # sub_cls is Class that inherit BaseClass
        cloned = self.sub_cls.__new__(self.sub_cls)
        # clone C++ state
        _thun.ContextualState.__init__(cloned, self)
        # clone Python state
        cloned.__dict__ = {key: deepcopy(value)
                           for key, value in self.__dict__.items()}
        return cloned


def beam_search_action(state: BaseContextualState, beam_width: int) -> List[int]:
    """Decide actions by beam search.

    Parameters
    ----------
    Subclass inheriting from BaseContextualState
        state
    int
        beam_width

    Returns
    -------
    List[int]
        List of actions to be taken until the task is completed
    """
    return _thun.beamSearchAction(state, beam_width)


def show_task(state: BaseContextualState, actions: List[int]) -> None:
    """Display the process of performing the specified actions

    Parameters
    ----------
    Subclass inheriting from BaseContextualState
        state
    List[int]
        actions

    Returns
    -------
    None
    """
    state = state.clone()
    line = "#"*30
    print(line)
    print(state)
    for action in actions:
        state.advance(action)
        print(line)
        print(state)
    print(line)


def play_task(state: BaseContextualState, ai: Callable, *args, **kwargs) -> None:
    """Display the process of doing a task with a specified AI

    Parameters
    ----------
    Subclass inheriting from BaseContextualState
        state
    Callable
        ai (Beam search, etc.)
    Args
        *args,**kargs
        args for ai

    Returns
    -------
    None
    """
    show_task(state, ai(state, *args, **kwargs))
