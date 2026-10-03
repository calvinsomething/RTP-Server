#include "Track.h"

#include <cstdint>

#include "Exception.h"
#include "RTPPacket.h"
#include "Server/Media.h"
#include "Stream.h"

/*
// TODO
// use Stream::load without a media type to get the SDP and store the media types
// m=video 0 RTP/AVP 96
// b=AS:6650
// a=rtpmap:96 H264/90000
// a=fmtp:96 packetization-mode=1; sprop-parameter-sets=Z2QAKqzZQHgCJ+WEAAADAAQAAAMB4Dxgxlg=,aOvhcsiw;
profile-level-id=64002A
// a=control:streamid=0
// m=audio 0 RTP/AVP 97
// b=AS:128
// a=rtpmap:97 MPEG4-GENERIC/48000/2
// a=fmtp:97 profile-level-id=1;mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3; config=119056E500
// a=control:streamid=1
*/

Track::Track(sockaddr_in6 client_addr, std::string_view basename, std::string_view control_id,
             std::string_view transport_header_value)
    : transport(client_addr, transport_header_value)
{
    auto media = Media::by_basename.find(basename);
    if (media == Media::by_basename.end())
    {
        throw Exception(Exception::Prefix{"Invalid presentation name: "}, basename);
    }

    Stream::MediaType stream_type = media->second->stream_type_by_control_id[control_id];

    stream.load(media->second->file_name.c_str(), stream_type);

    is_video = stream_type == Stream::MediaType::AVMEDIA_TYPE_VIDEO;

    ssrc = rng.get();
}

std::string_view Track::get_id_from_uri(std::string_view uri)
{
    std::string_view key_str("a=control:");

    size_t i = uri.find(key_str);
    if (i == std::string::npos || i + key_str.size() == uri.size())
    {
        throw Exception(Exception::Prefix{"Invalid URI: "}, uri);
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

float Track::get_play_time()
{
    return play_time;
}

float Track::set_play_time(float npt)
{
    // TODO clamp npt to valid track play range

    play_time = npt;

    return play_time;
}

float Track::set_play_range_end(float npt)
{
    play_range_end = npt;

    return play_range_end;
}
