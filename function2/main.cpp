#include "ld2450.h"
#include "radar_trigger.h"
#include "spatial_audio.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

// 사용자 기준 좌우 위치를 막대로 표시
std::string positionBar(int radarX)
{
    // 후방 배치이므로 반전
    const float userX = -radarX / 1000.0f;

    int pos = static_cast<int>(
        (userX + 3.0f) / 6.0f * 40.0f
    );

    pos = std::max(0, std::min(40, pos));

    std::string bar(41, '-');

    bar[20] = '|';
    bar[pos] = 'O';

    return bar;
}

}  // namespace


int main(int argc, char* argv[])
{
    bool verbose = false;

    std::string port = "/dev/ttyUSB0";
    std::string sound =
        "./function2/sounds/warning.wav";

    std::vector<std::string> args;


    for (int i = 1; i < argc; ++i) {

        const std::string arg = argv[i];

        if (arg == "-v" || arg == "--verbose") {
            verbose = true;
        }
        else {
            args.push_back(arg);
        }
    }


    if (args.size() > 0) {
        port = args[0];
    }

    if (args.size() > 1) {
        sound = args[1];
    }


    try {

        LD2450 sensor(port);
        RadarTrigger radar;
        SpatialAudio audio(sound);


        std::cout
            << "Rear warning started (Ctrl+C to stop)\n";

        std::cout
            << "0.3~1.0m 0.10s | "
            << "~1.2m 0.15s | "
            << "~1.5m 0.3s | "
            << "~2.0m 0.6s | "
            << "~3.0m 1.2s | "
            << "~6.0m 2.5s\n";

        std::cout
            << "0.3m 미만 / 6.0m 초과 알림 없음, "
            << "3초 이상 정지 시 중단\n\n";


        while (true) {

            auto targets = sensor.readTargets();

            auto result = radar.update(targets);


            // 공간음향 경고 재생
            for (const auto& event : result.events) {

                audio.playRear(
                    event.x,
                    event.y
                );
            }


            // Debug 출력
            for (const auto& item : result.debug) {

                if (!item.has_value()) {
                    continue;
                }


                const auto& d = *item;


                // 일반 실행에서는 경고 발생 시에만 출력
                // -v에서는 모든 판정 출력
                if (!verbose && !d.triggered) {
                    continue;
                }


                std::cout
                    << (d.triggered ? ">>> " : "    ")
                    << "T" << d.slot
                    << " [" << positionBar(d.x) << "]"
                    << "  d "
                    << std::fixed
                    << std::setprecision(2)
                    << (d.distance / 1000.0)
                    << " m"
                    << "  int "
                    << d.interval
                    << " s"
                    << (d.approaching
                        ? "  [접근]"
                        : "  [비접근]")
                    << (d.stationary
                        ? "  [정지]"
                        : "")
                    << std::endl;


                // 기존처럼 첫 번째 유효 Target만 출력
                break;
            }
        }
    }
    catch (const std::exception& e) {

        std::cerr
            << "ERROR: "
            << e.what()
            << '\n';

        return 1;
    }


    return 0;
}