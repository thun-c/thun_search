import thun_search as thun
import pandas


def test_main():
    assert thun.add(1, 2) == 3
    assert thun.subtract(1, 2) == -1


class Cat(thun.Animal):

    def go(self, n_times):
        return "meow! " * n_times


class MazeState(thun.State):
    dy = [1, -1, 0, 0]
    dx = [0, 0, 1, 1]

    def __init__(self) -> None:
        # thun.State.__init__(self)
        super().__init__()
        self.a = 10

    def isDone(self):
        return True

    def evaluateScore(self):
        super().setEvaluateScore(10)
        return

    def advance(self, action):
        return

    def legalActions(self):
        return []


if __name__ == "__main__":
    print("thun\__version__", thun.__version__)
    print([key for key in thun.__dict__.keys() if "__" != key[:2]])
    test_main()
    cat = Cat()
    print(thun.call_go(cat))
    state = MazeState()
    print("thun.State", [
          key for key in thun.State.__dict__.keys() if "__" != key[:2]])
    print("MazeState", [
          key for key in MazeState.__dict__.keys() if "__" != key[:2]])

    print("state", [
          key for key in state.__dict__.keys() if True or "__" != key[:2]])
    # state.evaluateScore()
    print(thun.getEvaluatedScore(state))
