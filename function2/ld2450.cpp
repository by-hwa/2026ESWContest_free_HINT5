#include "ld2450.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <stdexcept>

#include <asm/termbits.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {

constexpr std::uint8_t HEADER[] = {0xAA, 0xFF, 0x03, 0x00};
constexpr std::uint8_t FOOTER[] = {0x55, 0xCC};
constexpr std::size_t FRAME_SIZE = 30;

}  // namespace


LD2450::LD2450(const std::string& port, int baud)
{
    fd_ = ::open(port.c_str(), O_RDWR | O_NOCTTY);

    if (fd_ < 0) {
        throw std::runtime_error(
            "Failed to open " + port + ": " + std::strerror(errno)
        );
    }

    try {
        configurePort(baud);
    }
    catch (...) {
        ::close(fd_);
        fd_ = -1;
        throw;
    }
}


LD2450::~LD2450()
{
    if (fd_ >= 0) {
        ::close(fd_);
    }
}


void LD2450::configurePort(int baud)
{
    struct termios2 tio {};

    if (::ioctl(fd_, TCGETS2, &tio) < 0) {
        throw std::runtime_error(
            "TCGETS2 failed: " + std::string(std::strerror(errno))
        );
    }

    // Raw 8N1
    tio.c_iflag = 0;
    tio.c_oflag = 0;
    tio.c_lflag = 0;

    tio.c_cflag &= ~(CBAUD | CSIZE | PARENB | CSTOPB);
    tio.c_cflag |= BOTHER | CS8 | CLOCAL | CREAD;

    tio.c_ispeed = baud;
    tio.c_ospeed = baud;

    tio.c_cc[VMIN] = 1;
    tio.c_cc[VTIME] = 5;

    if (::ioctl(fd_, TCSETS2, &tio) < 0) {
        throw std::runtime_error(
            "TCSETS2 failed: " + std::string(std::strerror(errno))
        );
    }
}


int LD2450::decode(std::uint8_t low, std::uint8_t high)
{
    int value = static_cast<int>(low)
              | (static_cast<int>(high) << 8);

    // LD2450 특수 부호 형식
    if (value & 0x8000) {
        return value - 0x8000;
    }

    return -value;
}


RadarTarget LD2450::parseTarget(const std::uint8_t* data)
{
    bool allZero = true;

    for (int i = 0; i < 8; ++i) {
        if (data[i] != 0) {
            allZero = false;
            break;
        }
    }

    if (allZero) {
        return {};
    }

    RadarTarget target;

    target.valid = true;
    target.x = decode(data[0], data[1]);
    target.y = decode(data[2], data[3]);
    target.speed = decode(data[4], data[5]);

    return target;
}


std::array<RadarTarget, 3> LD2450::readTargets()
{
    std::uint8_t temp[256];

    while (true) {

        ssize_t n = ::read(fd_, temp, sizeof(temp));

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }

            throw std::runtime_error(
                "Serial read failed: " + std::string(std::strerror(errno))
            );
        }

        if (n == 0) {
            continue;
        }

        buffer_.insert(buffer_.end(), temp, temp + n);

        while (buffer_.size() >= FRAME_SIZE) {

            auto start = std::search(
                buffer_.begin(),
                buffer_.end(),
                std::begin(HEADER),
                std::end(HEADER)
            );

            // Header가 다음 데이터와 걸칠 수 있으므로 끝 3 byte만 남김
            if (start == buffer_.end()) {
                if (buffer_.size() > 3) {
                    buffer_.erase(buffer_.begin(), buffer_.end() - 3);
                }
                break;
            }

            // Header 앞의 불필요한 데이터 제거
            if (start != buffer_.begin()) {
                buffer_.erase(buffer_.begin(), start);
                continue;
            }

            if (buffer_.size() < FRAME_SIZE) {
                break;
            }

            // Footer가 틀리면 1 byte만 버리고 재탐색
            if (buffer_[FRAME_SIZE - 2] != FOOTER[0]
                || buffer_[FRAME_SIZE - 1] != FOOTER[1]) {
                buffer_.erase(buffer_.begin());
                continue;
            }

            std::array<RadarTarget, 3> targets = {
                parseTarget(&buffer_[4]),
                parseTarget(&buffer_[12]),
                parseTarget(&buffer_[20]),
            };

            buffer_.erase(
                buffer_.begin(),
                buffer_.begin() + FRAME_SIZE
            );

            return targets;
        }
    }
}