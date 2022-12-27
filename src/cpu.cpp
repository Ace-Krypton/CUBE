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
#include <chrono>
#include <thread>
#include <cstdarg>
#include <iomanip>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <x86intrin.h>

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

#if defined(X86)
/**
 * \brief Formats values to given styles
 * @param interval
 * @param width
 * @param unit
 * @return formatted string after normalising the value into
 * engineering style and using a suitable unit prefix (e.g. ms, us, ns)
 */
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
        os << std::setw(width - 0x3)
           << std::right << "0.00"
           << std::setw(0x3) << unit;
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
            os << std::fixed << std::setprecision(2) << std::setw(width - 0x3)
               << std::right << (negative ? -interval : interval) << std::setw(0x2)
               << range.prefix << std::setw(0x1) << unit;
            return os.str();
        }
    }

    os << std::setprecision(0x2) << std::fixed
       << std::right << std::setw(width - 0x3)
       << interval << std::setw(0x3) << unit;

    return os.str();
}

/**
 * \brief Reads cycle count with assembly instruction
 * \code __asm__ __volatile__ ("rdtsc" : "=a" (some_variable), "=d" (some_other_variable))
 *                                          equals
 * \code __rdtsc() function
 * @return value returned from cpu after instruction
 */
inline auto cpu::read_cycle_count()-> std::uint64_t {
    return __rdtsc();
}

/**
 * \brief Measures time stamp counter tick with steady_clock
 *          as documentation recommends it against hrtime
 * @return tick time
 */
auto cpu::measure_TSC_tick() -> double {
    std::chrono::time_point start = std::chrono::steady_clock::now();
    std::uint64_t start_tick = cpu::read_cycle_count();
    std::chrono::time_point end = start + std::chrono::milliseconds(0x5);
    while (std::chrono::steady_clock::now() < end) { }
    size_t elapsed = cpu::read_cycle_count() - start_tick;
    double tick_time = 5.e-3 / static_cast<double>(elapsed);
    return tick_time;
}

/**
 * \brief Fatal error indicator
 * @param Format type
 * @param ... (Variadic function)
 */
[[maybe_unused]] [[noreturn]] auto cpu::fatal_error(char const * Format, ...) -> void {
    fflush(stdout);
    va_list var_args;
    va_start(var_args, Format);
    vfprintf(stderr, Format, var_args);
    exit(0x1);
}

/**
 * \brief Checks if CPU supports invariant time stamp counter(TSC)
 * @return boolean value
 */
auto cpu::supports_invariantTSC() -> bool {
    __asm__("mov $0x80000007, %eax\n\t");
    __asm__("cpuid\n\t");
    __asm__("mov %%eax, %0\n\t":"=r" (cpu::invariantTSC[0x0]));
    __asm__("mov %%ebx, %0\n\t":"=r" (cpu::invariantTSC[0x1]));
    __asm__("mov %%ecx, %0\n\t":"=r" (cpu::invariantTSC[0x2]));
    __asm__("mov %%edx, %0\n\t":"=r" (cpu::invariantTSC[0x3]));

    return (cpu::invariantTSC[0x3] & (0x1 << 0x8)) != 0x0;
}

/**
 * \brief Extract the value from CPUID information
 * “TSC frequency” = “core crystal clock frequency” * EBX/EAX
 * EAX Bits 31 - 00: An unsigned integer which is the denominator of the TSC/”core crystal clock” ratio
 * EBX Bits 31 - 00: An unsigned integer which is the numerator of the TSC/”core crystal clock” ratio
 * ECX Bits 31 - 00: An unsigned integer which is the nominal frequency of the core crystal clock in Hz
 * EDX Bits 31 - 00: Reserved = 0.
 * @param time
 * @return boolean value
 */
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

    if (cpu::leaf_extract[0x1] == 0x0 || cpu::leaf_extract[0x2] == 0x0) {
        std::cout << "cpuid leaf 15H does not give frequency" << std::endl;
        return false;
    }

    double core_crystal_frequency = cpu::leaf_extract[0x2];
    *time = cpu::leaf_extract[0x0] / (cpu::leaf_extract[0x1] * core_crystal_frequency);
    printf("cpuid leaf 15H: coreCrystal = %g, eax=%u, ebx=%u, ecx=%u "
           "=> %s\n",
           core_crystal_frequency,
           cpu::leaf_extract[0x0],
           cpu::leaf_extract[0x1],
           cpu::leaf_extract[0x2],

           cpu::format_SI(*time, 0x9, 's').c_str());
    return true;
}

