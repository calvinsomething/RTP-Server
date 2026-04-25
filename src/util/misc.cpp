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
