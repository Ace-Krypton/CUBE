/*
 * (C)opyright 2022 Ramiz Abbasov <ramizna@code.edu.az>
 * See LICENSE file for license details
 */

#ifndef CUBE_TUI_HPP
#define CUBE_TUI_HPP

class tui {
public:
    [[noreturn]] static auto draw() -> void;
    static auto write_console(WINDOW * win) -> void;
    static auto progress_bar(const std::string& percent) -> std::string;
};

#endif //CUBE_TUI_HPP
