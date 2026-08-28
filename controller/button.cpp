#include "button.hpp"

#include <iostream>

Button::Button(int gpio_pin, int long_press_ms)
    : gpio_pin_(gpio_pin),
      long_press_ms_(long_press_ms),
      is_pressed_(false),
      long_press_triggered_(false)
{
}

bool Button::initialize()
{
    std::cout << "[Button] Initialize GPIO: "
              << gpio_pin_ << std::endl;

    // TODO:
    // Raspberry Pi GPIO 초기화

    return true;
}

ButtonEvent Button::update()
{
    const bool pressed = read_gpio();

    // --------------------------------
    // 버튼을 새롭게 누름
    // --------------------------------
    if (pressed && !is_pressed_)
    {
        is_pressed_ = true;
        long_press_triggered_ = false;

        press_start_time_ =
            std::chrono::steady_clock::now();

        return ButtonEvent::NONE;
    }

    // --------------------------------
    // 버튼을 계속 누르고 있음
    // --------------------------------
    if (pressed && is_pressed_)
    {
        if (!long_press_triggered_)
        {
            const auto now =
                std::chrono::steady_clock::now();

            const auto elapsed =
                std::chrono::duration_cast<
                    std::chrono::milliseconds
                >(now - press_start_time_).count();

            if (elapsed >= long_press_ms_)
            {
                long_press_triggered_ = true;

                return ButtonEvent::LONG_PRESS;
            }
        }

        return ButtonEvent::NONE;
    }

    // --------------------------------
    // 버튼을 뗌
    // --------------------------------
    if (!pressed && is_pressed_)
    {
        is_pressed_ = false;

        // Long Press가 이미 발생했다면
        // 버튼을 뗄 때 Short Press를 발생시키지 않음
        if (long_press_triggered_)
        {
            return ButtonEvent::NONE;
        }

        return ButtonEvent::SHORT_PRESS;
    }

    return ButtonEvent::NONE;
}

bool Button::read_gpio()
{
    // TODO:
    // Raspberry Pi GPIO 실제 구현

    return false;
}