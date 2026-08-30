#include "vlm.hpp"
#include "vlm_model.hpp"

#include <iostream>

VLM::VLM(VLMModel& model)
    : model_(model)
{
    std::cout << "[VLM] Initialized\n";
}

std::string VLM::describe(
    const std::string& image_path)
{
    std::cout << "[VLM] Describing image: "
              << image_path << '\n';

    const std::string prompt =
        "Describe the image briefly and clearly in English for a blind user. "
        "Prioritize people, obstacles, hazards, and a safe direction of travel.";

    return model_.infer(image_path, prompt);
}

std::string VLM::answer(
    const std::string& image_path,
    const std::string& question)
{
    std::cout << "[VLM] Question: "
              << question << '\n';

    const std::string prompt =
        "Answer this question about the image briefly and accurately in English. "
        "Do not guess when the image does not provide enough information. "
        "Question: " + question;

    return model_.infer(image_path, prompt);
}
