#pragma once

#include <string>

struct whisper_context;

class WhisperModel
{
public:
    explicit WhisperModel(
        const std::string& model_path = "",
        const std::string& executable = "");

    bool initialize();

    std::string transcribeFile(const std::string& wav_path) const;

private:
    std::string model_path_;
    std::string executable_;
    bool initialized_ = false;
};
