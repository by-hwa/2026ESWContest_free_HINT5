import serial

# USB-TTL 연결
PORT = "/dev/ttyUSB0"

# GPIO 직결 시
# PORT = "/dev/serial0"

BAUD = 256000

HEADER = b"\xAA\xFF\x03\x00"
FOOTER = b"\x55\xCC"
FRAME_SIZE = 30


def decode(low, high):
    # 최상위 bit가 1이면 양수, 0이면 음수
    value = low | (high << 8)
    if value & 0x8000:
        return value - 0x8000
    return -value


def parse_target(data):
    if data == bytes(8):
        return None

    return {
        "x": decode(data[0], data[1]),
        "y": decode(data[2], data[3]),
        "speed": decode(data[4], data[5]),
    }


def read_targets():
    ser = serial.Serial(PORT, BAUD, timeout=0.5)
    buffer = bytearray()

    try:
        while True:
            buffer.extend(ser.read(ser.in_waiting or 1))

            while len(buffer) >= FRAME_SIZE:
                start = buffer.find(HEADER)

                if start == -1:
                    del buffer[:-3]
                    break

                if start > 0:
                    del buffer[:start]
                    continue

                frame = bytes(buffer[:FRAME_SIZE])

                if frame[-2:] != FOOTER:
                    del buffer[0]
                    continue

                del buffer[:FRAME_SIZE]

                yield [
                    parse_target(frame[4:12]),
                    parse_target(frame[12:20]),
                    parse_target(frame[20:28]),
                ]
    finally:
        ser.close()