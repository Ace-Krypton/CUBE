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
#include <x86intrin.h>
#include <cstdarg>
#include <chrono>
#include <iomanip>
#include <cstring>

#include "architecture.hpp"

#ifdef UNIX
#include "cpu.hpp"
#endif

/**
 * \brief "mov $0x0, %eax" will return the processor's manufacture string
 *             and highest function parameters possible (EAX=0)
 * @return ID of the vendor (GenuineIntel) for example
 */
auto cpu::vendor_id() -> std::string {
#if defined(X86)
    __asm__("mov $0x0, %eax\n\t");
    __asm__("cpuid\n\t");
    __asm__("mov %%ebx, %0\n\t":"=r" (cpu::vendor_output[0x0]));
    __asm__("mov %%edx, %0\n\t":"=r" (cpu::vendor_output[0x1]));
    __asm__("mov %%ecx, %0\n\t":"=r" (cpu::vendor_output[0x2]));

    return std::string{ (const char *)cpu::vendor_output };
#endif
}

auto cpu::format_SI(double interval, int width, char unit) -> std::string {
    std::stringstream os;
    static struct {
        double scale;
        char prefix;
    } ranges[] = {{1.e21, 'y'},  {1.e18, 'z'},  {1.e15, 'a'},  {1.e12, 'f'},
                  {1.e9, 'p'},   {1.e6, 'n'},   {1.e3, 'u'},   {1.0, 'm'},
                  {1.e-3, ' '},  {1.e-6, 'k'},  {1.e-9, 'M'},  {1.e-12, 'G'},
                  {1.e-15, 'T'}, {1.e-18, 'P'}, {1.e-21, 'E'}, {1.e-24, 'Z'},
                  {1.e-27, 'Y'}};

    if (interval == 0.0) {
        os << std::setw(width - 3)
           << std::right << "0.00"
           << std::setw(3) << unit;
        return os.str();
    }

    bool negative = false;
    if (interval < 0.0) {
        negative = true;
        interval = -interval;
    }

    for (auto & range : ranges) {
        if (interval * range.scale < 1.e0) {
            interval = interval * 1000.e0 * range.scale;
            os << std::fixed << std::setprecision(2) << std::setw(width - 3)
               << std::right << (negative ? -interval : interval) << std::setw(2)
               << range.prefix << std::setw(1) << unit;
            return os.str();
        }
    }

    os << std::setprecision(2) << std::fixed
       << std::right << std::setw(width - 3)
       << interval << std::setw(3) << unit;

    return os.str();
}

inline auto cpu::read_cycle_count()-> uint64_t {
    return __rdtsc();
}

auto cpu::measure_TSC_tick() -> double {
    auto start = std::chrono::steady_clock::now();
    uint64_t startTick = cpu::read_cycle_count();
    auto end = start + std::chrono::milliseconds(5);
    while (std::chrono::steady_clock::now() < end) { }
    size_t elapsed = cpu::read_cycle_count() - startTick;
    double tickTime = 5.e-3 / static_cast<double>(elapsed);
    return tickTime;
}

[[maybe_unused]] [[noreturn]] auto cpu::fatal_error(char const * Format, ...) -> void {
    fflush(stdout);
    va_list var_args;
    va_start(var_args, Format);
    vfprintf(stderr, Format, var_args);
    exit(1);
}

auto cpu::supports_invariantTSC() -> bool {
    __asm__("mov $0x80000007, %eax\n\t");
    __asm__("cpuid\n\t");
    __asm__("mov %%eax, %0\n\t":"=r" (cpu::invariantTSC[0x0]));
    __asm__("mov %%ebx, %0\n\t":"=r" (cpu::invariantTSC[0x1]));
    __asm__("mov %%ecx, %0\n\t":"=r" (cpu::invariantTSC[0x2]));
    __asm__("mov %%edx, %0\n\t":"=r" (cpu::invariantTSC[0x3]));

    return (cpu::invariantTSC[0x3] & (1 << 8)) != 0;
}

