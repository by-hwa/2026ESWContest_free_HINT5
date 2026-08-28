#include "ld2450.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <asm/termbits.h>
#include <cstring>

namespace {

constexpr int BAUD = 256000;
constexpr int FRAME_SIZE = 30;

const uint8_t HEADER[4] = {0xAA, 0xFF, 0x03, 0x00};
const uint8_t FOOTER[2] = {0x55, 0xCC};

}  // namespace


LD2450::LD2450(const char* port)
{
    fd_ = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (fd_ < 0) {
        return;
    }

    // B256000 상수가 없으므로 BOTHER로 임의 보드레이트 지정
    termios2 tio{};

    if (ioctl(fd_, TCGETS2, &tio) < 0) {
        close(fd_);
        fd_ = -1;
        return;
    }

    tio.c_cflag &= ~CBAUD;
    tio.c_cflag |= BOTHER;
    tio.c_ispeed = BAUD;
    tio.c_ospeed = BAUD;

    tio.c_cflag &= ~PARENB;         // 패리티 없음
    tio.c_cflag &= ~CSTOPB;         // 정지 bit 1
    tio.c_cflag &= ~CSIZE;
    tio.c_cflag |= CS8;             // 8 bit
    tio.c_cflag |= CREAD | CLOCAL;

    tio.c_lflag = 0;                // raw 모드
    tio.c_iflag = 0;
    tio.c_oflag = 0;

    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 0;

    if (ioctl(fd_, TCSETS2, &tio) < 0) {
        close(fd_);
        fd_ = -1;
    }
}


LD2450::~LD2450()
{
    if (fd_ >= 0) {
        close(fd_);
    }
}


int LD2450::decode(uint8_t low, uint8_t high)
{
    // 최상위 bit가 1이면 양수, 0이면 음수
    int value = low | (high << 8);

    if (value & 0x8000) {
        return value - 0x8000;
    }

    return -value;
}


Target LD2450::parseTarget(const uint8_t* data)
{
    Target t;

    // 8 byte가 전부 0이면 Target 없음
    bool allZero = true;

    for (int i = 0; i < 8; ++i) {
        if (data[i] != 0) {
            allZero = false;
            break;
        }
    }

    if (allZero) {
        return t;
    }

    t.valid = true;
    t.x = decode(data[0], data[1]);
    t.y = decode(data[2], data[3]);
    t.speed = decode(data[4], data[5]);

    return t;
}


bool LD2450::readFrame(Target targets[3])
{
    if (fd_ < 0) {
        return false;
    }

    uint8_t chunk[256];
    ssize_t n = read(fd_, chunk, sizeof(chunk));

    if (n > 0) {
        buffer_.insert(buffer_.end(), chunk, chunk + n);
    }

    while (buffer_.size() >= FRAME_SIZE) {

        // Header 탐색
        size_t start = 0;
        bool found = false;

        for (size_t i = 0; i + 4 <= buffer_.size(); ++i) {
            if (std::memcmp(&buffer_[i], HEADER, 4) == 0) {
                start = i;
                found = true;
                break;
            }
        }

        // Header가 다음 데이터와 걸칠 수 있으므로 끝 3 byte만 남김
        if (!found) {
            if (buffer_.size() > 3) {
                buffer_.erase(buffer_.begin(), buffer_.end() - 3);
            }
            return false;
        }

        // Header 앞의 불필요한 데이터 제거
        if (start > 0) {
            buffer_.erase(buffer_.begin(), buffer_.begin() + start);
            continue;
        }

        // 아직 30 byte가 다 안 들어옴
        if (buffer_.size() < FRAME_SIZE) {
            return false;
        }

        // Footer가 틀리면 1 byte만 버리고 재탐색
        if (std::memcmp(&buffer_[28], FOOTER, 2) != 0) {
            buffer_.erase(buffer_.begin());
            continue;
        }

        targets[0] = parseTarget(&buffer_[4]);
        targets[1] = parseTarget(&buffer_[12]);
        targets[2] = parseTarget(&buffer_[20]);

        buffer_.erase(buffer_.begin(), buffer_.begin() + FRAME_SIZE);

        return true;
    }

    return false;
}