#include "Stream.h"

#include "Exception.h"

void Stream::load(const char *file_name, MediaType media_type)
{
    if (avformat_open_input(&ctx, file_name, nullptr, nullptr) < 0)
    {
        throw Exception("Could not open source file: ", file_name);
    }

    stream_index = av_find_best_stream(ctx, media_type, -1, -1, nullptr, 0);
    if (stream_index < 0)
    {
        char msg[256] = {};
        sprintf(msg, "Could not find %s stream in input file '%s'", av_get_media_type_string(media_type), file_name);
        throw Exception(msg);
    }

    AVPacket *packet = av_packet_alloc();
    if (!packet)
    {
        throw Exception("Could not allocate packet");
    }
}

Stream::Stream(Stream &&other)
{
    ctx = other.ctx;
    stream = other.stream;
    packet = other.packet;
    stream_index = other.stream_index;

    other.ctx = 0;
    other.stream = 0;
    other.packet = 0;
    other.stream_index = 0;
}

Stream::~Stream()
{
    av_packet_free(&packet);
}

Stream::Packet Stream::read_frame()
{
    AVStream *stream = ctx->streams[stream_index];

    if (!stream || !stream->codecpar)
    {
        throw Exception("invalid stream");
    }

    while (av_read_frame(ctx, packet) >= 0)
    {
        if (packet->stream_index == stream_index)
        {
            int32_t rtp_timestamp = av_rescale_q(packet->pts, stream->time_base, {1, stream->codecpar->sample_rate});

            return {
                rtp_timestamp,
                packet->size,
                packet->data,
            };
        }
    }

    av_packet_unref(packet);

    return {};
}
