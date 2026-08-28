#include <chrono>
#include <cstdio>
#include <thread>

#include "radar_trigger.h"
#include "spatial_audio.h"

int main(int argc, char* argv[])
{
    const char* port = (argc > 1) ? argv[1] : "/dev/ttyUSB0";
    const char* sound = (argc > 2) ? argv[2] : "./sounds/warning.wav";

    RadarTrigger radar(port);

    if (!radar.isOpen()) {
        std::fprintf(stderr, "Radar: 포트 열기 실패 (%s)\n", port);
        return 1;
    }

    SpatialAudio audio;

    if (!audio.init(sound)) {
        std::fprintf(stderr, "Audio: 초기화 실패 (%s)\n", sound);
        return 1;
    }

    std::printf("Rear warning started (Ctrl+C to stop)\n");

    while (true) {
        for (const auto& e : radar.poll()) {
            std::printf(">>> EVENT  X %+.2f m  Y %.2f m\n", e.x, e.y);
            audio.playRear(e.x, e.y);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}