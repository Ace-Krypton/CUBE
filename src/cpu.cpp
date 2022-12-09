/*
 * (C)opyright 2022 Ramiz Abbasov <ramizna@code.edu.az>
 * See LICENSE file for license details
 */

/**
 * TODO List
 * 1. Getting number of physical and logical cores
 * 2. Getting clock speed (Max, Regular)
 * 3. Getting cache size
 * 4. Sockets?
 */

#include <iostream>

#include "architecture.hpp"

#ifdef UNIX
#include <fstream>

#include "cpu.hpp"
#endif

/**
 * \brief "mov $0x0, %eax" will return the processor's manufacture string
 *             and highest function parameters possible (EAX=0)
 */
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

auto cpu::cpu_id(size_t i, unsigned regs[4]) -> void {
    asm volatile ("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3]) : "a" (i), "c" (0));
}

auto cpu::get_both_cores() -> void {
    unsigned regs[0x4];

    char vendor[0xC];
    cpu::cpu_id(0, regs);
    ((unsigned *)vendor)[0x0] = regs[0x1];
    ((unsigned *)vendor)[0x1] = regs[0x3];
    ((unsigned *)vendor)[0x2] = regs[0x2];
    std::string cpu_vendor = std::string(vendor, 0xC);

    cpu::cpu_id(0x1, regs);
    unsigned cpu_features = regs[0x3];

    cpu::cpu_id(0x1, regs);
    unsigned logical = (regs[0x1] >> 0x10) & 0xff;
    std::cout << "logical cpus: " << logical << std::endl;
    unsigned cores = logical;

    if (cpu_vendor == "GenuineIntel") {
        cpu::cpu_id(0x4, regs);
        cores = ((regs[0x0] >> 0x1A) & 0x3f) + 0x1;

    } else if (cpu_vendor == "AuthenticAMD") {
        cpu::cpu_id(0x80000008, regs);
        cores = ((unsigned)(regs[0x2] & 0xff)) + 0x1;
    }

    std::cout << "cpu cores: " << cores << std::endl;

    bool hyperThreads = cpu_features & (0x1 << 0x1C) && cores < logical;
    std::cout << "hyper-threads: " << (hyperThreads ? "true" : "false") << std::endl;
}

/**
 * \brief "mov $0x1 , %eax" will give processor features
 *             and model related information (EAX=1)
 */
auto cpu::instruction_set_checker() -> void {
#if defined(X86)
    __asm__("mov $0x1 , %eax\n\t");
    __asm__("cpuid\n\t");
    __asm__("mov %%eax, %0\n\t":"=r" (cpu::instruction_detection[0x2]));
    __asm__("mov %%ecx, %0\n\t":"=r" (cpu::instruction_detection[0x0]));
    __asm__("mov %%edx, %0\n\t":"=r" (cpu::instruction_detection[0x1]));

    instruction_set::has_fpu = (cpu::instruction_detection[0x1] & (0x1 << 0x0)) != 0x0;
    instruction_set::has_mmx = (cpu::instruction_detection[0x1] & (0x1 << 0x17)) != 0x0;
    instruction_set::has_sse = (cpu::instruction_detection[0x1] & (0x1 << 0x19)) != 0x0;
    instruction_set::has_sse3 = (cpu::instruction_detection[0x0] & (0x1 << 0x0)) != 0x0;
    instruction_set::has_avx = (cpu::instruction_detection[0x0] & (0x1 << 0x1C)) != 0x0;
    instruction_set::has_sse2 = (cpu::instruction_detection[0x1] & (0x1 << 0x1A)) != 0x0;
    instruction_set::has_ssse3 = (cpu::instruction_detection[0x0] & (0x1 << 0x9)) != 0x0;
    instruction_set::has_f16c = (cpu::instruction_detection[0x0] & (0x1 << 0x1D)) != 0x0;
    instruction_set::has_sse4_1 = (cpu::instruction_detection[0x0] & (0x1 << 0x13)) != 0x0;
    instruction_set::has_sse4_2 = (cpu::instruction_detection[0x0] & (0x1 << 0x14)) != 0x0;
    instruction_set::has_pclmulqdq = (cpu::instruction_detection[0x0] & (0x1 << 0x1)) != 0x0;

    std::cout << std::boolalpha;
    std::cout << "Has FPU -> " << instruction_set::has_fpu << std::endl;
    std::cout << "Has MMX -> " << instruction_set::has_mmx << std::endl;
    std::cout << "Has SSE -> " << instruction_set::has_sse << std::endl;
    std::cout << "Has AVX -> " << instruction_set::has_avx << std::endl;
    std::cout << "Has F16C -> " << instruction_set::has_f16c << std::endl;
    std::cout << "Has SSE3 -> " << instruction_set::has_sse3 << std::endl;
    std::cout << "Has SSE2 -> " << instruction_set::has_sse2 << std::endl;
    std::cout << "Has SSSE3 -> " << instruction_set::has_ssse3 << std::endl;
    std::cout << "Has SSE4_1 -> " << instruction_set::has_sse4_1 << std::endl;
    std::cout << "Has SSE4_2 -> " << instruction_set::has_sse4_2 << std::endl;
    std::cout << "Has PCLMULQDQ -> " << instruction_set::has_pclmulqdq << std::endl;
#endif
}

/**
 * \brief Runs CPUID instruction with eax input of 8000002H through 80000004H
 * @param eax_values values from 8000002H to 80000004H
 */
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

#else
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

/**
 * \brief "xor %eax, %eax" is a faster way of setting eax to zero
 *        Also to avoid 0s when compiled as used on shell codes for exploitation of buffer overflows, etc
 */
auto cpu::get_cpu_id() -> void {
#if defined(X86)
    __asm__("xor %eax, %eax\n\t");
    __asm__("xor %ebx, %ebx\n\t");
    __asm__("xor %ecx, %ecx\n\t");
    __asm__("xor %edx, %edx\n\t");

    for (std::uint32_t values { 0x1 }; values <= 0x3; values++) cpu::model_name(values);
#endif
}
