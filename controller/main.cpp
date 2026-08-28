#include "system_controller.hpp"

#include <iostream>

int main()
{
    std::cout
        << "================================"
        << std::endl;

    std::cout
        << "  AI Assistive System"
        << std::endl;

    std::cout
        << "================================"
        << std::endl;

    SystemController controller;

    if (!controller.initialize())
    {
        std::cerr
            << "System initialization failed."
            << std::endl;

        return 1;
    }

    controller.run();

    controller.shutdown();

    return 0;
}