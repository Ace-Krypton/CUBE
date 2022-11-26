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

int register_output[0xA];

auto eax_1() -> void {
    __asm__("mov $0x1 , %eax\n\t");
    __asm__("cpuid\n\t");
    __asm__("mov %%eax, %0\n\t":"=r" (register_output[0x0]));
    __asm__("mov %%ebx, %0\n\t":"=r" (register_output[0x1]));
    __asm__("mov %%ecx, %0\n\t":"=r" (register_output[0x2]));
    __asm__("mov %%edx, %0\n\t":"=r" (register_output[0x3]));
}

auto brand_string(int eax_values) -> void {
    switch (eax_values) {
        case 0x1 : __asm__("mov $0x80000002 , %eax\n\t"); break;
        case 0x2: __asm__("mov $0x80000003 , %eax\n\t"); break;
        case 0x3: __asm__("mov $0x80000004 , %eax\n\t"); break;
        default: std::cout << "Something went wrong" << std::endl; break;
    }

    __asm__("cpuid\n\t");
    __asm__("mov %%eax, %0\n\t":"=r" (register_output[0x0]));
    __asm__("mov %%ebx, %0\n\t":"=r" (register_output[0x1]));
    __asm__("mov %%ecx, %0\n\t":"=r" (register_output[0x2]));
    __asm__("mov %%edx, %0\n\t":"=r" (register_output[0x3]));
    printf("%s", &register_output[0x0]);
}

auto get_cpu_id()-> void {
    __asm__("xor %eax , %eax\n\t");
    __asm__("xor %ebx , %ebx\n\t");
    __asm__("xor %ecx , %ecx\n\t");
    __asm__("xor %edx , %edx\n\t");

    for (int i = 0x1; i <= 0x3; i++) brand_string(i);
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

    name = name.substr(pretty_name.length(),name.length() - (pretty_name.length() + 0x1));
    return (name.empty()) ? std::string() : name;
}

auto main(int argc, const char* argv[]) -> int {
    get_cpu_id();

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
