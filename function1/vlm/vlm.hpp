#pragma once

#include <string>

class VLM
{
public:
    VLM();

    // 이미지 → 장면 설명
    std::string describe(
        const std::string& image_path);

    // 이미지 + 질문 → 답변
    std::string answer(
        const std::string& image_path,
        const std::string& question);
};