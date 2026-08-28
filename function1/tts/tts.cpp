#include "tts.hpp"

#include <iostream>
#include <cstdlib>

TTS::TTS()
{
    std::cout << "[TTS] eSpeak NG initialized\n";
}

void TTS::speak(const std::string& text)
{
    std::cout << "[TTS] "
              << text << '\n';

    std::string command =
        "espeak-ng -v ko+f2 \"" +
        text +
        "\"";

    std::system(command.c_str());
}