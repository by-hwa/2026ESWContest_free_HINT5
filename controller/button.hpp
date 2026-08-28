#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <chrono>

enum class ButtonEvent
{
    NONE,
    SHORT_PRESS,
    LONG_PRESS
};

class Button
{
public:
    Button(int gpio_pin, int long_press_ms = 1000);

    bool initialize();
    ButtonEvent update();

private:
    int gpio_pin_;
    int long_press_ms_;

    bool is_pressed_;
    bool long_press_triggered_;

    std::chrono::steady_clock::time_point press_start_time_;

    bool read_gpio();
};

#endif // BUTTON_HPP