#include "tts.hpp"

#include "../process_runner.hpp"

#include <iostream>

TTS::TTS()
{
    std::cout << "[TTS] eSpeak NG initialized\n";
}

void TTS::speak(const std::string& text)
{
    std::cout << "[TTS] "
              << text << '\n';

    const ProcessResult result = runProcess({
        "espeak-ng", "-v", "ko", text
    });

    if (result.exit_code != 0)
    {
        std::cerr << "[TTS] eSpeak NG failed\n";
    }
}
