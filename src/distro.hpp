/*
 * (C)opyright 2022 Ramiz Abbasov <ramizna@code.edu.az>
 * See LICENSE file for license details
 */

#pragma once
#ifndef CUBE_DISTRO_HPP
#define CUBE_DISTRO_HPP

#define RELEASE "/etc/os-release"

struct distro {
public:
    [[maybe_unused]] static auto distro_display() -> std::string;
};

#endif //CUBE_DISTRO_HPP
