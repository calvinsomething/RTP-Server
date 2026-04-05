#include <gtest/gtest.h>
#include <sys/socket.h>

#include "RTSPTest.h"

const char initial_message[] = "DESCRIBE rtsp://192.168.1.100:554/stream1 RTSP/1.0\n"
                               "CSeq: 2\n"
                               "User-Agent: LibVLC/3.0.18 (Live555 Streaming Media v2016.11.28)\n"
                               "Accept: application/sdp\n\n";

TEST_F(RTSPTest, ParseInitialHeader)
{
    HANDLE_INT_RESULT(send(get_fd(), initial_message, sizeof(initial_message) - 1, 0));

    char b[4098]{};
    int n = receive(b, sizeof(b));
    HANDLE_INT_RESULT(n);

    std::cout << b << "\n";
}
