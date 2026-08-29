#pragma once

#include <string>

class VLMModel;

class VLM
{
public:
    explicit VLM(VLMModel& model);

    std::string describe(const std::string& image_path);

    std::string answer(
        const std::string& image_path,
        const std::string& question);

private:
    VLMModel& model_;
};