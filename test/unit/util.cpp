#include "gtest/gtest.h"

#include <chrono>
#include <utility>

#include "util/NPT.h"
#include "util/misc.h"

TEST(UtilTest, get_date_string)
{
    std::string date_string =
        util::get_date_string(std::chrono::system_clock::time_point(std::chrono::seconds(854033706)));

    ASSERT_EQ(date_string, "23 Jan 1997 15:35:06 GMT");
}

TEST(UtilTest, split)
{
    std::vector<std::string_view> parts = util::split("RTP/AVP;unicast;client_port=4588-4589", ';');

    ASSERT_EQ(parts.size(), 3);

    std::string_view parts_eq[] = {"RTP/AVP", "unicast", "client_port=4588-4589"};

    for (size_t i = 0; i < parts.size(); ++i)
    {
        ASSERT_EQ(parts[i], parts_eq[i]);
    }
}

TEST(UtilTest, format_npt_sec)
{
    NPT npt(15.25);

    ASSERT_EQ(npt.get_time(), "15.25");
}

TEST(UtilTest, format_npt_range)
{
    NPT a(15.25), b(20);

    auto npt_range = NPT::format_range(a, b);

    ASSERT_EQ(npt_range, "15.25-20");
}

TEST(UtilTest, format_npt_hhmmss)
{
    NPT npt(/* 1h */ 1 * 3600 + /* 35m */ 35 * 60 + /* 11.5s */ 11.5);
    npt.set_mode(NPT::Mode::HHMMSS);

    ASSERT_EQ(npt.get_time(), "1:35:11");
}
