#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstring>

class RingBuffer
{
  public:
    RingBuffer(size_t size);
    ~RingBuffer();

    char *write(char *bytes, size_t size);
    char *write(char **parts, size_t *sizes, size_t count);
    const char *write(const char *str);
    const char *write(const char *bytes, size_t size);
    const char *write(const char **parts, size_t *sizes, size_t count);

  private:
    char *buffer = 0;
    size_t i = 0, n = 0;

    char *get_write_dest(size_t size);
};
