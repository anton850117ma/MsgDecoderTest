cmake_minimum_required(VERSION 3.20)
project(decoder)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_definitions(NDEBUG)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using Clang
  add_compile_options(-march=native -O3 -ggdb3 -Wall)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  add_compile_options(-march=native -O3 -ggdb3 -Wall)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
endif()

# add_executable(main main.cpp)
# enable_testing()
# add_test(NAME test1 COMMAND $<TARGET_FILE:main> 1)
# add_test(NAME test2 COMMAND $<TARGET_FILE:main> 2)
# add_test(NAME test3 COMMAND $<TARGET_FILE:main> 3)
# add_test(NAME test4 COMMAND $<TARGET_FILE:main>)

# add_executable(temp temp.cpp)
add_executable(test mmin.cpp source.cpp)