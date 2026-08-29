#pragma once

#include <string>

struct llama_model;
struct llama_context;

struct mtmd_context;

class VLMModel
{
public:
    VLMModel(
        const std::string& model_path,
        const std::string& mmproj_path,
        const std::string& executable = "");

    bool initialize();

    std::string infer(
        const std::string& image_path,
        const std::string& prompt);

private:
    std::string model_path_;
    std::string mmproj_path_;

    std::string executable_;
    bool initialized_ = false;
};
