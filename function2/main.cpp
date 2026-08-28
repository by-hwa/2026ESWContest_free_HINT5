#include "ld2450.h"
#include "radar_trigger.h"
#include "spatial_audio.h"

#include <iomanip>
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    const std::string port =
        (argc > 1) ? argv[1] : "/dev/ttyUSB0";

    const std::string sound =
        (argc > 2) ? argv[2] : "./sounds/warning.wav";

    try {
        LD2450 sensor(port);
        RadarTrigger radar;
        SpatialAudio audio(sound);

        std::cout << "Rear warning started (Ctrl+C to stop)\n\n";

        while (true) {

            auto targets = sensor.readTargets();
            auto result = radar.update(targets);

            for (const auto& event : result.events) {

                std::cout
                    << ">>> EVENT"
                    << " | X "
                    << std::fixed
                    << std::setprecision(2)
                    << std::showpos
                    << event.x
                    << std::noshowpos
                    << " m"
                    << " | Y "
                    << event.y
                    << " m\n";

                audio.playRear(event.x, event.y);
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