#include <iostream>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <filesystem>
#include <experimental/string_view>

#include "cpu.hpp"

/* TODO List
 * 1. Distro Display
 * 2. Linux version
 * 3. Packages
 * 4. Shell version
 * 5. Up Time
 * 6. Storage
 * 7. RAM percentage
 * 8. CPU information */

#define RELEASE "/etc/os-release"

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

auto main(int argc, const char* argv[]) -> int {
    cpu::vendor_id();
    std::cout << std::endl;
    cpu::get_cpu_id();
    std::cout << std::endl;
    std::cout << physmem_total() << std::endl;
    std::cout << physmem_available() << std::endl;

    for (std::size_t i { 0x1 }; i < argc; i++) {
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
}
