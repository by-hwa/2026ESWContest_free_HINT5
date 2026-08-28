#pragma once

#include <string>

class Camera
{
public:
    Camera();

    // 사진을 촬영하고 파일 경로 반환
    std::string capture();
};