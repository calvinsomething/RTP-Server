#include <gtest/gtest.h>
#include <sys/socket.h>

#include "../test_common.h"
#include "RTSPTest.h"

TEST_F(RTSPTest, DescribeRequest)
{
    HANDLE_INT_RESULT(send(get_fd(), describe_test_message, sizeof(describe_test_message) - 1, 0));

    char b[2048]{};
    int n = receive(b, sizeof(b));
    HANDLE_INT_RESULT(n);

    ASSERT_STREQ(b, "DESCRIBE");
}

TEST_F(RTSPTest, SplitDescribeRequest)
{
    int n = sizeof(describe_test_message) - 1;
    size_t half = n / 2;

    HANDLE_INT_RESULT(send(get_fd(), describe_test_message, half, 0));

    HANDLE_INT_RESULT(send(get_fd(), describe_test_message + half, n - half, 0));

    char b[2048]{};
    n = receive(b, sizeof(b));
    HANDLE_INT_RESULT(n);

    ASSERT_STREQ(b, "DESCRIBE");
}

TEST_F(RTSPTest, OptionsRequest)
{
    HANDLE_INT_RESULT(send(get_fd(), options_test_message, sizeof(options_test_message) - 1, 0));

    std::string buffer(2048, 0);

    int n = receive(buffer.data(), buffer.size());
    HANDLE_INT_RESULT(n);

    for (auto p : options_test_response_parts)
    {
        ASSERT_NE(buffer.find(p), std::string::npos);
    }
}
