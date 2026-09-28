#include "Stream.h"

#include "Exception.h"
#include "util/misc.h"
#include <iostream>

extern "C"
{
#include <libavutil/error.h>
}

#define HANDLE_AV_ERR(fn)                                                                                              \
    {                                                                                                                  \
        int result = fn;                                                                                               \
        if (result < 0)                                                                                                \
        {                                                                                                              \
            char buf[512] = {};                                                                                        \
            throw Exception(Exception::Prefix{__FILE__ ":" TO_STR(__LINE__) ": "},                                     \
                            av_make_error_string(buf, sizeof(buf), result));                                           \
        }                                                                                                              \
    }

void Stream::load(const char *file_name)
{
    if (avformat_open_input(&ctx, file_name, nullptr, nullptr) < 0)
    {
        throw Exception("Failed to open media file: %s", file_name);
    }
}

void Stream::load(const char *file_name, MediaType media_type)
{
    load(file_name);

    stream_index = av_find_best_stream(ctx, media_type, -1, -1, nullptr, 0);
    if (stream_index < 0)
    {
        char msg[256] = {};
        sprintf(msg, "Failed to find %s stream in input file '%s'", av_get_media_type_string(media_type), file_name);
        throw Exception(msg);
    }

    packet = av_packet_alloc();
    if (!packet)
    {
        throw Exception("Failed to allocate packet");
    }

    stream = ctx->streams[stream_index];

    timestamp_frequency = media_type == AVMEDIA_TYPE_AUDIO ? stream->codecpar->sample_rate : VIDEO_TIMESTAMP_FREQUENCY;
}

std::string Stream::get_sdp()
{
    std::string buffer(512, 0);

    if (av_sdp_create(&ctx, 1, buffer.data(), buffer.size()) < 0)
    {
        throw Exception("Failed to create SDP");
    }

    size_t n = strlen(buffer.data()), count = 0;
    int start = -1;

    for (size_t i = 0; i <= n; ++i)
    {
        if (i == n || buffer[i] == '\r')
        {
            if (start != -1)
            {
                memmove(&buffer[start - count], &buffer[start + 1], i - start);
                ++count;
            }

            start = i;
        }
    }

    return buffer;
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
    if (ctx)
    {
        avformat_close_input(&ctx);
        avformat_free_context(ctx);
    }

    if (packet)
    {
        av_packet_free(&packet);
    }
}

void Stream::jump_to(float timestamp)
{
    HANDLE_AV_ERR(
        av_seek_frame(ctx, stream_index, av_rescale_q(timestamp * timestamp_frequency, {1, 1}, stream->time_base), 0));
}

float Stream::sample_rate_to_npt(int64_t timestamp)
{
    return float(timestamp) / stream->codecpar->sample_rate;
}

Stream::Packet Stream::read_frame()
{
    int result;
    while ((result = av_read_frame(ctx, packet)) >= 0)
    {
        if (packet->stream_index == stream_index)
        {
            return {
                av_rescale_q(packet->pts, stream->time_base, {1, int(timestamp_frequency)}),
                packet->size,
                packet->data,
            };
        }
    }

    av_packet_unref(packet);

    return {};
}
