#include "vlm_model.hpp"

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

VLMModel::VLMModel(const std::string& model_path, const std::string& mmproj_path,
                   const std::string& executable)
    : model_path_(model_path.empty() ? envOrDefault("VLM_MODEL_PATH", "models/smolvlm.gguf") : model_path),
      mmproj_path_(mmproj_path.empty() ? envOrDefault("VLM_MMPROJ_PATH", "models/mmproj.gguf") : mmproj_path),
      executable_(executable.empty() ? envOrDefault("VLM_CLI", "llama-mtmd-cli") : executable)
{
}

bool VLMModel::initialize()
{
    if (!std::filesystem::is_regular_file(model_path_)) {
        std::cerr << "[VLMModel] Model file not found: " << model_path_ << '\n';
        return false;
    }
    if (!std::filesystem::is_regular_file(mmproj_path_)) {
        std::cerr << "[VLMModel] Projector file not found: " << mmproj_path_ << '\n';
        return false;
    }

    initialized_ = true;
    std::cout << "[VLMModel] Ready (model is loaded by " << executable_ << ")\n";
    return true;
}

std::string VLMModel::infer(const std::string& image_path, const std::string& prompt)
{
    if (!initialized_ || !std::filesystem::is_regular_file(image_path)) {
        std::cerr << "[VLMModel] Model is not ready or image is missing\n";
        return "";
    }

    try {
        const ProcessResult result = runProcess({
            executable_, "-m", model_path_, "--mmproj", mmproj_path_,
            "--image", image_path, "-p", prompt, "-n", "128"
        });
        if (result.exit_code != 0) {
            std::cerr << "[VLMModel] Inference process failed (exit "
                      << result.exit_code << ")\n";
            return "";
        }
        return trimOutput(result.output);
    }
    catch (const std::exception& error) {
        std::cerr << "[VLMModel] " << error.what() << '\n';
        return "";
    }
}
