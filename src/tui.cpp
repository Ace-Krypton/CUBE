/*
 * (C)opyright 2022 Ramiz Abbasov <ramizna@code.edu.az>
 * See LICENSE file for license details
 */

#include <string>
#include <thread>
#include <ncurses.h>

#include "cpu.hpp"
#include "tui.hpp"

/**
 * \brief Prints the "|" according to percentage argument
 * @param percent Percentage value
 * @return final std::string as "|"
 */
auto tui::progress_bar(const std::string& percent) -> std::string {
    std::string result = "CPU [";
    int _size = 0x32;

    int boundaries = static_cast<int>((std::stof(percent) / 0x64) * static_cast<float>(_size));

    for (size_t i = 0x0; i < _size; ++i){
        if (i <= boundaries) result += "|";
        else result += " ";
    }

    result += percent.substr(0x0, 0x4) + "%]";
    return result;
}

/**
 * \brief Does the writing part to console
 * @param win Takes WINDOW object instance
 */
auto tui::write_console(WINDOW * win) -> void {
    wattron(win, A_BOLD);
    wattron(win, COLOR_PAIR(0x1));
    mvwprintw(win, 0x1, 0x3, "%s", (tui::progress_bar(cpu::cpu_percentage())).c_str());
    wattron(win, COLOR_PAIR(0x3));
    mvwprintw(win, 0x1, 0xF, "%s", (cpu::print_thermal_state()).c_str());
}

/**
 * \brief Endless while loop for updating value and printing it to the screen
 *      Exits when user presses "CTRL+C" or whatever the binding is
 */
[[noreturn]] auto tui::draw() -> void {
    start_color();
    int xMax;
    [[maybe_unused]] int yMax;
    getmaxyx(stdscr, yMax, xMax);
    WINDOW * sys_win = newwin(0x11, xMax - 0x1, 0x0, 0x0);
    init_pair(0x1, COLOR_GREEN, COLOR_BLACK);
    init_pair(0x2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(0x3, COLOR_RED, COLOR_BLACK);

    while (true) {
        tui::write_console(sys_win);
        wrefresh(sys_win);
        refresh();
        std::this_thread::sleep_for(std::chrono::seconds(0x1));
    }
}
