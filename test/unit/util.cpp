#include "gtest/gtest.h"
#include <chrono>

#include "util/misc.h"

TEST(UtilTest, get_date_string)
{
    std::string date_string = get_date_string(std::chrono::system_clock::time_point(std::chrono::seconds(854033706)));

    ASSERT_EQ(date_string, "23 Jan 1997 15:35:06 GMT");
}

TEST(UtilTest, split)
{
    std::vector<std::string_view> parts = split("RTP/AVP;unicast;client_port=4588-4589", ';');

    ASSERT_EQ(parts.size(), 3);

    std::string_view parts_eq[] = {"RTP/AVP", "unicast", "client_port=4588-4589"};

    for (size_t i = 0; i < parts.size(); ++i)
    {
        ASSERT_EQ(parts[i], parts_eq[i]);
    }
}
