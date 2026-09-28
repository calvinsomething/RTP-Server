#pragma once

// Requests
static inline const char describe_test_message[] = "DESCRIBE rtsp://192.168.1.100:8554/sample RTSP/1.0\n"
                                                   "CSeq: 2\n"
                                                   "User-Agent: LibVLC/3.0.18 (Live555 Streaming Media v2016.11.28)\n"
                                                   "Accept: application/sdp\n\n";

static inline const char duplicate_header_test[] = "DESCRIBE rtsp://192.168.1.100:8554/sample RTSP/1.0\n"
                                                   "Accept: application/sdp\n"
                                                   "Accept: application/rtsl, text/html\n\n";

static inline const char broken_header_message[] = "DESCRIBE rtsp://192.168.1.100:8554/sample RTSP/1.0\n"
                                                   "BROKEN HEADER\n\n";

static inline const char options_test_message[] = "OPTIONS rtsp://192.168.1.100:8554/sample RTSP/1.0\n"
                                                  "CSeq: 2\n"
                                                  "User-Agent: LibVLC/3.0.18 (Live555 Streaming Media v2016.11.28)\n\n";

static inline const char setup_test_message[] = "SETUP rtsp://example.com/stream/streamid=0 RTSP/1.0\n"
                                                "CSeq: 302\n"
                                                "Transport: RTP/AVP;unicast;client_port=4588-4589\n\n";

static inline const char play_test_message_fmt[] = "PLAY rtsp://audio.example.com/audio RTSP/1.0\n"
                                                   "CSeq: 302\n"
                                                   "Session: %s\n"
                                                   "Range: npt=10-15\n\n";

static inline const char pause_test_message_fmt[] = "PAUSE rtsp://audio.example.com/audio RTSP/1.0\n"
                                                    "CSeq: 302\n"
                                                    "Session: %s\n\n";

// Responses
static inline const char *options_test_response_parts[] = {
    "RTSP/1.0 200 OK",
    "CSeq: 2",
    "Public: DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE",
};

static inline const char *describe_test_response_parts[] = {
    "RTSP/1.0 200 OK", "CSeq: 2", "Content-Type: application/sdp", "Content-Length: ", R"(v=0
o=- 0 0 IN IP4 127.0.0.1
s=No Name
t=0 0
a=tool:libavformat 58.76.100
m=video 0 RTP/AVP 96
b=AS:6650
a=rtpmap:96 H264/90000
a=fmtp:96 packetization-mode=1; sprop-parameter-sets=Z2QAKqzZQHgCJ+WEAAADAAQAAAMB4Dxgxlg=,aOvhcsiw; profile-level-id=64002A
a=control:streamid=0
m=audio 0 RTP/AVP 97
b=AS:128
a=rtpmap:97 MPEG4-GENERIC/48000/2
a=fmtp:97 profile-level-id=1;mode=AAC-hbr;sizelength=13;indexlength=3;indexdeltalength=3; config=119056E500
a=control:streamid=1)",
};

static inline const char *setup_test_response_parts[] = {
    "RTSP/1.0 200 OK\n",
    "CSeq: 302\n",
    "Date: ",
    "Session: ",
    "Transport: RTP/AVP;unicast;client_port=4588-4589;server_port=5004-5005\n",
};

static inline const char *play_test_response_parts[] = {
    "RTSP/1.0 200 OK\n",
    "CSeq: 302\n",
    "Date: ",
    "Range: npt=10-15\n",
};

static inline const char *pause_test_response_parts[] = {
    "RTSP/1.0 200 OK\n",
    "CSeq: 302\n",
    "Date: ",
};
