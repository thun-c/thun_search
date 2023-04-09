import thun_search as thun
import pandas


def test_main():
    assert thun.add(1, 2) == 3
    assert thun.subtract(1, 2) == -1


class Cat(thun.Animal):

    def go(self, n_times):
        return "meow! " * n_times


if __name__ == "__main__":
    print("thun\__version__", thun.__version__)
    print([key for key in thun.__dict__.keys() if "__" != key[:2]])
    test_main()
    cat = Cat()
    print(thun.call_go(cat))
