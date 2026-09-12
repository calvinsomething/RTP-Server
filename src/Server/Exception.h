#pragma once

#include <exception>
#include <mutex>
#include <string_view>

#include "../util/RingBuffer.h"

#ifdef THROW_IF_FALSE
#undef THROW_IF_FALSE
#endif
#define THROW_IF_FALSE(condition, msg)                                                                                 \
    {                                                                                                                  \
        if (!condition)                                                                                                \
        {                                                                                                              \
            throw Exception(Exception::Prefix{__FILE__ ":" TO_STR(__LINE__) ": "}, msg);                               \
        }                                                                                                              \
    }

class Exception : public std::exception
{
  public:
    struct Prefix
    {
        const char *value;
    };

    template <typename... Args> Exception(const char *fmt, const Args... args)
    {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        message = buffer.write(fmt, args...);
    }

    Exception(const char *msg);
    Exception(std::string_view msg);
    Exception(Prefix prefix, const char *msg);
    Exception(Prefix prefix, std::string_view sv);

    const char *what() const noexcept override;

  private:
    static inline RingBuffer buffer{4096};
    static inline std::mutex buffer_mutex;

    const char *message = 0;

    void store_prefixed_msg(Prefix prefix, const char *msg);
};
