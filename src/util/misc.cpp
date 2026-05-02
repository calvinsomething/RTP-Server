#include "misc.h"

#include <ctime>
#include <iomanip>

std::string get_date_string(std::chrono::time_point<std::chrono::system_clock> timestamp)
{
    auto t = std::chrono::system_clock::to_time_t(timestamp);

    std::stringstream ss;
    ss << std::put_time(gmtime(&t), "%d %b %Y %H:%M:%S %Z");

    return ss.str();
}

std::vector<std::string_view> split(std::string_view str, char c)
{
    std::vector<std::string_view> output;

    for (size_t i = 0; i < str.size();)
    {
        size_t j = str.find(c, i);
        if (j == std::string::npos)
        {
            j = str.size();
        }

        auto begin = str.begin();
        output.push_back({begin + i, begin + j});

        i = j + 1;
    }

    return output;
}
