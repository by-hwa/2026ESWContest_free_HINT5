#include "stt.hpp"

#include "../process_runner.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>

STT::STT()
    : recording_path_("/tmp/ai_assistive_question.wav"),
      recording_seconds_(5)
{
}

bool STT::initialize()
{
    return whisper_.initialize();
}

std::string STT::recordAndTranscribe()
{
    std::cout << "[STT] Recording...\n";

    const char* device = std::getenv("AUDIO_DEVICE");
    const std::string input_device =
        (device != nullptr && *device != '\0') ? device : "default";

    try {
        const ProcessResult recording = runProcess({
            "arecord", "-D", input_device, "-f", "S16_LE", "-r", "16000",
            "-c", "1", "-d", std::to_string(recording_seconds_), recording_path_
        });
        if (recording.exit_code != 0 ||
            !std::filesystem::is_regular_file(recording_path_) ||
            std::filesystem::file_size(recording_path_) <= 44) {
            std::cerr << "[STT] Recording failed\n";
            return "";
        }
        return whisper_.transcribeFile(recording_path_);
    }
    catch (const std::exception& error) {
        std::cerr << "[STT] " << error.what() << '\n';
        return "";
    }
}
