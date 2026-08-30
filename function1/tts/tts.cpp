#include "tts.hpp"

#include "../process_runner.hpp"

#include <cstdlib>
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
        "espeak-ng", "-v", "en-us", text
    });

    if (result.exit_code != 0)
    {
        std::cerr << "[TTS] eSpeak NG failed\n";
    }
}

void TTS::playListeningTone()
{
    const char* configured_path = std::getenv("NOTIFICATION_SOUND_PATH");
    const std::string sound_path =
        (configured_path != nullptr && *configured_path != '\0')
            ? configured_path
            : "spatial_audio.cpp/sounds/message_sound.wav";

    const ProcessResult result = runProcess({
        "aplay", "-q", sound_path
    });

    if (result.exit_code != 0)
    {
        std::cerr << "[TTS] Listening tone failed: "
                  << sound_path << '\n';
    }
}
