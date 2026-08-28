#include "camera.hpp"

#include <iostream>

Camera::Camera()
{
    std::cout << "[Camera] Initialized\n";
}

std::string Camera::capture()
{
    std::cout << "[Camera] Capturing image...\n";

    // TODO:
    // Raspberry Pi Camera 실제 촬영 코드 구현

    std::string image_path =
        "/tmp/scene.jpg";

    std::cout << "[Camera] Image: "
              << image_path << '\n';

    return image_path;
}