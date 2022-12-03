/*
 * (C)opyright 2022 Ramiz Abbasov <ramizna@code.edu.az>
 * See LICENSE file for license details
 */

#pragma once
#ifndef CUBE_CPU_HPP
#define CUBE_CPU_HPP
#define CPU_INFO "/proc/cpuinfo"

class cpu {
public:
    static inline std::uint32_t vendor_output[0x3];
    static inline std::uint32_t register_output[0xA];

    static auto get_cpu_id() -> void;
    static auto vendor_id() -> void;
    static auto model_name(std::uint32_t eax_values) -> void;
};
#endif //CUBE_CPU_HPP
