#pragma once

#include "whisper_model.hpp"

#include <string>

class STT
{
public:
    STT();

    bool initialize();

    std::string recordAndTranscribe();

private:
    WhisperModel whisper_;
    std::string recording_path_;
    int recording_seconds_;
};
