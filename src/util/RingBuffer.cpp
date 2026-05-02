#include "RingBuffer.h"

#include <algorithm>

RingBuffer::RingBuffer(size_t size) : n(size)
{
    buffer = static_cast<char *>(std::malloc(n));
}

RingBuffer::~RingBuffer()
{
    std::free(buffer);
}

const char *RingBuffer::write(const char *str)
{
    return write(str, std::strlen(str));
}

const char *RingBuffer::write_as_c_str(const char *bytes, size_t size)
{
    char *dest = get_write_dest(size + 1);

    std::memcpy(dest, bytes, std::min(size, n));
    dest[size] = 0;

    return dest;
}

const char *RingBuffer::write(const char *bytes, size_t size)
{
    return write(const_cast<char *>(bytes), size);
}

char *RingBuffer::write(char *bytes, size_t size)
{
    char *dest = get_write_dest(size);

    std::memcpy(dest, bytes, std::min(size, n));

    return dest;
}

const char *RingBuffer::write(const char **parts, size_t *sizes, size_t count)
{
    return write(const_cast<char **>(parts), sizes, count);
}

char *RingBuffer::write(char **parts, size_t *sizes, size_t count)
{
    size_t total_size = 0;
    for (size_t i = 0; i < count; ++i)
    {
        total_size += sizes[i];
    }

    char *dest = get_write_dest(total_size), *position = dest;

    size_t size_limit = n;

    for (size_t i = 0; i < count && size_limit; ++i)
    {
        size_t write_size = std::min(sizes[i], size_limit);

        std::memcpy(position, parts[i], write_size);

        size_limit -= write_size;
        position += write_size;
    }

    return dest;
}

char *RingBuffer::write_as_c_str(const char **parts, size_t *sizes, size_t count)
{
    size_t total_size = 0;
    for (size_t i = 0; i < count; ++i)
    {
        total_size += sizes[i];
    }

    char *dest = get_write_dest(total_size + 1), *position = dest;

    size_t size_limit = n;

    for (size_t i = 0; i < count && size_limit; ++i)
    {
        size_t write_size = std::min(sizes[i], size_limit);

        std::memcpy(position, parts[i], write_size);

        size_limit -= write_size;
        position += write_size;
    }

    dest[total_size] = 0;

    return dest;
}

char *RingBuffer::get_write_dest(size_t size)
{
    size_t contiguous_count = n - i, start = 0;
    if (size <= contiguous_count)
    {
        start = i;
    }

    return buffer + start;
}
