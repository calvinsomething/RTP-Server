#pragma once

#include <charconv>
#include <cstddef>
#include <string>
#include <type_traits>

template <typename T, size_t N> constexpr size_t array_size(T (&)[N])
{
    return N;
}

template <typename T, typename std::enable_if<std::is_integral<T>::value>> std::string parse_int(T i)
{
    std::string buffer(20, 0);
    std::to_chars(buffer.begin(), buffer.end(), i);
    return buffer;
}

template <typename T, typename std::enable_if<std::is_floating_point<T>::value>> std::string parse_float(T f)
{
    std::string buffer(48, 0);
    std::to_chars(buffer.begin(), buffer.end(), f);
    return buffer;
}
