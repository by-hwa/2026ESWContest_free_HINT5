#pragma once

#include "button.hpp"
#include "../function1/scene_assistant.hpp"

class SystemController
{
public:
    SystemController();

    bool initialize();

    void run();

    void shutdown();

private:
    // -----------------------------
    // Button
    // -----------------------------
    Button button_;
    SceneAssistant scene_assistant_;

    // -----------------------------
    // System State
    // -----------------------------
    bool running_;

    // -----------------------------
    // Event 처리
    // -----------------------------
    void handle_button_event(ButtonEvent event);

    // -----------------------------
    // Function 1
    // -----------------------------
    void execute_function1_scene();

    void execute_function1_question();

    // -----------------------------
    // Function 2
    // -----------------------------
    // void update_function2();

    // -----------------------------
    // Utility
    // -----------------------------
    void sleep_ms(int milliseconds);
};
