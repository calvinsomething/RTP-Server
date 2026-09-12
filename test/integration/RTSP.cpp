#include <cstdio>
#include <gtest/gtest.h>

#include <sys/socket.h>
#include <type_traits>

#include "../test_common.h"
#include "RTSPTest.h"

TEST_F(RTSPTest, DescribeRequest)
{
    int result = send(get_fd(), describe_test_message, sizeof(describe_test_message) - 1, 0);
    HANDLE_INT_RESULT(result);

    std::string b(2048, 0);

    result = receive(b.data(), b.size());
    HANDLE_INT_RESULT(result);

    for (auto p : describe_test_response_parts)
    {
        ASSERT_NE(b.find(p), std::string::npos);
    }
}

TEST_F(RTSPTest, SplitDescribeRequest)
{
    int n = sizeof(describe_test_message) - 1;
    size_t half = n / 2;

    HANDLE_INT_RESULT(send(get_fd(), describe_test_message, half, 0));

    HANDLE_INT_RESULT(send(get_fd(), describe_test_message + half, n - half, 0));

    std::string b(2048, 0);
    n = receive(b.data(), b.size());
    HANDLE_INT_RESULT(n);

    for (auto p : describe_test_response_parts)
    {
        ASSERT_NE(b.find(p), std::string::npos);
    }
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

std::string send_setup_request(RTSPTest *test, int (RTSPTest::*get_fd)() const,
                               int (RTSPTest::*receive)(char *buffer, size_t buffer_size))
{
    HANDLE_INT_RESULT(send((test->*get_fd)(), setup_test_message, sizeof(setup_test_message) - 1, 0));

    std::string buffer(2048, 0);

    int n = (test->*receive)(buffer.data(), buffer.size());
    HANDLE_INT_RESULT(n);

    return buffer;
}

TEST_F(RTSPTest, SetupRequest)
{
    std::string response = send_setup_request(this, &std::remove_reference_t<decltype(*this)>::get_fd,
                                              &std::remove_reference_t<decltype(*this)>::receive);

    for (auto p : setup_test_response_parts)
    {
        ASSERT_NE(response.find(p), std::string::npos);
    }
}

TEST_F(RTSPTest, PlayRequest)
{
    std::string response = send_setup_request(this, &std::remove_reference_t<decltype(*this)>::get_fd,
                                              &std::remove_reference_t<decltype(*this)>::receive);

    size_t session_index = response.find("Session: ");

    EXPECT_NE(session_index, std::string::npos);

    session_index += sizeof("Session: ") - 1;

    size_t session_end = response.find('\n', session_index);

    EXPECT_NE(session_end, std::string::npos);

    size_t session_id_len = session_end - session_index;

    auto session_id = response.substr(session_index, session_id_len);

    size_t play_message_size =
        sizeof(play_test_message_fmt) - 2 + session_id_len; // size of test message fmt + session ID

    char *play_message = reinterpret_cast<char *>(alloca(play_message_size));

    memset(play_message, 0, play_message_size);

    std::sprintf(play_message, play_test_message_fmt, session_id.c_str());

    HANDLE_INT_RESULT(send(get_fd(), play_message, play_message_size - 1, 0));

    response.replace(0, response.size(), response.size(), 0);

    int n = receive(response.data(), response.size());
    HANDLE_INT_RESULT(n);

    for (auto p : play_test_response_parts)
    {
        ASSERT_NE(response.find(p), std::string::npos);
    }
}
