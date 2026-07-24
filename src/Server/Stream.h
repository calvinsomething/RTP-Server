#pragma once

extern "C"
{
#include <libavformat/avformat.h>
}

class Stream
{
    static constexpr int64_t VIDEO_TIMESTAMP_FREQUENCY = 90000;

  public:
    using MediaType = AVMediaType;
    struct Packet
    {
        int64_t timestamp = 0;
        int size = 0;
        uint8_t *data = 0;
    };

    Stream() = default;
    ~Stream();
    Stream(Stream &&other);

    Stream(Stream &other) = delete;
    void operator=(Stream &other) = delete;
    void operator=(Stream &&other) = delete;

    void load(const char *file_name, MediaType media_type);

    void jump_to(float timestamp);
    float sample_rate_to_npt(int64_t timestamp);

    Packet read_frame();

  private:
    AVFormatContext *ctx = 0;
    AVStream *stream = 0;
    AVPacket *packet = 0;

    int stream_index = 0;
    int64_t timestamp_frequency = 0;
};
