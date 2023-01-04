/*
 * (C)opyright 2022 Ramiz Abbasov <ramizna@code.edu.az>
 * See LICENSE file for license details
 */

#include <iostream>
#include <vector>
#include <fstream>
#include <ncurses.h>
#include <filesystem>
#include <experimental/string_view>

#include "tui.hpp"
#include "cpu.hpp"

#define UPTIME "/proc/uptime"

auto uptime_display() -> void {
    std::ifstream uptime_file(UPTIME);
    if (!uptime_file.is_open()) cpu::fatal_error("Error: Failed to open /proc/uptime");

    std::string line;
    std::getline(uptime_file, line);
    std::istringstream iss(line);
    uint64_t uptime_seconds;
    iss >> uptime_seconds;

    uint64_t hours = uptime_seconds / 3600;
    uint64_t minutes = (uptime_seconds / 60) % 60;
    uint64_t seconds = uptime_seconds % 60;

    std::cout << std::setw(2) << std::setfill('0') << hours << ":"
              << std::setw(2) << std::setfill('0') << minutes << ":"
              << std::setw(2) << std::setfill('0') << seconds << std::endl;
}

auto main(int argc, const char* argv[]) -> int {
    /*  ------------------------------------  Tests  ------------------------------------  */
    initscr();
    noecho();
    cbreak();
    setlocale(LC_ALL, "");
    tui::draw();
    endwin();

    /*std::cout << "-------------------------------------------------------------------------" << std::endl;
    bool invariant = cpu::supports_invariantTSC();

    printf("Invariant TSC: %s\n", invariant ? "True" : "False");

    if (!invariant) {
        std::cout << "*** Without invariant TSC rdtsc is not a useful timer for wall clock time" << std::endl;
        return 0x1;
    }

    char const * source;
    double res;

    if (cpu::extract_leaf_15H(&res)) {
        source = "leaf 15H";

    } else if (cpu::read_HW_tick_from_name(&res)) {
        source = "model name string";

    } else {
        res = cpu::measure_TSC_tick();
        source = "measurement";
    }

    printf ("From %s frequency %sz => %s\n",
            source,
            cpu::format_SI(1./res,0x9,'H').c_str(),
            cpu::format_SI(res,0x9,'s').c_str());

    double measured = cpu::measure_TSC_tick();

    printf ("Sanity check against std::chrono::steady_clock gives frequency %sz => %s\n",
            cpu::format_SI(1./measured,0x9,'H').c_str(),
            cpu::format_SI(measured,0x9,'s').c_str());

    uint64_t minTicks = cpu::measure_clock_granularity();
    res = res*(double)minTicks;

    printf ("Measured granularity = %llu tick%s => %sz, %s\n",
            (unsigned long long)minTicks, minTicks != 0x1 ? "s": "", cpu::format_SI(1./res,0x9,'H').c_str(),
            cpu::format_SI(res,0x9,'s').c_str());

    std::cout << "-------------------------------------------------------------------------" << std::endl;
    cpu::get_both_cores();
    std::cout << "-------------------------------------------------------------------------" << std::endl;
    cpu::print_instructions();
    std::cout << "-------------------------------------------------------------------------" << std::endl;
    std::cout << "Cube version: " << cube::version() << std::endl;
    std::cout << cpu::vendor_id() << std::endl;
    cpu::get_cpu_id();
    std::cout << std::endl;
    std::cout << "-------------------------------------------------------------------------" << std::endl;
    std::cout << physmem_total() << std::endl;
    std::cout << physmem_available() << std::endl;
    std::cout << "-------------------------------------------------------------------------" << std::endl;*/
    //---------------------------------------------------------------------------------
    /*for (std::size_t i { 0x1 }; i < argc; ++i) {
        if (std::experimental::string_view(argv[i]) == "--cpu") {
            cpu::vendor_id();
            cpu::get_cpu_id();
        }

        else if (std::experimental::string_view(argv[i]) == "--distro") {
            std::cout << distro_display() << std::endl;
        }

        else if (std::experimental::string_view(argv[i]) == "--battery-manufacturer") {
            for (auto const &battery : get_battery()) std::cout << battery << std::endl;
        }

        else {
            std::cout << "Invalid command line argument" << std::endl;
            return 0x1;
        }
    }*/

    return 0x0;

    /*  ---------------------------------------------------------------------------------  */
}
