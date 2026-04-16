#pragma once

static inline const char describe_test_message[] = "DESCRIBE rtsp://192.168.1.100:554/stream1 RTSP/1.0\n"
                                                   "CSeq: 2\n"
                                                   "User-Agent: LibVLC/3.0.18 (Live555 Streaming Media v2016.11.28)\n"
                                                   "Accept: application/sdp\n\n";

static inline const char duplicate_header_test[] = "DESCRIBE rtsp://192.168.1.100:554/stream1 RTSP/1.0\n"
                                                   "Accept: application/sdp\n"
                                                   "Accept: application/rtsl, text/html\n\n";

static inline const char *broken_header_message = "DESCRIBE rtsp://192.168.1.100:554/stream1 RTSP/1.0\n"
                                                  "BROKEN HEADER\n\n";
