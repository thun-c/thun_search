import sys
import random
from copy import deepcopy
import time
# import thunsearch._thunsearch as thun
import thunsearch as thun
from typing import List, Callable
print("thun", [key for key in thun.__dict__.keys()])
print("thun._thunsearch", [key for key in thun._thunsearch.__dict__.keys()])


class Coord:
    def __init__(self, y=0, x=0) -> None:
        self.y_ = y
        self.x_ = x
        pass

    def __eq__(self, other):
        if not isinstance(other, Coord):
            return NotImplemented
        return (self.y_, self.x_) == (other.y_, other.x_)

    def __ne__(self, other):
        return not self.__eq__(other)


class MazeState(thun.BaseState):
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
        self.trap_ = Coord(0, 0)
        self.game_score_ = 0
        if seed is not None:
            random.seed(seed)
            self.character_.y_ = random.randrange(MazeState.H)
            self.character_.x_ = random.randrange(MazeState.W)
            while self.character_ == self.trap_:
                self.trap_.y_ = random.randrange(MazeState.H)
                self.trap_.x_ = random.randrange(MazeState.W)

            for y in range(MazeState.H):
                for x in range(MazeState.W):
                    if (y, x) == (self.character_.y_, self.character_.x_):
                        continue

                    if (y, x) == (self.trap_.y_, self.trap_.x_):
                        continue
                    self.points_[y][x] = random.randrange(10)

    def is_done(self):
        return self.turn_ == MazeState.END_TURN

    def is_dead(self):
        return self.trap_ == self.character_

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
        return thun.clone_child(__class__, self)

    def __str__(self):
        ss = ""
        ss += f"turn:\t{self.turn_}\n"
        ss += f"game_score_:\t{self.game_score_}\n"
        for h in range(MazeState.H):
            for w in range(MazeState.W):
                if (self.character_ == Coord(h, w)):
                    ss += "@"
                elif (self.trap_ == Coord(h, w)):
                    ss += "X"
                elif self.points_[h][w] > 0:
                    ss += str(self.points_[h][w])
                else:
                    ss += "."
            ss += '\n'
        return ss


def test_ai_performance(name_ai, game_number, per_game_number):
    name, ai = name_ai
    diff_sum = 0
    score_sum = 0
    for i in range(game_number):
        state = MazeState(i)
        start_time = time.time()
        actions = None
        for j in range(per_game_number):
            tmp_actions = ai(state)
            if j == 0:
                actions = tmp_actions
        diff = time.time()-start_time
        diff_sum += diff
        for action in actions:
            state.advance(action)
            state.evaluate_score()
        score_sum += state.game_score_
    time_mean = diff_sum*1000//game_number
    score_mean = round(score_sum/game_number, 2)
    print(f"\"{name}\" score:{score_mean}\ttime:{time_mean}")


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
    # play_game(state, thun.randomAction)
    # play_game(state, beam_py_function(beam_width=100))
    game_number = 10
    per_game_number = 10
    numbers = game_number, per_game_number

    def get_name_beam(beamwidth):
        return (
            f"beam {beamwidth}",
            thun.beam_py_function(beam_width=beamwidth)
        )
    test_ai_performance(get_name_beam(2), *numbers)
    test_ai_performance(get_name_beam(4), *numbers)
    test_ai_performance(get_name_beam(8), *numbers)
    test_ai_performance(get_name_beam(16), *numbers)
    test_ai_performance(get_name_beam(32), *numbers)
