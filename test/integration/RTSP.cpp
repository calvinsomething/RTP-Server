#include <gtest/gtest.h>
#include <sys/socket.h>

#include "../test_common.h"
#include "RTSPTest.h"
#include "Server/RTSPRequest.h"

TEST_F(RTSPTest, DescribeRequest)
{
    HANDLE_INT_RESULT(send(get_fd(), describe_test_message, sizeof(describe_test_message) - 1, 0));

    char b[4098]{};
    int n = receive(b, sizeof(b));
    HANDLE_INT_RESULT(n);

    ASSERT_STREQ(b, "OK");
}

TEST_F(RTSPTest, SplitDescribeRequest)
{
    int n = sizeof(describe_test_message) - 1;
    size_t half = n / 2;

    HANDLE_INT_RESULT(send(get_fd(), describe_test_message, half, 0));

    HANDLE_INT_RESULT(send(get_fd(), describe_test_message + half, n - half, 0));

    char b[4098]{};
    n = receive(b, sizeof(b));
    HANDLE_INT_RESULT(n);

    ASSERT_STREQ(b, "OK");
}
