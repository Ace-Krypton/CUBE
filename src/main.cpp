/*
 * (C)opyright 2022 Ramiz Abbasov <ramizna@code.edu.az>
 * See LICENSE file for license details
 */

#include <ncurses.h>
#include <experimental/string_view>

auto main([[maybe_unused]] int argc, [[maybe_unused]] const char* argv[]) -> int {
    /*  ------------------------------------  Tests  ------------------------------------  */

    initscr();
    noecho();
    cbreak();
    setlocale(LC_ALL, "");
    endwin();
    /*std::cout << "-------------------------------------------------------------------------" << std::endl;
    bool invariant = cpu::supports_invariantTSC();

    printf("Invariant TSC: %s\n", invariant ? "True" : "False");

    if (!invariant) {
        std::cout << "*** Without invariant TSC rdtsc is not a useful timer for wall clock time" << std::endl;
        return 0x1;
    }

    char const * source;
    double res;

    if (cpu::extract_leaf_15H(&res)) {
        source = "leaf 15H";

    } else if (cpu::read_HW_tick_from_name(&res)) {
        source = "model name string";

    } else {
        res = cpu::measure_TSC_tick();
        source = "measurement";
    }

    printf ("From %s frequency %sz => %s\n",
            source,
            cpu::format_SI(1./res,0x9,'H').c_str(),
            cpu::format_SI(res,0x9,'s').c_str());

    double measured = cpu::measure_TSC_tick();

    printf ("Sanity check against std::chrono::steady_clock gives frequency %sz => %s\n",
            cpu::format_SI(1./measured,0x9,'H').c_str(),
            cpu::format_SI(measured,0x9,'s').c_str());

    uint64_t minTicks = cpu::measure_clock_granularity();
    res = res*(double)minTicks;

    printf ("Measured granularity = %llu tick%s => %sz, %s\n",
            (unsigned long long)minTicks, minTicks != 0x1 ? "s": "", cpu::format_SI(1./res,0x9,'H').c_str(),
            cpu::format_SI(res,0x9,'s').c_str());

    std::cout << "-------------------------------------------------------------------------" << std::endl;
    cpu::get_both_cores();
    std::cout << "-------------------------------------------------------------------------" << std::endl;
    cpu::print_instructions();
    std::cout << "-------------------------------------------------------------------------" << std::endl;
    std::cout << "Cube version: " << cube::version() << std::endl;
    std::cout << cpu::vendor_id() << std::endl;
    cpu::get_cpu_id();
    std::cout << std::endl;
    std::cout << "-------------------------------------------------------------------------" << std::endl;
    std::cout << physmem_total() << std::endl;
    std::cout << physmem_available() << std::endl;
    std::cout << "-------------------------------------------------------------------------" << std::endl;*/
    return 0x0;
}
