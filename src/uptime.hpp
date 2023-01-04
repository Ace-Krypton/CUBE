/*
 * (C)opyright 2022 Ramiz Abbasov <ramizna@code.edu.az>
 * See LICENSE file for license details
 */

#pragma once
#ifndef CUBE_UPTIME_HPP
#define CUBE_UPTIME_HPP

#define UPTIME "/proc/uptime"

struct uptime {
public:
    [[maybe_unused]] static auto uptime_display() -> void;
};

#endif //CUBE_UPTIME_HPP
