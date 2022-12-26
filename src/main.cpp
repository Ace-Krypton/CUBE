/*
 * (C)opyright 2022 Ramiz Abbasov <ramizna@code.edu.az>
 * See LICENSE file for license details
 */

#include <iostream>
#include <vector>
#include <ncurses.h>
#include <fstream>
#include <unistd.h>
#include <filesystem>
#include <experimental/string_view>
#include <cstring>
#include <thread>

#include "cpu.hpp"
#include "version.hpp"

/* TODO List
 * 1. Distro Display
 * 2. Linux version
 * 3. Packages
 * 4. Shell version
 * 5. Uptime
 * 6. Storage
 * 7. RAM percentage
 * 8. CPU information */

typedef long long ll;
#define RELEASE "/etc/os-release/"
#define CPU_STAT "/proc/stat"

static std::string base_path = "/sys/class/power_supply/";

auto physmem_total() -> std::int64_t {
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    return ((pages > 0x0) && (page_size > 0x0)) ? (pages * page_size) / 0x3E8 : 0x1;
}

auto physmem_available() -> std::int64_t {
    long pages = sysconf (_SC_AVPHYS_PAGES);
    long page_size = sysconf (_SC_PAGESIZE);
    return (0x0 <= pages && 0x0 <= page_size) ? (pages * page_size) / 0x3E8 : 0x1;
}

auto get_battery() -> std::vector<std::string> {
    std::string vendor { };
    std::uint16_t _id { 0x0 };
    std::vector<std::string> vendors { };
    std::vector<std::uint16_t> batteries { };

    while (std::filesystem::exists(std::filesystem::path(base_path + "BAT" + std::to_string(_id)))) {
        batteries.emplace_back(_id++);
    }

    for (auto const &battery : batteries) {
        if (battery < 0x0) std::cout << "<unknown>" << std::endl;
        std::ifstream vendor_file(base_path + "BAT" + std::to_string(battery) + "/" + "manufacturer");
        if (!(vendor_file.is_open())) return { };
        while ((std::getline(vendor_file, vendor))) vendors.emplace_back(vendor);
    }

    return vendors;
}

auto distro_display() -> std::string {
    std::string pretty_name { "PRETTY_NAME=\"" }, name { };
    std::ifstream file { RELEASE };

    if (!(file.is_open())) return { };

    while ((std::getline(file, name))) if (name.find(pretty_name) != std::string::npos) break;
    file.close();

    name = name.substr(pretty_name.length(), name.length() - (pretty_name.length() + 0x1));
    return (name.empty()) ? std::string() : name;
}

auto progress_bar(const std::string& percent) -> std::string {
    std::string result = "CPU [";
    uint64_t _size = 50;

    uint64_t boundaries = (uint64_t)(stof(percent) / 100) * _size;

    for (size_t i = 0; i < _size; ++i){
        if (i <= boundaries) result += "|";
        else result += " ";
    }

    result += percent + "]";
    return result;
}

auto cpu_percentage() -> std::string {
    std::ifstream stat_file(CPU_STAT);
    std::string line;
    std::getline(stat_file, line);

    std::istringstream iss(line);
    std::string token;
    iss >> token;

    ll user = 0, nice = 0, system = 0, idle = 0, iowait = 0, irq = 0, softirq = 0, steal = 0, guest = 0, guest_nice = 0;
    iss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

    ll total_time = user + nice + system + idle + iowait + irq + softirq + steal;
    ll idle_time = idle + iowait;

    stat_file.close();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    stat_file.open(CPU_STAT);
    std::getline(stat_file, line);

    std::istringstream iss2(line);
    iss2 >> token;
    iss2 >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

    ll total_time_2 = user + nice + system + idle + iowait + irq + softirq + steal;
    ll idle_time_2 = idle + iowait;

    stat_file.close();

    double usage = (double)(total_time_2 - total_time - (idle_time_2 - idle_time))
            / (double)(total_time_2 - total_time) * 100;

    return std::to_string(usage);
}

auto write_console(WINDOW * win) -> void {
    mvwprintw(win, 2, 2, "%s", distro_display().c_str());
    wattron(win,COLOR_PAIR(1));
    wprintw(win, "%s", (progress_bar(cpu_percentage())).c_str());
}

auto main(int argc, const char* argv[]) -> int {
    /*  ------------------------------------  Tests  ------------------------------------  */
    initscr();
    noecho();
    cbreak();
    start_color();
    int yMax,xMax;
    getmaxyx(stdscr,yMax,xMax);
    WINDOW * sys_win = newwin(17,xMax-1,0,0);
    init_pair(1,COLOR_BLUE,COLOR_BLACK);
    init_pair(2,COLOR_GREEN,COLOR_BLACK);
    keypad(sys_win, TRUE);

    while (true) {
        box(sys_win,0,0);
        write_console(sys_win);
        wrefresh(sys_win);
        refresh();
    }
    endwin();

/*    std::cout << "-------------------------------------------------------------------------" << std::endl;
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

    for (std::size_t i { 0x1 }; i < argc; ++i) {
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
    }
    return 0x0;

    /*  ---------------------------------------------------------------------------------  */
}
