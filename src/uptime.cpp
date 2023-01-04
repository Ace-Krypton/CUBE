/*
 * (C)opyright 2022 Ramiz Abbasov <ramizna@code.edu.az>
 * See LICENSE file for license details
 */

#include <iostream>
#include <fstream>
#include <iomanip>

#include "uptime.hpp"
#include "cpu.hpp"

/**
 * \brief Prints the information about uptime by reading the "/proc/uptime" file
 */
[[maybe_unused]] auto uptime::uptime_display() -> void {
    std::ifstream uptime_file(UPTIME);
    if (!uptime_file.is_open()) cpu::fatal_error("Error: Failed to open /proc/uptime");

    std::string line;
    std::getline(uptime_file, line);
    std::istringstream iss(line);
    std::uint64_t uptime_seconds;
    iss >> uptime_seconds;

    std::uint64_t hours = uptime_seconds / 3600;
    std::uint64_t minutes = (uptime_seconds / 60) % 60;
    std::uint64_t seconds = uptime_seconds % 60;

    std::cout << std::setw(2) << std::setfill('0') << hours << ":"
              << std::setw(2) << std::setfill('0') << minutes << ":"
              << std::setw(2) << std::setfill('0') << seconds << std::endl;
}
