#pragma once

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "util/misc.h"

class RingBuffer
{
  public:
    RingBuffer(size_t size);
    ~RingBuffer();

    template <typename... Args> const char *write(const char *fmt, const Args... args)
    {
        int n = snprintf(nullptr, 0, fmt, args...);
        if (n < 0)
        {
            return DEBUG_STR("RingBuffer::write failed.");
        }

        char *dest = get_write_dest(n + 1);

        if (sprintf(dest, fmt, args...) < 0)
        {
            return DEBUG_STR("RingBuffer::write failed.");
        }

        return dest;
    }

    char *write(char *bytes, size_t size);
    const char *write(const char *bytes, size_t size);

    const char *write_as_c_str(const char *bytes, size_t size);
    char *write_as_c_str(const char **parts, size_t *sizes, size_t count);

  private:
    char *buffer = 0;
    size_t i = 0, n = 0;

    char *get_write_dest(size_t size);
};
