/*
 * (C)opyright 2022 Ramiz Abbasov <ramizna@code.edu.az>
 * See LICENSE file for license details
 */

#include <string>
#include <thread>
#include <ncurses.h>

#include "cpu.hpp"
#include "tui.hpp"

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

auto tui::write_console(WINDOW * win) -> void {
    wattron(win,COLOR_PAIR(0x1));
    wattron(win, A_BOLD);
    wprintw(win, "%s", (tui::progress_bar(cpu::cpu_percentage())).c_str());
}

[[noreturn]] auto tui::draw() -> void {
    start_color();
    [[maybe_unused]] int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);
    WINDOW * sys_win = newwin(0x11, xMax - 0x1, 0x0, 0x0);
    init_pair(0x1, COLOR_GREEN, COLOR_BLACK);
    init_pair(0x2, COLOR_GREEN, COLOR_BLACK);
    keypad(sys_win, TRUE);

    while (true) {
        box(sys_win, 0x0, 0x0);
        tui::write_console(sys_win);
        wrefresh(sys_win);
        refresh();
        std::this_thread::sleep_for(std::chrono::seconds(0x1));
    }
}
