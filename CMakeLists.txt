cmake_minimum_required(VERSION 3.23)
project(CUBE)
set(CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_FLAGS "-lncursesw -lsensors")

add_executable(CUBE src/main.cpp src/cpu.cpp src/cpu.hpp src/architecture.hpp src/version.hpp src/version.cpp src/tui.cpp src/tui.hpp src/ram.cpp src/ram.hpp src/acpi.cpp src/acpi.hpp src/distro.cpp src/distro.hpp src/uptime.cpp src/uptime.hpp)