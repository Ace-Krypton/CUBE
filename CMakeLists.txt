cmake_minimum_required(VERSION 3.23)
project(CUBE)
set(CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_FLAGS "-lncurses")

add_executable(CUBE src/main.cpp src/cpu.cpp src/cpu.hpp src/architecture.hpp src/version.hpp src/version.cpp)
