import sys
import random
from copy import deepcopy
# import thunsearch._thunsearch as thun
import thunsearch as thun
from abc import abstractmethod
from typing import List, Callable
print("thun", [key for key in thun.__dict__.keys()])
print("thun._thunsearch", [key for key in thun._thunsearch.__dict__.keys()])


def clone_child(child, instance):

    cloned = child.__new__(child)
    # clone C++ state
    thun.State.__init__(cloned, instance)
    # clone Python state
    cloned.__dict__ = {key: deepcopy(value)
                       for key, value in instance.__dict__.items()}
    return cloned


class BaseState(thun.State):
    @abstractmethod
    def advance(self, action):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @abstractmethod
    def legal_actions(self) -> List[int]:
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    def _legal_actions(self) -> thun.VectorInt:
        return thun.VectorInt(self.legal_actions())

    @abstractmethod
    def is_done(self):
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


class Coord:
    def __init__(self, y=0, x=0) -> None:
        self.y_ = y
        self.x_ = x
        pass


class MazeState(BaseState):
    dy = [0, 0, 1, -1]  # 右、左、下、上への移動方向のy成分
    dx = [1, -1, 0, 0]  # 右、左、下、上への移動方向のx成分
    H = 3
    W = 4
    END_TURN = 4
    INF = 1000000000

    def __init__(self, seed=None) -> None:
        thun.State.__init__(self)
        # super().__init__()
        self.turn_ = 0
        self.points_ = [[0 for w in range(MazeState.W)]
                        for h in range(MazeState.H)]
        self.character_ = Coord(0, 0)
        self.game_score_ = 0
        if seed is not None:
            random.seed(seed)
            self.character_.y_ = random.randrange(MazeState.H)
            self.character_.x_ = random.randrange(MazeState.W)
            for y in range(MazeState.H):
                for x in range(MazeState.W):
                    if (y, x) == (self.character_.y_, self.character_.x_):
                        continue
                    self.points_[y][x] = random.randrange(10)

    def is_done(self):
        return self.turn_ == MazeState.END_TURN

    def evaluate_score(self):
        super().setEvaluateScore(self.game_score_)
        return

    def advance(self, action):
        self.character_.y_ += MazeState.dy[action]
        self.character_.x_ += MazeState.dx[action]
        if self.points_[self.character_.y_][self.character_.x_] > 0:
            self.game_score_ += self.points_[
                self.character_.y_][self.character_.x_]
            self.points_[self.character_.y_][self.character_.x_] = 0
        self.turn_ += 1
        return

    def legal_actions(self):
        actions = []
        for action in range(4):
            ty = self.character_.y_+MazeState.dy[action]
            tx = self.character_.x_+MazeState.dx[action]
            if ty >= 0 and ty < MazeState.H and tx >= 0 and tx < MazeState.W:
                actions.append(action)
        return actions

    def clone(self):
        return clone_child(__class__, self)

    def __str__(self):
        ss = ""
        ss += f"turn:\t{self.turn_}\n"
        ss += f"game_score_:\t{self.game_score_}\n"
        for h in range(MazeState.H):
            for w in range(MazeState.W):
                if (self.character_.y_ == h and self.character_.x_ == w):
                    ss += "@"
                elif self.points_[h][w] > 0:
                    ss += str(self.points_[h][w])
                else:
                    ss += "."
            ss += '\n'
        return ss


if __name__ == "__main__":
    print("thun.__version__", thun.__version__)
    # print([key for key in thun.__dict__.keys() if "__" != key[:2]])
    # state = MazeState(0)
    # print("thun.State", [
    #       key for key in thun.State.__dict__.keys() if "__" != key[:2]])
    # print("MazeState", [
    #       key for key in MazeState.__dict__.keys() if "__" != key[:2]])

    # print("state", [
    #       key for key in state.__dict__.keys() if True or "__" != key[:2]])

    state = MazeState(0)
    # print("state\n###########\n", state)
    # state2 = state.cloneAdvanced(1)
    # print("state2\n###########\n", state2)
    # print("state\n###########\n", state)
    # print(thun.randomAction(state))
    play_game(state, thun.randomAction)
