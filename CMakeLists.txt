cmake_minimum_required(VERSION 3.4...3.18)
project(thun_search)

add_subdirectory(pybind11)
pybind11_add_module(thun_search src/main.cpp)

# EXAMPLE_VERSION_INFO is defined by setup.py and passed into the C++ code as a
# define (VERSION_INFO) here.
target_compile_definitions(thun_search
    PRIVATE VERSION_INFO=${EXAMPLE_VERSION_INFO})