/**
 * \brief Try to extract hardware tick from the brand string
 * @param time
 * @return boolean value
 */
auto cpu::read_HW_tick_from_name(double * time) -> bool {
    std::string model_name = cpu::vendor_id();

    if (model_name.find("Apple") != std::string::npos) return false;

    char const * model = model_name.c_str();
    char const * end = model + std::strlen(model) - 0x3;
    std::uint64_t multiplier;
    if (*end == 'M') multiplier = 1000LL * 1000LL;
    else if (*end == 'G') multiplier = 1000LL * 1000LL * 1000LL;
    else if (*end == 'T') multiplier = 1000LL * 1000LL * 1000LL * 1000LL;
    else return false;

    while (*end != ' ' && end >= model) end--;

    char * uninteresting;
    double freq = strtod(end + 0x1, &uninteresting);
    
    if (freq == 0.0) return false;

    *time = ((double)1.0) / (freq * (double)multiplier);
    return true;
}

/**
 * \brief Measures hardware tick time
 * @return value from time stamp counter function
 */
[[maybe_unused]] auto cpu::read_HW_tick_time() -> double {
    if (!cpu::supports_invariantTSC()) cpu::fatal_error("TSC may not be invariant. Use another clock!");
    double res;
    if (cpu::extract_leaf_15H(&res)) return res;
    if (cpu::read_HW_tick_from_name(&res)) return res;
    return cpu::measure_TSC_tick();
}

/**
 * \brief Check whether the clock actually ticks at the same rate as its value is enumerated in
 * @return delta variable
 */
