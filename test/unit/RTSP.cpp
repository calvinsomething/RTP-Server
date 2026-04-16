#include <gtest/gtest.h>
#include <sys/socket.h>

#include "../test_common.h"
#include "Server/Exception.h"
#include "Server/RTSPRequest.h"

TEST(RTSPRequestTest, ParseRequestLine)
{
    RTSPRequest request(describe_test_message);

    ASSERT_EQ(request.get_method(), "DESCRIBE");

    ASSERT_EQ(request.get_uri(), "rtsp://192.168.1.100:554/stream1");

    ASSERT_EQ(request.get_version(), "RTSP/1.0");
}

TEST(RTSPRequestTest, ParseHeaders)
{
    RTSPRequest request(describe_test_message);

    ASSERT_EQ(request.get_header("cseq"), "2");

    ASSERT_EQ(request.get_header("user-agent"), "LibVLC/3.0.18 (Live555 Streaming Media v2016.11.28)");

    ASSERT_EQ(request.get_header("accept"), "application/sdp");
}

TEST(RTSPRequestTest, DuplicateHeader)
{
    RTSPRequest request(duplicate_header_test);

    ASSERT_EQ(request.get_header("accept"), "application/sdp, application/rtsl, text/html");
}

TEST(RTSPRequestTest, RequestException)
{
    ASSERT_THROW(RTSPRequest{broken_header_message}, Exception);
}
