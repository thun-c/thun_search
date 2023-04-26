import sys
import random
from copy import deepcopy
import time
from abc import abstractmethod
from typing import List, Callable
import heapq


def clone_inherited_instance(child, instance):

    cloned = child.__new__(child)
    cloned.__dict__ = {key: deepcopy(value)
                       for key, value in instance.__dict__.items()}
    return cloned


class BaseState():
    def __init__(self) -> None:
        self.parent_ = None
        self.evaluated_score_ = 0
        self.last_action_ = -1
        pass

    @abstractmethod
    def advance(self, action):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @abstractmethod
    def legal_actions(self) -> List[int]:
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @abstractmethod
    def is_done(self):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @abstractmethod
    def is_dead(self):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    @abstractmethod
    def clone(self):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented.\n"
            f"It is recommended to write "
            f"\"return clone_inherited_instance(__class__, self)\"")

    @abstractmethod
    def __str__(self):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")

    def __lt__(self, other):
        return self.evaluated_score_ < other.evaluated_score_

    def cloneAdvanced(self, action: int):
        cloned: BaseState = self.clone()
        cloned.advance(action)
        cloned.parent_ = self
        cloned.last_action_ = action
        return cloned

    @abstractmethod
    def evaluate_score(self):
        raise NotImplementedError(
            f"{sys._getframe().f_code.co_name} is not implemented")


def beamSearchAction(state: BaseState, beam_width):
    now_beam: List[BaseState] = []
    best_state: BaseState = None
    heapq.heappush(now_beam, state)
    while True:
        next_beam: List[BaseState] = []

        for i in range(beam_width):
            if len(now_beam) == 0:
                break
            now_state = now_beam[0]
            assert (now_state is not None)
            heapq.heappop(now_beam)
            legal_actions = now_state.legal_actions()
            for action in legal_actions:
                next_state = now_state.cloneAdvanced(action)
                if next_state.is_dead():
                    continue
                next_state.evaluate_score()

                if (len(next_beam) >= beam_width
                        and
                        next_beam[0].evaluated_score_
                        >= next_state.evaluated_score_):
                    continue

                assert (next_state.parent_ is not None)

                if next_state.is_done():
                    if best_state is None or best_state < next_state:
                        best_state = next_state
                    continue
                heapq.heappush(next_beam, next_state)
                if len(next_beam) > beam_width:
                    heapq.heappop(next_beam)
        if best_state is not None:
            break
        now_beam = next_beam
    actions = []
    while best_state.parent_ is not None:
        actions.append(best_state.last_action_)
        best_state = best_state.parent_
    actions = actions[::-1]
    return actions


def beam_py_function(beam_width):
    return lambda state: beamSearchAction(state, beam_width)


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

    def __eq__(self, other):
        if not isinstance(other, Coord):
            return NotImplemented
        return (self.y_, self.x_) == (other.y_, other.x_)

    def __ne__(self, other):
        return not self.__eq__(other)


class MazeState(BaseState):
    dy = [0, 0, 1, -1]  # 右、左、下、上への移動方向のy成分
    dx = [1, -1, 0, 0]  # 右、左、下、上への移動方向のx成分
    H = 3
    W = 4
    END_TURN = 4
    INF = 1000000000

    def __init__(self, seed=None) -> None:
        super().__init__()
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
        self.evaluated_score_ = self.game_score_
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
        return clone_inherited_instance(__class__, self)

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
    state = MazeState(0)

    game_number = 10
    per_game_number = 10
    numbers = game_number, per_game_number

    # states: List[MazeState] = [MazeState(i) for i in range(10)]
    # pq: List[MazeState] = []
    # for state in states:
    #     state.evaluated_score_ = random.randrange(4)
    #     heapq.heappush(pq, state)

    # while len(pq) > 0:
    #     print(pq[0].evaluated_score_)
    #     heapq.heappop(pq)

    def get_name_beam(beamwidth):
        return (f"beam {beamwidth}", beam_py_function(beam_width=beamwidth))
    test_ai_performance(get_name_beam(2), *numbers)
    test_ai_performance(get_name_beam(4), *numbers)
    test_ai_performance(get_name_beam(8), *numbers)
    test_ai_performance(get_name_beam(16), *numbers)
    test_ai_performance(get_name_beam(32), *numbers)
