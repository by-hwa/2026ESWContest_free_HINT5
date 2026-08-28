#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

struct RadarTarget {
    bool valid = false;
    int x = 0;       // [mm]
    int y = 0;       // [mm]
    int speed = 0;   // [cm/s]
};

class LD2450 {
public:
    explicit LD2450(
        const std::string& port = "/dev/ttyUSB0",
        int baud = 256000
    );

    ~LD2450();

    LD2450(const LD2450&) = delete;
    LD2450& operator=(const LD2450&) = delete;

    std::array<RadarTarget, 3> readTargets();

private:
    int fd_ = -1;
    std::vector<std::uint8_t> buffer_;

    void configurePort(int baud);

    static int decode(std::uint8_t low, std::uint8_t high);
    static RadarTarget parseTarget(const std::uint8_t* data);
};