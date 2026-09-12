#include "Exception.h"

#include "../util/misc.h"

Exception::Exception(const char *msg)
{
    size_t n = std::strlen(msg);

    std::lock_guard<std::mutex> lock(buffer_mutex);
    message = buffer.write_as_c_str(msg, n);
}

Exception::Exception(std::string_view msg)
{
    std::lock_guard<std::mutex> lock(buffer_mutex);
    message = buffer.write_as_c_str(msg.begin(), msg.size());
}

Exception::Exception(Prefix prefix, const char *msg)
{
    store_prefixed_msg(prefix, msg);
}

Exception::Exception(Prefix prefix, std::string_view msg)
{
    store_prefixed_msg(prefix, msg.data());
}

void Exception::store_prefixed_msg(Prefix prefix, const char *msg)
{
    const char *parts[] = {prefix.value, msg};
    size_t sizes[] = {std::strlen(prefix.value), std::strlen(msg)};

    std::lock_guard<std::mutex> lock(buffer_mutex);
    message = buffer.write_as_c_str(parts, sizes, util::array_size(parts));
}

const char *Exception::what() const noexcept
{
    return message;
}
