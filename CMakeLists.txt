cmake_minimum_required(VERSION 3.1)

project(expected)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(./include/)

enable_testing()

add_executable(test_monads ./test/main.cpp ./test/expected.cpp
						   ./test/optional.cpp)

add_test(Test test_monads)
