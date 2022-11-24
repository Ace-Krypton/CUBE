#include <iostream>
#include <fstream>
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

auto cpu_info() -> std::string {
    std::string model_name { "model name" }, cpu_info { };
    std::ifstream file { "/proc/cpuinfo" };

    if (!(file.is_open())) return { };

    for (std::string line; (std::getline(file, line));) {
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

    while ((std::getline(file, name))) {
        if (name.find(pretty_name) != std::string::npos) break;
    }
    file.close();
    name = name.substr(pretty_name.length(),name.length() - (pretty_name.length() + 0x1));
    return (name.empty()) ? std::string() : name;
}

auto main(int argc, const char* argv[]) -> int {
    for (std::size_t i = 0x1; i < argc; i++) {
        if (std::experimental::string_view(argv[i]) == "--cpu") {
            std::cout << cpu_info() << std::endl;
        }

        else if (std::experimental::string_view(argv[i]) == "--distro") {
            std::cout << distro_display() << std::endl;
        }

        else {
            std::cout << "Invalid command line argument" << std::endl;
            return 0x1;
        }
    }
    return 0x0;
}
