/*
 * (C)opyright 2022 Ramiz Abbasov <ramizna@code.edu.az>
 * See LICENSE file for license details
 */

#ifndef CUBE_VERSION_HPP
#define CUBE_VERSION_HPP

namespace cube {
#define CUBE_VERSION_STRING v1_0_0

    inline namespace CUBE_VERSION_STRING {
        [[maybe_unused]] auto version() -> std::string;
    }
}

#endif //CUBE_VERSION_HPP
