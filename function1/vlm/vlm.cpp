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
        "Give one short description sentence.";

    std::cout << "[VLM] Prompt: "
              << prompt << '\n'; 

    return model_.infer(image_path, prompt);
}

std::string VLM::answer(
    const std::string& image_path,
    const std::string& question)
{
    std::cout << "[VLM] Question: "
              << question << '\n';

    const std::string prompt =
        "Answer in one short sentence: " + question;

    std::cout << "[VLM] Prompt: "
              << prompt << '\n';

    return model_.infer(image_path, prompt);
}
