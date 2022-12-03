#include <iostream>

#include "os-check.hpp"

#ifdef UNIX
#include <fstream>

#include "cpu.hpp"
#endif

auto cpu::vendor_id() -> void {
#if defined(X86)
    __asm__("mov $0x0, %eax\n\t");
    __asm__("cpuid\n\t");
    __asm__("mov %%ebx, %0\n\t":"=r" (cpu::vendor_output[0x0]));
    __asm__("mov %%edx, %0\n\t":"=r" (cpu::vendor_output[0x1]));
    __asm__("mov %%ecx, %0\n\t":"=r" (cpu::vendor_output[0x2]));

    std::cout << std::string{ (const char *)cpu::vendor_output };
#endif
}

auto cpu::model_name(std::uint32_t eax_values) -> void {
#if defined(X86)
    switch (eax_values) {
        case 0x1: __asm__("mov $0x80000002, %eax\n\t"); break;
        case 0x2: __asm__("mov $0x80000003, %eax\n\t"); break;
        case 0x3: __asm__("mov $0x80000004, %eax\n\t"); break;
        default: std::cout << "Something went wrong" << std::endl; break;
    }

    __asm__("cpuid\n\t");
    __asm__("mov %%eax, %0\n\t":"=r" (cpu::register_output[0x0]));
    __asm__("mov %%ebx, %0\n\t":"=r" (cpu::register_output[0x1]));
    __asm__("mov %%ecx, %0\n\t":"=r" (cpu::register_output[0x2]));
    __asm__("mov %%edx, %0\n\t":"=r" (cpu::register_output[0x3]));

    std::cout << std::string{ (const char *)&cpu::register_output[0x0] };
#elif defined(UNIX)
    std::string model_name { "model name" }, cpu_info { };
    std::ifstream file { CPU_INFO };

    if (!(file.is_open())) std::cout << "<unknown>" << std::endl;

    for (std::string line; (std::getline(file, line)); ) {
        if (line.find(model_name) != std::string::npos) {
            const std::size_t start_pos = line.find(model_name);
            std::string temp = line.substr(start_pos + 0xD);
            const std::size_t stop_pos = temp.find('\"');
            cpu_info = temp.substr(0x0, stop_pos);
        }
    }
    file.close();
    std::cout << cpu_info << std::endl;
#endif
}

auto cpu::get_cpu_id() -> void {
#if defined(X86)
    __asm__("xor %eax, %eax\n\t");
    __asm__("xor %ebx, %ebx\n\t");
    __asm__("xor %ecx, %ecx\n\t");
    __asm__("xor %edx, %edx\n\t");

    for (std::uint32_t values { 0x1 }; values <= 0x3; values++) cpu::model_name(values);
#endif
}
