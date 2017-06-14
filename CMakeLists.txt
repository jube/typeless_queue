cmake_minimum_required(VERSION 3.0)

project(QUEUE
  VERSION 0.1.0
)

add_executable(random-test
  tests/random-test.cc
)

target_include_directories(random-test
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_options(random-test
  PRIVATE
    -O2 -Wall -Wextra -std=c++11
)
