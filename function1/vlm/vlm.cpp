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
        "Describe the image for a blind user in one short English sentence. "
        "Use no more than 12 words. Mention only the most important person, "
        "obstacle, or hazard.";

    return model_.infer(image_path, prompt);
}

std::string VLM::answer(
    const std::string& image_path,
    const std::string& question)
{
    std::cout << "[VLM] Question: "
              << question << '\n';

    const std::string prompt =
        "Answer this question about the image in one short English sentence. "
        "Use no more than 12 words. Do not add details or guess. "
        "Question: " + question;

    return model_.infer(image_path, prompt);
}
