#pragma once

#include "Server/RTSPResponse.h"

// Requests
static inline const char describe_test_message[] = "DESCRIBE rtsp://192.168.1.100:8554/stream1 RTSP/1.0\n"
                                                   "CSeq: 2\n"
                                                   "User-Agent: LibVLC/3.0.18 (Live555 Streaming Media v2016.11.28)\n"
                                                   "Accept: application/sdp\n\n";

static inline const char duplicate_header_test[] = "DESCRIBE rtsp://192.168.1.100:8554/stream1 RTSP/1.0\n"
                                                   "Accept: application/sdp\n"
                                                   "Accept: application/rtsl, text/html\n\n";

static inline const char broken_header_message[] = "DESCRIBE rtsp://192.168.1.100:8554/stream1 RTSP/1.0\n"
                                                   "BROKEN HEADER\n\n";

static inline const char options_test_message[] = "OPTIONS rtsp://192.168.1.100:8554/stream1 RTSP/1.0\n"
                                                  "CSeq: 2\n"
                                                  "User-Agent: LibVLC/3.0.18 (Live555 Streaming Media v2016.11.28)\n\n";

static inline const char setup_test_message[] = "SETUP rtsp://example.com/stream/trackID=0 RTSP/1.0\n"
                                                "CSeq: 302\n"
                                                "Transport: RTP/AVP;unicast;client_port=4588-4589\n\n";

// Responses
static inline const char *options_test_response_parts[] = {
    "RTSP/1.0 200 OK",
    "CSeq: 2",
    "Public: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE",
};

static inline const char *describe_test_response_parts[] = {
    "RTSP/1.0 200 OK", "CSeq: 2", "Content-Type: application/sdp", "Content-Length: 351", HARD_CODED_SDP.data(),
};

static inline const char *setup_test_response_parts[] = {
    "RTSP/1.0 200 OK\n",
    "CSeq: 302\n",
    "Date: ",
    "Session: ",
    "Transport: RTP/AVP;unicast;client_port=4588-4589;server_port=5004-5005\n",
};
