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
    // Raspberry Pi Camera 실제 촬영 코드 구현 TEST 필

    const std::string image_path = "/tmp/scene.jpg";

    std::string command =
        "rpicam-still "
        "--output " + image_path +
        " --width 640 "
        " --height 480 "
        " --timeout 100 "
        "--nopreview";

    int result = std::system(command.c_str());

    if (result != 0)
    {
        std::cerr << "[Camera] Failed to capture image\n";
        return "";
    }

    std::cout << "[Camera] Image: "
              << image_path << '\n';

    return image_path;
}