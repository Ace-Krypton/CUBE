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
#define RELEASE "/etc/os-release/"

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

auto cpu_percentage() -> void {
    // Initialize variables to store the clock speed and elapsed time
    int clockSpeed;
    long long elapsedTime;

    // Save the current clock speed of the CPU
    unsigned long long MSRValue;
    __asm__ __volatile__("rdmsr" : "=A"(MSRValue) : "c"(0x198)); // IA32_PERF_CTL MSR
    int currentClockSpeed = (int)(MSRValue & 0xFFFFFFFF);
    std::cout << currentClockSpeed << std::endl;

    // Set the clock speed of the CPU to a constant value
    int newClockSpeed = 1000; // 1000 MHz
    MSRValue &= ~0xFFFFFFFF;
    MSRValue |= newClockSpeed;
    __asm__ __volatile__("wrmsr" : : "c"(0x198), "A"(MSRValue)); // IA32_PERF_CTL MSR

    // Get the clock speed of the CPU
    clockSpeed = newClockSpeed;

    // Measure the elapsed time
    long long startTime = 0;
    __asm__ __volatile__("rdtsc" : "=A"(startTime));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    long long endTime = 0;
    __asm__ __volatile__("rdtsc" : "=A"(endTime));
    elapsedTime = endTime - startTime;

    // Calculate the CPU usage percentage
    double usagePercentage = 100.0 * (double)elapsedTime / (double)(clockSpeed * 1000);

    // Print the CPU usage percentage
    std::cout << "CPU usage: " << usagePercentage << "%" << std::endl;

    // Restore the original clock speed of the CPU
    MSRValue &= ~0xFFFFFFFF;
    MSRValue |= currentClockSpeed;
    __asm__ __volatile__("wrmsr" : : "c"(0x198), "A"(MSRValue)); // IA32_PERF_CTL MSR
}

/*auto write_console(WINDOW * win) -> void {
    mvwprintw(win, 2, 2, "%s", distro_display().c_str());
    mvwprintw(win, 3, 2, "%s", reinterpret_cast<const char *>(physmem_available()));
    mvwprintw(win, 4, 2, "%s", reinterpret_cast<const char *>(physmem_total()));
    wattron(win,COLOR_PAIR(1));
    wprintw(win,getCString(Util::getProgressBar(sys.getCpuPercent())));
    wattroff(win,COLOR_PAIR(1));
    mvwprintw(win,5,2,getCString(( "Other cores:")));
    wattron(win,COLOR_PAIR(1));
    std::vector<std::string> val = sys.getCoresStats();
    for (int i = 0; i < val.size(); i++) {
        mvwprintw(win,(6+i),2,getCString(val[i]));
    }
    wattroff(win,COLOR_PAIR(1));
    mvwprintw(win,10,2,getCString(( "Memory: ")));
    wattron(win,COLOR_PAIR(1));
    wprintw(win,getCString(Util::getProgressBar(sys.getMemPercent())));
    wattroff(win,COLOR_PAIR(1));
    mvwprintw(win,11,2,getCString(( "Total Processes:" + sys.getTotalProc())));
    mvwprintw(win,12,2,getCString(( "Running Processes:" + sys.getRunningProc())));
    mvwprintw(win,13,2,getCString(( "Up Time: " + Util::convertToTime(sys.getUpTime()))));
}*/

auto main(int argc, const char* argv[]) -> int {
    /*  ------------------------------------  Tests  ------------------------------------  */
    cpu_percentage();
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
    endwin();
    return 0x0;

    /*  ---------------------------------------------------------------------------------  */
}