auto cpu::extract_leaf_15H(double * time) -> bool {
    __asm__("mov $0x0, %eax\n\t");
    __asm__("cpuid\n\t");
    __asm__("mov %%eax, %0\n\t":"=r" (cpu::leaf_extract[0x0]));
    __asm__("mov %%ebx, %0\n\t":"=r" (cpu::leaf_extract[0x1]));
    __asm__("mov %%ecx, %0\n\t":"=r" (cpu::leaf_extract[0x2]));
    __asm__("mov %%edx, %0\n\t":"=r" (cpu::leaf_extract[0x3]));

    if (cpu::leaf_extract[0x0] < 0x15) {
        std::cout << "cpuid leaf 15H is not supported" << std::endl;
        return false;
    }

    __asm__("xor %eax, %eax\n\t");
    __asm__("xor %ebx, %ebx\n\t");
    __asm__("xor %ecx, %ecx\n\t");
    __asm__("xor %edx, %edx\n\t");

    __asm__("mov $0x15, %eax\n\t");
    __asm__("cpuid\n\t");
    __asm__("mov %%eax, %0\n\t":"=r" (cpu::leaf_extract[0x0]));
    __asm__("mov %%ebx, %0\n\t":"=r" (cpu::leaf_extract[0x1]));
    __asm__("mov %%ecx, %0\n\t":"=r" (cpu::leaf_extract[0x2]));
    __asm__("mov %%edx, %0\n\t":"=r" (cpu::leaf_extract[0x3]));

    if (cpu::leaf_extract[0x1] == 0 || cpu::leaf_extract[0x2] == 0) {
        std::cout << "cpuid leaf 15H does not give frequency" << std::endl;
        return false;
    }

    double core_crystal_frequency = cpu::leaf_extract[0x2];
    *time = cpu::leaf_extract[0x0] / (cpu::leaf_extract[0x1] * core_crystal_frequency);
    printf("   cpuid leaf 15H: coreCrystal = %g, eax=%u, ebx=%u, ecx=%u "
           "=> %s\n",
           core_crystal_frequency,
           cpu::leaf_extract[0x0],
           cpu::leaf_extract[0x1],
           cpu::leaf_extract[0x2],
           cpu::format_SI(*time, 9, 's').c_str());
    return true;
}

auto cpu::read_HW_tick_from_name(double * time) -> bool {
    std::string model_name = cpu::vendor_id();
    
    if (model_name.find("Apple") != std::string::npos) return false;

    char const * model = model_name.c_str();
    auto end = model + strlen(model) - 3;
    uint64_t multiplier;

    if (*end == 'M')multiplier = 1000LL * 1000LL;
    
    else if (*end == 'G')multiplier = 1000LL * 1000LL * 1000LL;
    
    else if (*end == 'T')multiplier = 1000LL * 1000LL * 1000LL * 1000LL;
    
    else return false;
    
    while (*end != ' ' && end >= model) end--;
    
    char * uninteresting;
    double freq = strtod(end + 1, &uninteresting);
    
    if (freq == 0.0) return false;

    *time = ((double)1.0) / (freq * (double)multiplier);
    return true;
}

[[maybe_unused]] auto cpu::read_HW_tick_time() -> double {
    if (!cpu::supports_invariantTSC()) cpu::fatal_error("TSC may not be invariant. Use another clock!");
    double res;
    if (cpu::extract_leaf_15H(&res)) return res;
    if (cpu::read_HW_tick_from_name(&res)) return res;
    return cpu::measure_TSC_tick();
}

auto  cpu::measure_clock_granularity() -> uint64_t {
    uint64_t delta = std::numeric_limits<uint64_t>::max();

    for (int i = 0; i < 50; i++) {
        uint64_t m1 = cpu::read_cycle_count();
        uint64_t m2 = cpu::read_cycle_count();
        uint64_t m3 = cpu::read_cycle_count();
        uint64_t m4 = cpu::read_cycle_count();
        uint64_t m5 = cpu::read_cycle_count();
        uint64_t m6 = cpu::read_cycle_count();
        uint64_t m7 = cpu::read_cycle_count();
        uint64_t m8 = cpu::read_cycle_count();
        uint64_t m9 = cpu::read_cycle_count();
        uint64_t m10 = cpu::read_cycle_count();

        auto d = (m2 - m1);
        if (d != 0) delta = std::min(d, delta);
        d = (m3 - m2);
        if (d != 0) delta = std::min(d, delta);
        d = (m4 - m3);
        if (d != 0) delta = std::min(d, delta);
        d = (m5 - m4);
        if (d != 0) delta = std::min(d, delta);
        d = (m6 - m5);
        if (d != 0) delta = std::min(d, delta);
        d = (m7 - m6);
        if (d != 0) delta = std::min(d, delta);
        d = (m8 - m7);
        if (d != 0) delta = std::min(d, delta);
        d = (m9 - m8);
        if (d != 0) delta = std::min(d, delta);
        d = (m10 - m9);
        if (d != 0) delta = std::min(d, delta);
    }
    return delta;
}

