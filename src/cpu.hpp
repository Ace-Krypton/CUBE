/*
 * (C)opyright 2022 Ramiz Abbasov <ramizna@code.edu.az>
 * See LICENSE file for license details
 */

#pragma once
#ifndef CUBE_CPU_HPP
#define CUBE_CPU_HPP
#define CPU_INFO "/proc/cpuinfo/"

struct instruction_set {
    static inline bool has_fpu = false;
    static inline bool has_mmx = false;
    static inline bool has_sse = false;
    static inline bool has_avx = false;
    static inline bool has_sse2 = false;
    static inline bool has_f16c = false;
    static inline bool has_sse3 = false;
    static inline bool has_ssse3 = false;
    static inline bool has_sse4_2 = false;
    static inline bool has_sse4_1 = false;
    static inline bool has_pclmulqdq = false;
};

class cpu {
public:
    static inline std::uint32_t vendor_output[0x3];
    static inline std::uint32_t register_output[0xA];
    static inline std::uint32_t instruction_detection[0x3];

    static auto get_cpu_id() -> void;
    static auto vendor_id() -> std::string;
    static auto get_both_cores() -> void;
    static auto print_instructions() -> void;
    static auto cpu_id(size_t i, unsigned regs[4]) -> void;
    static auto instruction_set_checker() -> void;
    static auto model_name(std::uint32_t eax_values) -> void;
};
#endif //CUBE_CPU_HPP
