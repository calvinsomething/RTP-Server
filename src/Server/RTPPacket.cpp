#include "RTPPacket.h"

#include <string.h>

//	  0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |V=2|P|X|  CC   |M|     PT      |       sequence number         |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |                           timestamp                           |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |           synchronization source (SSRC) identifier            |
//   +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//   |            contributing source (CSRC) identifiers             |
//   |                             ....                              |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |F|NRI|  Type   |                                               |
//  +-+-+-+-+-+-+-+-+                                               |
//  |                                                               |
//  |             one or more aggregation units                     |
//  |                                                               |
//  |                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                               :...OPTIONAL RTP padding        |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// NAL Header / FU Indicator
// +---------------+
// |0|1|2|3|4|5|6|7|
// +-+-+-+-+-+-+-+-+
// |F|NRI|  Type   |
// +---------------+
//
// FU Header
// +---------------+
// |0|1|2|3|4|5|6|7|
// +-+-+-+-+-+-+-+-+
// |S|E|R|  Type   |
// +---------------+

// padding, header extension, CSRC, all == 0
RTPPacket::RTPPacket(int32_t ssrc, int32_t timestamp, int size, uint8_t *data, uint32_t offset,
                     uint16_t sequence_number, bool is_video)
{
    header[0] = version_bits | (is_video * payload_type_video_bits) | sequence_number;
    header[1] = timestamp;
    header[2] = ssrc;

    int remaining = size - offset;

    if (size_t(remaining) > max_size || offset)
    {
        // S = 1 for first fragment
        // E = 1 for last fragment
        // R = 0 reserved bit always ignored
        uint8_t SER = 0;
        if (!offset)
        {
            SER = 1 << 7;
        }
        else if (size_t(remaining) <= max_size)
        {
            SER = 1 << 6;
        }

        constexpr int fu_a_type = 28;

        this->data = new uint8_t[sizeof(header) + max_size + 2];

        // RTP Header
        memcpy(this->data, header, sizeof(header));

        uint8_t nal_header = *data;

        // FU Indicator
        this->data[sizeof(header)] = (nal_header & (15 << 4)) | fu_a_type;

        // FU Header
        this->data[sizeof(header) + 1] = SER | (nal_header & 15);

        // NAL Payload
        memcpy(this->data, data, max_size);

        length = max_size;
    }
    else
    {
        this->data = new uint8_t[sizeof(header) + size];

        memcpy(this->data, header, sizeof(header));

        memcpy(this->data, data, size);

        length = size;
    }
}

RTPPacket::~RTPPacket()
{
    if (data)
    {
        delete[] data;
    }
}

uint32_t RTPPacket::get_bytes_written()
{
    return length;
}
