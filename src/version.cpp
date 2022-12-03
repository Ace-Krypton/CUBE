/*
 * (C)opyright 2022 Ramiz Abbasov <ramizna@code.edu.az>
 * See LICENSE file for license details
 */
#include <iostream>

#include "version.hpp"

namespace cube {
    inline namespace CUBE_VERSION_STRING {
        auto version() -> std::string {
            return "1.0.0";
        }
    }
}
