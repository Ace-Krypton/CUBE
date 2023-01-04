/*
 * (C)opyright 2022 Ramiz Abbasov <ramizna@code.edu.az>
 * See LICENSE file for license details
 */

#pragma once
#ifndef CUBE_CPU_HPP
#define CUBE_CPU_HPP

#include <unordered_map>

#define CPU_STAT "/proc/stat"
#define CPU_INFO "/proc/cpuinfo"

typedef long long ll;

struct instruction_set {
    static inline std::unordered_map<std::string, bool> instructions;
};

class cpu {
public:
    static inline std::uint32_t cache[0x4];
    static inline std::uint32_t leaf_extract[0x4];
    static inline std::uint32_t invariantTSC[0x4];
    static inline std::uint32_t vendor_output[0x3];
    static inline std::uint32_t cores_register[0x4];
    static inline std::uint32_t register_output[0xA];
    static inline std::uint32_t instruction_detection[0x3];

    static auto get_cpu_id() -> void;
    static auto get_both_cores() -> void;
    static auto vendor_id() -> std::string;
    static auto print_instructions() -> void;
    static auto measure_TSC_tick() -> double;
    static auto supports_invariantTSC() -> bool;
    static auto cpu_percentage() -> std::string;
    static auto instruction_set_checker() -> void;
    static auto print_thermal_state() -> std::string;
    static auto extract_leaf_15H(double * time) -> bool;
    [[maybe_unused]] static auto get_cache_info() -> void;
    static inline auto read_cycle_count()-> std::uint64_t;
    static auto measure_clock_granularity() -> std::uint64_t;
    static auto model_name(std::uint32_t eax_values) -> void;
    static auto read_HW_tick_from_name(double * time) -> bool;
    [[maybe_unused]] static auto read_HW_tick_time() -> double;
    static auto format_SI(double interval, int width, char unit) -> std::string;
    [[maybe_unused]] [[noreturn]] static auto fatal_error(char const * Format, ...) -> void;
};

#endif //CUBE_CPU_HPP
