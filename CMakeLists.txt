cmake_minimum_required(VERSION 3.16)

project(ai_assistive_system LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GPIOD REQUIRED IMPORTED_TARGET libgpiod)
pkg_check_modules(OPENAL REQUIRED IMPORTED_TARGET openal)

# function 1
add_executable(ai_assistive
    controller/main.cpp
    controller/button.cpp
    controller/system_controller.cpp
    function1/process_runner.cpp
    function1/scene_assistant.cpp
    function1/camera/camera.cpp
    function1/stt/stt.cpp
    function1/stt/whisper_model.cpp
    function1/tts/tts.cpp
    function1/vlm/vlm.cpp
    function1/vlm/vlm_model.cpp
)

target_include_directories(ai_assistive PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/function1
)

target_link_libraries(ai_assistive PRIVATE PkgConfig::GPIOD)

# function 2
add_executable(rear_warning
    function2/main.cpp
    function2/ld2450.cpp
    function2/radar_trigger.cpp
    function2/spatial_audio.cpp
)

target_link_libraries(rear_warning PRIVATE PkgConfig::OPENAL)
