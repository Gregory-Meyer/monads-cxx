cmake_minimum_required(VERSION 3.8)

project(expected)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(./include/)

enable_testing()

add_executable(test_expected ./test/main.cpp ./test/expected.cpp)

add_test(Test test_expected)
