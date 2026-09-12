#pragma once

#include <netinet/in.h>

#include <unordered_map>

#include "../util/RNG.h"
#include "RTPTransport.h"
#include "Stream.h"

class Track
{
    static constexpr float frame_buffer_size_seconds = 10;

  public:
    Track(in6_addr client_addr, std::string_view uri, std::string_view transport_header_value);

    RTPTransport transport;

    bool send_frames();

    float get_play_time();
    float set_play_time(float npt);
    float set_play_range_end(float npt);

  private:
    struct Data
    {
        std::string_view file_name;
        Stream::MediaType media_type;
    };

    bool is_video = 0;
    uint16_t sequence_number = 0;
    uint32_t ssrc = 0;

    static RNG<uint32_t> rng;

    static std::unordered_map<std::string_view, Track::Data> data_by_id;

    std::string_view get_id_from_uri(std::string_view uri);

    Stream stream;

    float play_time = 0, play_range_end = 0;
};