/**
 * \brief Gets the cpu core information both (Logical and Physical) from CPUID
 *             Also checks Hyper-Threading support
 */
auto cpu::get_both_cores() -> void {
    __asm__("mov $0x1, %eax\n\t");
    __asm__("cpuid\n\t");
    __asm__("mov %%eax, %0\n\t":"=r" (cpu::cores_register[0x0]));
    __asm__("mov %%ebx, %0\n\t":"=r" (cpu::cores_register[0x1]));
    __asm__("mov %%ecx, %0\n\t":"=r" (cpu::cores_register[0x2]));
    __asm__("mov %%edx, %0\n\t":"=r" (cpu::cores_register[0x3]));

    std::uint32_t cpu_features = cpu::cores_register[0x3];
    std::uint32_t logical_cores = (cpu::cores_register[0x1] >> 0x10) & 0xff;
    std::cout << "Logical: " << logical_cores << std::endl;
    std::uint32_t physical_cores = logical_cores;

    if (cpu::vendor_id() == "GenuineIntel") {
        __asm__("xor %eax, %eax\n\t");
        __asm__("xor %ebx, %ebx\n\t");
        __asm__("xor %ecx, %ecx\n\t");
        __asm__("xor %edx, %edx\n\t");

        __asm__("mov $0x4, %eax\n\t");
        __asm__("cpuid\n\t");
        __asm__("mov %%eax, %0\n\t":"=r" (cpu::cores_register[0x0]));
        __asm__("mov %%ebx, %0\n\t":"=r" (cpu::cores_register[0x1]));
        __asm__("mov %%ecx, %0\n\t":"=r" (cpu::cores_register[0x2]));
        __asm__("mov %%edx, %0\n\t":"=r" (cpu::cores_register[0x3]));

        physical_cores = ((std::uint32_t)(cpu::cores_register[0x0] >> 0x1A) & 0x3f) + 0x1;

    } else if (cpu::vendor_id() == "AuthenticAMD") {
        __asm__("xor %eax, %eax\n\t");
        __asm__("xor %ebx, %ebx\n\t");
        __asm__("xor %ecx, %ecx\n\t");
        __asm__("xor %edx, %edx\n\t");

        __asm__("mov $0x80000008, %eax\n\t");
        __asm__("cpuid\n\t");
        __asm__("mov %%eax, %0\n\t":"=r" (cpu::cores_register[0x0]));
        __asm__("mov %%ebx, %0\n\t":"=r" (cpu::cores_register[0x1]));
        __asm__("mov %%ecx, %0\n\t":"=r" (cpu::cores_register[0x2]));
        __asm__("mov %%edx, %0\n\t":"=r" (cpu::cores_register[0x3]));
        physical_cores = ((std::uint32_t)(cpu::cores_register[0x2] & 0xff)) + 0x1;
    }

    std::cout << "Physical: " << physical_cores << std::endl;

    bool has_hyper_threads = cpu_features & (0x1 << 0x1C) && physical_cores < logical_cores;
    std::cout << "Hyper-Threads: " << (has_hyper_threads ? "true" : "false") << std::endl;
}

/**
 * \brief "mov $0x1 , %eax" will give processor features
 *             and model related information (EAX=1)
 */
auto cpu::instruction_set_checker() -> void {
#if defined(X86)
    __asm__("mov $0x1, %eax\n\t");
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
#endif
}

auto cpu::print_instructions() -> void {
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
    #include <fstream>
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

    for (std::uint32_t values { 0x1 }; values <= 0x3; ++values) cpu::model_name(values);
#endif
}
