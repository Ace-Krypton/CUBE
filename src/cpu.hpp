#pragma once

class cpu {
public:
    static inline std::uint32_t vendor_output[0x3];
    static inline std::uint32_t register_output[0xA];

    static auto get_cpu_id() -> void;
    static auto vendor_id() -> void;
    static auto model_name(std::uint32_t eax_values) -> void;
};
