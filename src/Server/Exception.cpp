#include "Exception.h"

#include "../util/misc.h"

Exception::Exception(const char *msg)
{
    std::lock_guard<std::mutex> lock(buffer_mutex);
    message = buffer.write(msg);
}

Exception::Exception(std::string_view msg)
{
    std::lock_guard<std::mutex> lock(buffer_mutex);
    message = buffer.write(msg.begin(), msg.size());
}

Exception::Exception(const char *prefix, const char *msg)
{
    store_prefixed_msg(prefix, msg);
}

Exception::Exception(const char *prefix, std::string_view msg)
{
    store_prefixed_msg(prefix, msg.data());
}

void Exception::store_prefixed_msg(const char *prefix, const char *msg)
{
    const char *parts[] = {prefix, msg};
    size_t sizes[] = {std::strlen(prefix), std::strlen(msg)};

    std::lock_guard<std::mutex> lock(buffer_mutex);
    message = buffer.write(parts, sizes, array_size(parts));
}

const char *Exception::what() const noexcept
{
    return message;
}
