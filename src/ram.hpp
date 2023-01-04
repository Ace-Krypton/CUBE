/*
 * (C)opyright 2022 Ramiz Abbasov <ramizna@code.edu.az>
 * See LICENSE file for license details
 */

#pragma once
#ifndef CUBE_RAM_HPP
#define CUBE_RAM_HPP

struct ram {
public:
    [[maybe_unused]] static auto physmem_total() -> std::int64_t;
    [[maybe_unused]] static auto physmem_available() -> std::int64_t;
};

#endif //CUBE_RAM_HPP
