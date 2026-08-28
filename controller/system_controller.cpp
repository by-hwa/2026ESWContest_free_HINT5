#include "system_controller.hpp"

#include <chrono>
#include <iostream>
#include <thread>

SystemController::SystemController()
    : button_(17, 1000),
      running_(false)
{
}

bool SystemController::initialize()
{
    std::cout << "[SystemController] Initialize"
              << std::endl;

    if (!button_.initialize())
    {
        std::cerr
            << "[SystemController] "
            << "Button initialization failed"
            << std::endl;

        return false;
    }

    // TODO:
    // Function 1 initialization
    // Function 2 initialization
    // Audio initialization

    running_ = true;

    std::cout << "[SystemController] "
              << "Initialization complete"
              << std::endl;

    return true;
}

void SystemController::run()
{
    std::cout << "[SystemController] "
              << "System started"
              << std::endl;

    while (running_)
    {
        // -----------------------------
        // 1. Button Event 확인
        // -----------------------------
        ButtonEvent event = button_.update();

        if (event != ButtonEvent::NONE)
        {
            handle_button_event(event);
        }

        // -----------------------------
        // 2. Function 2
        // -----------------------------
        // update_function2();

        // 너무 빠른 polling 방지
        sleep_ms(10);
    }
}

void SystemController::shutdown()
{
    if (!running_)
    {
        return;
    }

    std::cout << "[SystemController] Shutdown"
              << std::endl;

    // TODO:
    // Function 1 shutdown
    // Function 2 shutdown
    // Audio shutdown

    running_ = false;
}

void SystemController::handle_button_event(
    ButtonEvent event)
{
    switch (event)
    {
        case ButtonEvent::SHORT_PRESS:

            std::cout << "[Controller] "
                      << "SHORT PRESS"
                      << std::endl;

            execute_function1_scene();

            break;

        case ButtonEvent::LONG_PRESS:

            std::cout << "[Controller] "
                      << "LONG PRESS"
                      << std::endl;

            execute_function1_question();

            break;

        case ButtonEvent::NONE:
        default:
            break;
    }
}

void SystemController::execute_function1_scene()
{
    std::cout << "[Function1] "
              << "Scene Assistant"
              << std::endl;

    // TODO:
    // Camera capture
    //      ↓
    // SmolVLM
    //      ↓
    // Text
    //      ↓
    // TTS

    std::cout << "[Function1] "
              << "Capture scene"
              << std::endl;
}

void SystemController::execute_function1_question()
{
    std::cout << "[Function1] "
              << "Question mode"
              << std::endl;

    // TODO:
    // Recording notification
    //      ↓
    // STT
    //      ↓
    // Camera image
    //      ↓
    // Text + Image
    //      ↓
    // SmolVLM
    //      ↓
    // TTS

    std::cout << "[Function1] "
              << "Start recording"
              << std::endl;
}

// void SystemController::update_function2()
// {
//     // TODO:
//     // Radar/LiDAR
//     //      ↓
//     // Hazard Detector
//     //      ↓
//     // Spatial Audio
// }

void SystemController::sleep_ms(int milliseconds)
{
    std::this_thread::sleep_for(
        std::chrono::milliseconds(milliseconds));
}