#include "whisper_model.hpp"

#include "../process_runner.hpp"

#include <cstdlib>
#include <filesystem>
#include <iostream>

namespace {

std::string envOrDefault(const char* name, const char* fallback)
{
    const char* value = std::getenv(name);
    return (value != nullptr && *value != '\0') ? value : fallback;
}

}  // namespace

WhisperModel::WhisperModel(const std::string& model_path, const std::string& executable)
    : model_path_(model_path.empty() ? envOrDefault("WHISPER_MODEL_PATH", "models/ggml-small.bin") : model_path),
      executable_(executable.empty() ? envOrDefault("WHISPER_CLI", "whisper-cli") : executable)
{
}

bool WhisperModel::initialize()
{
    if (!std::filesystem::is_regular_file(model_path_)) {
        std::cerr << "[WhisperModel] Model file not found: " << model_path_ << '\n';
        return false;
    }

    initialized_ = true;
    std::cout << "[WhisperModel] Ready\n";
    return true;
}

std::string WhisperModel::transcribeFile(const std::string& wav_path) const
{
    if (!initialized_ || !std::filesystem::is_regular_file(wav_path)) {
        std::cerr << "[WhisperModel] Model is not ready or recording is missing\n";
        return "";
    }

    try {
        const ProcessResult result = runProcess({
            executable_, "-m", model_path_, "-f", wav_path, "-l", "en", "-nt"
        });
        if (result.exit_code != 0) {
            std::cerr << "[WhisperModel] Transcription process failed (exit "
                      << result.exit_code << ")\n";
            return "";
        }
        return trimOutput(result.output);
    }
    catch (const std::exception& error) {
        std::cerr << "[WhisperModel] " << error.what() << '\n';
        return "";
    }
}
