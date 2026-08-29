#include "button.hpp"

#include <gpiod.h>
#include <iostream>

Button::Button(int gpio_pin, int long_press_ms)
    : gpio_pin_(gpio_pin),
      long_press_ms_(long_press_ms),
      is_pressed_(false),
      long_press_triggered_(false),
      gpio_chip_(nullptr),
      gpio_line_(nullptr)
{
}

bool Button::initialize()
{
    std::cout << "[Button] Initialize GPIO: "
              << gpio_pin_ << std::endl;

    gpio_chip_ = gpiod_chip_open_by_name("gpiochip0");
    if (gpio_chip_ == nullptr)
    {
        std::cerr << "[Button] Failed to open gpiochip0" << std::endl;
        return false;
    }

    gpio_line_ = gpiod_chip_get_line(gpio_chip_, gpio_pin_);
    if (gpio_line_ == nullptr ||
        gpiod_line_request_input(gpio_line_, "button") < 0)
    {
        std::cerr << "[Button] Failed to configure GPIO: "
                  << gpio_pin_ << std::endl;
        gpiod_chip_close(gpio_chip_);
        gpio_chip_ = nullptr;
        gpio_line_ = nullptr;
        return false;
    }

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
    return gpio_line_ != nullptr &&
           gpiod_line_get_value(gpio_line_) == 0;
}
