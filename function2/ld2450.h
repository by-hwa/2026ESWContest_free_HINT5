#pragma once

#include <cstdint>
#include <vector>

struct Target {
    bool valid = false;
    int x = 0;          // mm
    int y = 0;          // mm
    int speed = 0;      // cm/s
};

class LD2450 {
public:
    LD2450(const char* port = "/dev/ttyUSB0");
    ~LD2450();

    bool isOpen() const { return fd_ >= 0; }

    // Frame 하나를 읽어 targets[3]에 채움
    // Frame이 아직 완성되지 않았으면 false
    bool readFrame(Target targets[3]);

private:
    int fd_ = -1;
    std::vector<uint8_t> buffer_;

    static int decode(uint8_t low, uint8_t high);
    static Target parseTarget(const uint8_t* data);
};