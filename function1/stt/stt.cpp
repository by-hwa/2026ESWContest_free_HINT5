#include "stt.hpp"

#include "../process_runner.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace {

void playRecordingTone(const char* phase)
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
        std::cerr << "[STT] " << phase << " tone failed: "
                  << sound_path << '\n';
    }
}

}  // namespace

STT::STT()
    : recording_path_("/tmp/ai_assistive_question.wav"),
      recording_seconds_(3)
{
}

bool STT::initialize()
{
    return whisper_.initialize();
}

std::string STT::recordAndTranscribe()
{
    playRecordingTone("Recording-start");

    std::cout << "[STT] Recording...\n";

    const char* device = std::getenv("AUDIO_DEVICE");
    const std::string input_device =
        (device != nullptr && *device != '\0') ? device : "default";

    std::cout << "[STT] Input device: " << input_device << '\n';

    try {
        // const ProcessResult recording = runProcess({
        //     "arecord", "-D", input_device, "-f", "cd", "-d",
        //     std::to_string(recording_seconds_), "-t", "wav", recording_path_
        // });
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

        playRecordingTone("Recording-complete");

        const std::string transcription = whisper_.transcribeFile(recording_path_);
        // whisper-cli 버전에 따라 무음은 '-', '[BLANK_AUDIO]' 등으로 출력된다.
        if (transcription == "-" ||
            transcription == "[BLANK_AUDIO]" ||
            transcription == "[BLANK]") {
            return "";
        }

        return transcription;
    }
    catch (const std::exception& error) {
        std::cerr << "[STT] " << error.what() << '\n';
        return "";
    }
}
