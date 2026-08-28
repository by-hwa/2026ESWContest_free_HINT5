#pragma once

#include <string>

class TTS
{
public:
    TTS();

    void speak(const std::string& text);
};