/*
 * (C)opyright 2022 Ramiz Abbasov <ramizna@code.edu.az>
 * See LICENSE file for license details
 */

#pragma once
#ifndef CUBE_ACPI_HPP
#define CUBE_ACPI_HPP

struct acpi {
public:
    static inline std::string base_path = "/sys/class/power_supply/";
    [[maybe_unused]] static auto get_battery() -> std::vector<std::string>;
};

#endif //CUBE_ACPI_HPP