auto cpu::measure_clock_granularity() -> std::uint64_t {
    std::uint64_t delta = std::numeric_limits<std::uint64_t>::max();

    for (size_t i = 0x0; i < 0x32; ++i) {
        std::uint64_t m1 = cpu::read_cycle_count();
        std::uint64_t m2 = cpu::read_cycle_count();
        std::uint64_t m3 = cpu::read_cycle_count();
        std::uint64_t m4 = cpu::read_cycle_count();
        std::uint64_t m5 = cpu::read_cycle_count();
        std::uint64_t m6 = cpu::read_cycle_count();
        std::uint64_t m7 = cpu::read_cycle_count();
        std::uint64_t m8 = cpu::read_cycle_count();
        std::uint64_t m9 = cpu::read_cycle_count();
        std::uint64_t m10 = cpu::read_cycle_count();

        size_t d = (m2 - m1);

        if (d != 0x0) delta = std::min(d, delta);
        d = (m3 - m2);
        if (d != 0x0) delta = std::min(d, delta);
        d = (m4 - m3);
        if (d != 0x0) delta = std::min(d, delta);
        d = (m5 - m4);
        if (d != 0x0) delta = std::min(d, delta);
        d = (m6 - m5);
        if (d != 0x0) delta = std::min(d, delta);
        d = (m7 - m6);
        if (d != 0x0) delta = std::min(d, delta);
        d = (m8 - m7);
        if (d != 0x0) delta = std::min(d, delta);
        d = (m9 - m8);
        if (d != 0x0) delta = std::min(d, delta);
        d = (m10 - m9);
        if (d != 0x0) delta = std::min(d, delta);
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
#endif

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

    instruction_set::instructions["SSE3"] = (cpu::instruction_detection[0x0] & (0x1 << 0x0)) != 0x0;
    instruction_set::instructions["PCLMUL"] = (cpu::instruction_detection[0x0] & (0x1 << 0x1)) != 0x0;
    instruction_set::instructions["DTES64"] = (cpu::instruction_detection[0x0] & (0x1 << 0x2)) != 0x0;
    instruction_set::instructions["MONITOR"] = (cpu::instruction_detection[0x0] & (0x1 << 0x3)) != 0x0;
    instruction_set::instructions["DS_CPL"] = (cpu::instruction_detection[0x0] & (0x1 << 0x4)) != 0x0;
    instruction_set::instructions["VMX"] = (cpu::instruction_detection[0x0] & (0x1 << 0x5)) != 0x0;
    instruction_set::instructions["SMX"] = (cpu::instruction_detection[0x0] & (0x1 << 0x6)) != 0x0;
    instruction_set::instructions["EST"] = (cpu::instruction_detection[0x0] & (0x1 << 0x7)) != 0x0;
    instruction_set::instructions["TM2"] = (cpu::instruction_detection[0x0] & (0x1 << 0x8)) != 0x0;
    instruction_set::instructions["SSSE3"] = (cpu::instruction_detection[0x0] & (0x1 << 0x9)) != 0x0;
    instruction_set::instructions["CID"] = (cpu::instruction_detection[0x0] & (0x1 << 0xA)) != 0x0;
    instruction_set::instructions["SDBG"] = (cpu::instruction_detection[0x0] & (0x1 << 0xB)) != 0x0;
    instruction_set::instructions["FMA"] = (cpu::instruction_detection[0x0] & (0x1 << 0xC)) != 0x0;
    instruction_set::instructions["CX16"] = (cpu::instruction_detection[0x0] & (0x1 << 0xD)) != 0x0;
    instruction_set::instructions["XTPR"] = (cpu::instruction_detection[0x0] & (0x1 << 0xE)) != 0x0;
    instruction_set::instructions["PDCM"] = (cpu::instruction_detection[0x0] & (0x1 << 0xF)) != 0x0;
    instruction_set::instructions["PCID"] = (cpu::instruction_detection[0x0] & (0x1 << 0x11)) != 0x0;
    instruction_set::instructions["DCA"] = (cpu::instruction_detection[0x0] & (0x1 << 0x12)) != 0x0;
    instruction_set::instructions["SSE4_1"] = (cpu::instruction_detection[0x0] & (0x1 << 0x13)) != 0x0;
    instruction_set::instructions["SSE4_2"] = (cpu::instruction_detection[0x0] & (0x1 << 0x14)) != 0x0;
    instruction_set::instructions["X2APIC"] = (cpu::instruction_detection[0x0] & (0x1 << 0x15)) != 0x0;
    instruction_set::instructions["MOVBE"] = (cpu::instruction_detection[0x0] & (0x1 << 0x16)) != 0x0;
    instruction_set::instructions["POPCNT"] = (cpu::instruction_detection[0x0] & (0x1 << 0x17)) != 0x0;
    instruction_set::instructions["TSC"] = (cpu::instruction_detection[0x0] & (0x1 << 0x18)) != 0x0;
    instruction_set::instructions["AES"] = (cpu::instruction_detection[0x0] & (0x1 << 0x19)) != 0x0;
    instruction_set::instructions["XSAVE"] = (cpu::instruction_detection[0x0] & (0x1 << 0x1A)) != 0x0;
    instruction_set::instructions["OSXSAVE"] = (cpu::instruction_detection[0x0] & (0x1 << 0x1B)) != 0x0;
    instruction_set::instructions["AVX"] = (cpu::instruction_detection[0x0] & (0x1 << 0x1C)) != 0x0;
    instruction_set::instructions["F16C"] = (cpu::instruction_detection[0x0] & (0x1 << 0x1D)) != 0x0;
    instruction_set::instructions["RDRAND"] = (cpu::instruction_detection[0x0] & (0x1 << 0x1E)) != 0x0;
    instruction_set::instructions["Hyper-Visor"] = (cpu::instruction_detection[0x0] & (0x1 << 0x1F)) != 0x0;

    instruction_set::instructions["FPU"] = (cpu::instruction_detection[0x1] & (0x1 << 0x0)) != 0x0;
    instruction_set::instructions["VME"] = (cpu::instruction_detection[0x1] & (0x1 << 0x1)) != 0x0;
    instruction_set::instructions["DE"] = (cpu::instruction_detection[0x1] & (0x1 << 0x2)) != 0x0;
    instruction_set::instructions["PSE"] = (cpu::instruction_detection[0x1] & (0x1 << 0x3)) != 0x0;
    instruction_set::instructions["MSR"] = (cpu::instruction_detection[0x1] & (0x1 << 0x5)) != 0x0;
    instruction_set::instructions["PAE"] = (cpu::instruction_detection[0x1] & (0x1 << 0x6)) != 0x0;
    instruction_set::instructions["MCE"] = (cpu::instruction_detection[0x1] & (0x1 << 0x7)) != 0x0;
    instruction_set::instructions["CX8"] = (cpu::instruction_detection[0x1] & (0x1 << 0x8)) != 0x0;
    instruction_set::instructions["APIC"] = (cpu::instruction_detection[0x1] & (0x1 << 0x9)) != 0x0;
    instruction_set::instructions["SEP"] = (cpu::instruction_detection[0x1] & (0x1 << 0xB)) != 0x0;
    instruction_set::instructions["MTRR"] = (cpu::instruction_detection[0x1] & (0x1 << 0xC)) != 0x0;
    instruction_set::instructions["PGE"] = (cpu::instruction_detection[0x1] & (0x1 << 0xD)) != 0x0;
    instruction_set::instructions["MCA"] = (cpu::instruction_detection[0x1] & (0x1 << 0xE)) != 0x0;
    instruction_set::instructions["CMOV"] = (cpu::instruction_detection[0x1] & (0x1 << 0xF)) != 0x0;
    instruction_set::instructions["PAT"] = (cpu::instruction_detection[0x1] & (0x1 << 0x10)) != 0x0;
    instruction_set::instructions["PSE36"] = (cpu::instruction_detection[0x1] & (0x1 << 0x11)) != 0x0;
    instruction_set::instructions["PSN"] = (cpu::instruction_detection[0x1] & (0x1 << 0x12)) != 0x0;
    instruction_set::instructions["CLFLUSH"] = (cpu::instruction_detection[0x1] & (0x1 << 0x13)) != 0x0;
    instruction_set::instructions["DS"] = (cpu::instruction_detection[0x1] & (0x1 << 0x13)) != 0x0;
    instruction_set::instructions["DS"] = (cpu::instruction_detection[0x1] & (0x1 << 0x15)) != 0x0;
    instruction_set::instructions["ACPI"] = (cpu::instruction_detection[0x1] & (0x1 << 0x16)) != 0x0;
    instruction_set::instructions["MMX"] = (cpu::instruction_detection[0x1] & (0x1 << 0x17)) != 0x0;
    instruction_set::instructions["FXSR"] = (cpu::instruction_detection[0x1] & (0x1 << 0x18)) != 0x0;
    instruction_set::instructions["SSE"] = (cpu::instruction_detection[0x1] & (0x1 << 0x19)) != 0x0;
    instruction_set::instructions["SSE2"] = (cpu::instruction_detection[0x1] & (0x1 << 0x1A)) != 0x0;
    instruction_set::instructions["SS"] = (cpu::instruction_detection[0x1] & (0x1 << 0x1B)) != 0x0;
    instruction_set::instructions["HTT"] = (cpu::instruction_detection[0x1] & (0x1 << 0x1C)) != 0x0;
    instruction_set::instructions["TM"] = (cpu::instruction_detection[0x1] & (0x1 << 0x1D)) != 0x0;
    instruction_set::instructions["IA64"] = (cpu::instruction_detection[0x1] & (0x1 << 0x1E)) != 0x0;
    instruction_set::instructions["PBE"] = (cpu::instruction_detection[0x1] & (0x1 << 0x1F)) != 0x0;

#endif
}

/**
 * \brief Calculates the cpu usage from "/proc/cpuinfo" file
 * @return CPU usage as string using std::to_string() method
 */
auto cpu::cpu_percentage() -> std::string {
    std::ifstream stat_file(CPU_STAT);
    std::string line;
    std::getline(stat_file, line);

    std::istringstream iss(line);
    std::string token;
    iss >> token;

    ll user = 0x0, nice = 0x0, system = 0x0, idle = 0x0, iowait = 0x0, irq = 0x0,
            softirq = 0x0, steal = 0x0, guest = 0x0, guest_nice = 0x0;
    iss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

    ll total_time = user + nice + system + idle + iowait + irq + softirq + steal;
    ll idle_time = idle + iowait;

    stat_file.close();

    std::this_thread::sleep_for(std::chrono::seconds(0x1));

    stat_file.open(CPU_STAT);
    std::getline(stat_file, line);

    std::istringstream iss2(line);
    iss2 >> token;
    iss2 >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;

    ll total_time_2 = user + nice + system + idle + iowait + irq + softirq + steal;
    ll idle_time_2 = idle + iowait;

    stat_file.close();

    double usage = (double)(total_time_2 - total_time - (idle_time_2 - idle_time))
                   / (double)(total_time_2 - total_time) * 0x64;

    return std::to_string(usage);
}

/**
 * \brief Prints instructions from instruction set
 */
auto cpu::print_instructions() -> void {
    std::uint64_t counter = 0x0;
    cpu::instruction_set_checker();
    for (auto const & elements : instruction_set::instructions) {
        if (counter == 0x5) {
            std::cout << std::endl;
            counter = 0x0;
        }

        std::cout << " [ " << elements.first;
        switch (elements.first.size()) {
            case 0x2: std::cout << "          "; break;
            case 0x3: std::cout << "         "; break;
            case 0x4: std::cout << "        "; break;
            case 0x5: std::cout << "       "; break;
            case 0x6: std::cout << "      "; break;
            case 0x7: std::cout << "     "; break;
            case 0xB: std::cout << " "; break;
        }
        std::cout << ((elements.second) ? "[Y] ]" : "[N] ]");
        counter++;
    }
    std::cout << std::endl;
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
