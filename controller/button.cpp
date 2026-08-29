#include "button.hpp"

#include <gpiod.h>
#include <iostream>

Button::Button(int gpio_pin, int long_press_ms)
    : gpio_pin_(gpio_pin),
      long_press_ms_(long_press_ms),
      is_pressed_(false),
      long_press_triggered_(false),
      armed_(false),
      gpio_chip_(nullptr),
      gpio_request_(nullptr)
{
}

Button::~Button()
{
    if (gpio_request_ != nullptr)
    {
        gpiod_line_request_release(gpio_request_);
    }

    if (gpio_chip_ != nullptr)
    {
        gpiod_chip_close(gpio_chip_);
    }
}

bool Button::initialize()
{
    std::cout << "[Button] Initialize GPIO: "
              << gpio_pin_ << std::endl;

    gpio_chip_ = gpiod_chip_open("/dev/gpiochip0");
    if (gpio_chip_ == nullptr)
    {
        std::cerr << "[Button] Failed to open gpiochip0" << std::endl;
        return false;
    }

    gpiod_line_settings* settings = gpiod_line_settings_new();
    gpiod_line_config* line_config = gpiod_line_config_new();
    gpiod_request_config* request_config = gpiod_request_config_new();
    const unsigned int offset = static_cast<unsigned int>(gpio_pin_);

    if (settings == nullptr || line_config == nullptr ||
        request_config == nullptr ||
        gpiod_line_settings_set_direction(
            settings, GPIOD_LINE_DIRECTION_INPUT) < 0 ||
        gpiod_line_settings_set_bias(
            settings, GPIOD_LINE_BIAS_PULL_UP) < 0 ||
        gpiod_line_config_add_line_settings(
            line_config, &offset, 1, settings) < 0)
    {
        std::cerr << "[Button] Failed to configure GPIO: "
                  << gpio_pin_ << std::endl;

        gpiod_request_config_free(request_config);
        gpiod_line_config_free(line_config);
        gpiod_line_settings_free(settings);
        gpiod_chip_close(gpio_chip_);
        gpio_chip_ = nullptr;
        return false;
    }

    gpiod_request_config_set_consumer(request_config, "button");
    gpio_request_ = gpiod_chip_request_lines(
        gpio_chip_, request_config, line_config);

    gpiod_request_config_free(request_config);
    gpiod_line_config_free(line_config);
    gpiod_line_settings_free(settings);

    if (gpio_request_ == nullptr)
    {
        std::cerr << "[Button] Failed to request GPIO: "
                  << gpio_pin_ << std::endl;
        gpiod_chip_close(gpio_chip_);
        gpio_chip_ = nullptr;
        return false;
    }

    // 프로그램 시작 시 이미 LOW인 상태를 새 버튼 입력으로 처리하지 않는다.
    // 버튼을 누른 채 부팅했거나 입력이 안정화되는 동안의 오동작을 막는다.
    is_pressed_ = read_gpio();
    long_press_triggered_ = is_pressed_;

    return true;
}

ButtonEvent Button::update()
{
    const bool pressed = read_gpio();

    // 기동 직후에는 버튼이 한 번 "안 눌림" 상태가 된 것을 확인한 뒤에만
    // 입력을 받는다. 배선/풀업 안정화 과정의 LOW를 버튼 입력으로 막는다.
    if (!armed_)
    {
        if (!pressed)
        {
            armed_ = true;
        }

        return ButtonEvent::NONE;
    }

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
    return gpio_request_ != nullptr &&
           gpiod_line_request_get_value(
               gpio_request_, gpio_pin_) == GPIOD_LINE_VALUE_INACTIVE;
}
