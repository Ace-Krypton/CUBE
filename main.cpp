#include <iostream>
#include <fstream>

/* TODO List
 * 1. Distro Display
 * 2. Linux version
 * 3. Packages
 * 4. Shell version
 * 5. Up Time
 * 6. Storage
 * 7. RAM percentage */

auto get_cpu() -> std::string {
    std::string model_name { "model name" }, cpu_info { };
    std::ifstream file("/proc/cpuinfo");

    if (!(file.is_open())) return { };

    for (std::string line; (std::getline(file,line));) {
        if (line.find(model_name) != std::string::npos) {
            const std::size_t start_pos = line.find(model_name);
            std::string temp = line.substr(start_pos+6);
            const std::size_t stop_pos = temp.find('\"');
            std::string result = temp.substr(0, stop_pos);
            std::cout << result << std::endl;
        }
    }
    return cpu_info;
}

auto main() -> int {
    get_cpu();
    return 0x0;
}
