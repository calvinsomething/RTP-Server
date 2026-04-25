#include "gtest/gtest.h"
#include <chrono>

#include "util/misc.h"

TEST(UtilTest, get_date_string)
{
    std::string date_string = get_date_string(std::chrono::system_clock::time_point(std::chrono::seconds(854033706)));

    ASSERT_EQ(date_string, "23 Jan 1997 15:35:06 GMT");
}
