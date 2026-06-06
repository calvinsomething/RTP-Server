#pragma once

#include <cstddef>
#include <cstdint>

class RTPPacket
{
    static constexpr uint32_t version_bits = 2 << 30;
    static constexpr uint32_t payload_type_video_bits = 32 << 16;
    static constexpr uint32_t marker_bit = 1 << 23;

    static constexpr size_t max_size = 1400;

  public:
    RTPPacket(int32_t ssrc, int32_t timestamp, int size, uint8_t *data, uint32_t offset, uint16_t sequence_number,
              bool is_video);
    ~RTPPacket();

    uint32_t get_bytes_written();

    uint8_t *data;
    uint32_t length = 0;

  private:
    uint32_t header[3] = {};

    void set_marker();
    void set_sequence_number(uint32_t bits);
};
