#pragma once

#include <exception>
#include <mutex>
#include <string_view>

#include "../util/RingBuffer.h"

class Exception : public std::exception
{
  public:
    Exception(const char *msg);
    Exception(std::string_view msg);
    Exception(const char *prefix, const char *msg);
    Exception(const char *prefix, std::string_view sv);

    const char *what() const noexcept override;

  private:
    static inline RingBuffer buffer{4096};
    static inline std::mutex buffer_mutex;

    const char *message = 0;

    void store_prefixed_msg(const char *prefix, const char *msg);
};
