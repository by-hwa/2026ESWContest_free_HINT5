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
        "One short description sentence, maximum 10 words.";

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
        "One short direct answer, maximum 10 words: " + question;

    std::cout << "[VLM] Prompt: "
              << prompt << '\n';

    return model_.infer(image_path, prompt);
}
