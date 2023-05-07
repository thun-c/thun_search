# thunsearch, Search algorithm library for Python built in C++

This is a library of search algorithms such as beam search.　　
Usually, implementing a search algorithm in python is too slow to be useful.  
Therefore, I developed this library to alleviate the speed problem by implementing the search part in c++.


## Prerequisites

* A compiler with C++11 support
* Pip 10+ or CMake >= 3.4 (or 3.14+ on Windows, which was the first version to support VS 2019)
* Ninja or Pip 10+


## Installation

Just pip install. 

```bash
pip install thunsearch
```

## How to use

Define a subclass that inherits from BaseState corresponding to the algorithm you want to use, and implement the methods with the "must" label.  
For example, if you want to use beam search, define a class that extends BaseContextualState and call beam_search_action.  
An example of calling beam_search_action by properly implementing a method with a "must" label in BaseContextualState is shown in `contextual_sample.py`.

```bash
python sample/contextual_sample.py
```

Conversely, an example of a failure to call beam_search_action without properly implementing a method labeled "must" in BaseContextualState is shown in `contextual_not_implemented_sample.py`.

```bash
python sample/contextual_not_implemented_sample.py
```

If a method labeled "must" is not properly implemented, raises a NotImplementedError.

```bash
NotImplementedError: must functions are not implemented. [legal_actions] 
```

## Speed Comparison (Python only vs With cpp)

I compared the speed of beam search between a program implemented using only python and a program implemented using c++ as well.

The results of a program implemented only in python are shown below.

```bash
python test_speed/test_speed_python_only.py
```

```bash
"beam 2" score:22.9     time:14.0
"beam 4" score:23.3     time:24.0
"beam 8" score:23.5     time:40.0
"beam 16" score:23.5    time:62.0
"beam 32" score:23.5    time:86.0
```

The results of the program, implemented in a combination of python and C++, are as follows.


```bash
python test_speed/test_speed_with_cpp.py 
```

```bash
"beam 2" score:22.9     time:7.0
"beam 4" score:23.3     time:14.0
"beam 8" score:23.5     time:21.0
"beam 16" score:23.5    time:30.0
"beam 32" score:23.5    time:40.0
```

It was shown to be approximately twice as fast when implemented using C++ as well, compared to implementation using python alone.




## Implemented Algorithms

### Algorithms for Contextual Problems

- Beam Search

## Algorithms to be implemented in the future (TBD)

### Algorithms for Contextual Problems

- Same board removal (e.g. zobrist hashing)
- Chokudai Search
 
### Algorithms for Non-Contextual Problems

- Hill Climbing
- Simulated Annealing
- Genetic Algorithm

### Algorithms for Alternate Games

- Alpha Beta
- Montecarlo Tree Search
- Thunder Search

### Algorithms for Simultaneous Games

- Decoupled Upper Confidence Tree

## Related resources

- [ゲームで学ぶ探索アルゴリズム実践入門
～木探索とメタヒューリスティクス （The Japanese technical book written by library developer）](https://gihyo.jp/book/2023/978-4-297-13360-3)
