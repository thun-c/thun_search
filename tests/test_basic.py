import thun_search as thun
import pandas
from copy import copy, deepcopy
import random


def test_main():
    assert thun.add(1, 2) == 3
    assert thun.subtract(1, 2) == -1


class Cat(thun.Animal):

    def go(self, n_times):
        return "meow! " * n_times


class Coord:
    def __init__(self, y=0, x=0) -> None:
        self.y_ = y
        self.x_ = x
        pass


class MazeState(thun.State):
    dy = [1, -1, 0, 0]
    dx = [0, 0, 1, 1]
    H = 3
    W = 4
    END_TURN = 4
    INF = 1000000000

    def __init__(self, seed=None) -> None:
        # thun.State.__init__(self)
        super().__init__()
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

    def isDone(self):
        return self.turn_ == MazeState.END_TURN

    def evaluateScore(self):
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

    def legalActions(self):
        actions = []
        for action in range(4):
            ty = self.character_.y_+MazeState.dy[action]
            tx = self.character_.x_+MazeState.dx[action]
            if ty >= 0 and ty < MazeState.H and tx >= 0 and tx < MazeState.W:
                actions.append(action)
        return actions

    def clone(self):
        return copy(self)

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
    print("thun\__version__", thun.__version__)
    print([key for key in thun.__dict__.keys() if "__" != key[:2]])
    test_main()
    cat = Cat()
    print(thun.call_go(cat))
    state = MazeState(0)
    print("thun.State", [
          key for key in thun.State.__dict__.keys() if "__" != key[:2]])
    print("MazeState", [
          key for key in MazeState.__dict__.keys() if "__" != key[:2]])

    print("state", [
          key for key in state.__dict__.keys() if True or "__" != key[:2]])

    print("state\n###########\n", state)
    # actions = thun.randomAction(state)
    state.advance(1)
    state.evaluateScore()
    print("state\n###########\n", state)
    print(thun.getEvaluatedScore(state))
