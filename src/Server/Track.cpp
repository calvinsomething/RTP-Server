#include "Track.h"

#include <cstdint>

#include "Exception.h"
#include "RTPPacket.h"
#include "Stream.h"
#include "media_files.h"

std::unordered_map<std::string_view, Track::Data> Track::data_by_id{
    {"0", {MediaFiles::sample, Stream::MediaType::AVMEDIA_TYPE_VIDEO}},
    {"1", {MediaFiles::sample, Stream::MediaType::AVMEDIA_TYPE_AUDIO}},
};

Track::Track(in6_addr client_addr, std::string_view uri, std::string_view transport_header_value)
    : transport(client_addr, transport_header_value)
{
    std::string_view id = get_id_from_uri(uri);

    auto file = data_by_id.find(id);
    if (file == data_by_id.end())
    {
        throw Exception("Invalid track ID: ", id);
    }

    stream.load(std::string(file->second.file_name).c_str(), file->second.media_type);

    is_video = file->second.media_type == Stream::MediaType::AVMEDIA_TYPE_VIDEO;

    ssrc = rng.get();
}

std::string_view Track::get_id_from_uri(std::string_view uri)
{
    std::string_view key_str("trackID=");

    size_t i = uri.find(key_str);
    if (i == std::string::npos || i + key_str.size() == uri.size())
    {
        throw Exception("Invalid URI: ", uri);
    }

    return std::string_view(uri.begin() + i + key_str.size(), uri.end());
}

bool Track::send_frames()
{
    float cutoff = play_time + frame_buffer_size_seconds;
    if (play_range_end)
    {
        cutoff = play_range_end < cutoff ? play_range_end : cutoff;
    }

    bool did_send = false;

    while (play_time < cutoff)
    {
        did_send = true;

        Stream::Packet frame = stream.read_frame();
        if (!frame.size || !frame.data)
        {
            // TODO
            // done? always error?
        }

        play_time = stream.sample_rate_to_npt(frame.timestamp);

        int bytes_to_send = frame.size;
        uint8_t *data = frame.data;
        uint32_t offset = 0;

        while (1)
        {
            RTPPacket packet(ssrc, frame.timestamp, bytes_to_send, data, offset, sequence_number++, is_video);

            transport.send(packet.data, packet.length);

            uint32_t bytes_sent = packet.get_bytes_written();

            if (bytes_sent >= uint32_t(bytes_to_send))
            {
                break;
            }

            bytes_to_send -= bytes_sent;
            offset += bytes_sent;
        }
    }

    return did_send;
}

void Track::set_play_time(float npt)
{
    play_time = npt;
}

void Track::set_play_range_end(float npt)
{
    play_range_end = npt;
}
