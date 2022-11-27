#include <iostream>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <filesystem>
#include <experimental/string_view>

/* TODO List
 * 1. Distro Display
 * 2. Linux version
 * 3. Packages
 * 4. Shell version
 * 5. Up Time
 * 6. Storage
 * 7. RAM percentage
 * 8. CPU information */

std::uint32_t register_output[0xA];
static std::string base_path = "/sys/class/power_supply/";

auto brand_string(std::uint32_t eax_values) -> void {
    switch (eax_values) {
        case 0x1: __asm__("mov $0x80000002, %eax\n\t"); break;
        case 0x2: __asm__("mov $0x80000003, %eax\n\t"); break;
        case 0x3: __asm__("mov $0x80000004, %eax\n\t"); break;
        default: std::cout << "Something went wrong" << std::endl; break;
    }

    __asm__("cpuid\n\t");
    __asm__("mov %%eax, %0\n\t":"=r" (register_output[0x0]));
    __asm__("mov %%ebx, %0\n\t":"=r" (register_output[0x1]));
    __asm__("mov %%ecx, %0\n\t":"=r" (register_output[0x2]));
    __asm__("mov %%edx, %0\n\t":"=r" (register_output[0x3]));

    std::cout << std::string((const char *)&register_output[0x0]);
}

auto get_cpu_id() -> void {
    __asm__("xor %eax, %eax\n\t");
    __asm__("xor %ebx, %ebx\n\t");
    __asm__("xor %ecx, %ecx\n\t");
    __asm__("xor %edx, %edx\n\t");

    for (std::uint32_t values { 0x1 }; values <= 0x3; values++) brand_string(values);
}

auto physmem_total() -> std::int64_t {
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    return ((pages > 0x0) && (page_size > 0x0)) ? (pages * page_size) / 0x3E8 : 0x1;
}

auto physmem_available() -> std::int64_t {
    long pages = sysconf (_SC_AVPHYS_PAGES);
    long pagesize = sysconf (_SC_PAGESIZE);
    return (0x0 <= pages && 0x0 <= pagesize) ? (pages * pagesize) / 0x3E8 : 0x1;
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

auto cpu_info() -> std::string {
    std::string model_name { "model name" }, cpu_info { };
    std::ifstream file { "/proc/cpuinfo" };

    if (!(file.is_open())) return { };

    for (std::string line; (std::getline(file, line)); ) {
        if (line.find(model_name) != std::string::npos) {
            const std::size_t start_pos = line.find(model_name);
            std::string temp = line.substr(start_pos + 0xD);
            const std::size_t stop_pos = temp.find('\"');
            cpu_info = temp.substr(0x0, stop_pos);
        }
    }
    file.close();
    return cpu_info;
}

auto distro_display() -> std::string {
    std::string pretty_name { "PRETTY_NAME=\"" }, name { };
    std::ifstream file { "/etc/os-release" };

    if (!(file.is_open())) return { };

    while ((std::getline(file, name))) if (name.find(pretty_name) != std::string::npos) break;
    file.close();

    name = name.substr(pretty_name.length(), name.length() - (pretty_name.length() + 0x1));
    return (name.empty()) ? std::string() : name;
}

auto main(int argc, const char* argv[]) -> int {
    get_cpu_id();
    std::cout << std::endl;
    std::cout << physmem_total() << std::endl;
    std::cout << physmem_available() << std::endl;

    for (std::size_t i { 0x1 }; i < argc; i++) {
        if (std::experimental::string_view(argv[i]) == "--cpu") {
            std::cout << cpu_info() << std::endl;
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
