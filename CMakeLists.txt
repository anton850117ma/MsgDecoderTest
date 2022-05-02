cmake_minimum_required(VERSION 3.20)
project(test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# add_compile_definitions(NDEBUG)
add_compile_options(-O3 -ggdb3 -Wall -Wextra)

add_executable(test main.cpp)

enable_testing()
add_test(NAME test_0 COMMAND $<TARGET_FILE:test>)