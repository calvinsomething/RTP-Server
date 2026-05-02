#pragma once

#include <charconv>
#include <chrono>
#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

#define _TO_STR(val) #val
#define TO_STR(val) _TO_STR(val)

template <typename T, size_t N> constexpr size_t array_size(T (&)[N])
{
    return N;
}

template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>> std::string int_to_string(T i)
{
    std::string buffer(20, 0);
    std::to_chars(buffer.begin().base(), buffer.end().base(), i);
    return buffer;
}

template <typename T, typename = std::enable_if_t<std::is_floating_point<T>::value>> std::string float_to_string(T f)
{
    std::string buffer(48, 0);
    std::to_chars(buffer.begin().base(), buffer.end().base(), f);
    return buffer;
}

std::string get_date_string(std::chrono::time_point<std::chrono::system_clock> timestamp);

std::vector<std::string_view> split(std::string_view str, char c);
