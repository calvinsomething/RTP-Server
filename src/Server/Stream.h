#pragma once

extern "C"
{
#include <libavformat/avformat.h>
}

class Stream
{
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

    Packet read_frame();

  private:
    AVFormatContext *ctx = 0;
    AVStream *stream = 0;
    AVPacket *packet = 0;

    int stream_index = 0;
};
