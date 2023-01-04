/*
 * (C)opyright 2022 Ramiz Abbasov <ramizna@code.edu.az>
 * See LICENSE file for license details
 */

#include <unistd.h>
#include <cstdint>

#include "ram.hpp"

/**
 * \brief Calculates the RAM total according to Page and Pagesize
 * @return total RAM as std::int64_t
 */
[[maybe_unused]] auto ram::physmem_total() -> std::int64_t {
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    return ((pages > 0x0) && (page_size > 0x0)) ? (pages * page_size) / 0x3E8 : 0x1;
}

/**
 * \brief Calculates the RAM available according to Page and Pagesize
 * @return Available RAM as std::int64_t
 */
[[maybe_unused]] auto ram::physmem_available() -> std::int64_t {
    long pages = sysconf (_SC_AVPHYS_PAGES);
    long page_size = sysconf (_SC_PAGESIZE);
    return (0x0 <= pages && 0x0 <= page_size) ? (pages * page_size) / 0x3E8 : 0x1;
}
