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
        "이미지를 보고 시각장애인에게 안내하듯 한국어로 짧고 명확하게 설명하세요. "
        "위험 요소, 사람, 장애물, 진행 가능한 방향을 우선 말하세요.";

    return model_.infer(image_path, prompt);
}

std::string VLM::answer(
    const std::string& image_path,
    const std::string& question)
{
    std::cout << "[VLM] Question: "
              << question << '\n';

    const std::string prompt =
        "이미지를 바탕으로 다음 질문에 한국어로 짧고 정확하게 답하세요. "
        "확실하지 않으면 추측하지 마세요. 질문: " + question;

    return model_.infer(image_path, prompt);
}
