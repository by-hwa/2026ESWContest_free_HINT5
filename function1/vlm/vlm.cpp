#include "vlm.hpp"

#include <iostream>

VLM::VLM()
{
    std::cout << "[VLM] SmolVLM initialized\n";
}

std::string VLM::describe(
    const std::string& image_path)
{
    std::cout << "[VLM] Describing image: "
              << image_path << '\n';

    // TODO:
    // SmolVLM inference

    return "앞에 사람이 서 있습니다.";
}

std::string VLM::answer(
    const std::string& image_path,
    const std::string& question)
{
    std::cout << "[VLM] Image: "
              << image_path << '\n';

    std::cout << "[VLM] Question: "
              << question << '\n';

    // TODO:
    // SmolVLM multimodal inference

    return "앞에 사람이 서 있습니다.";
}