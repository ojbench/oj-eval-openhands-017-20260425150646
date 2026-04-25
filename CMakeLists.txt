cmake_minimum_required(VERSION 3.10)
project(problem_017)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(code code.cpp)
# For speed and smaller binary
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(code PRIVATE -O2 -pipe -static -s)
  # static may fail on some systems; keep dynamic fallback if static not available
endif()
