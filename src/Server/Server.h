#pragma once

#include <chrono>
#include <netinet/in.h>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_map>

// 554 is default, but would require root user priveleges
inline constexpr unsigned RTSP_PORT = 8554;

// RFC dictates these must be a consecutive pair
inline constexpr unsigned RTP_PORT = 5004; // must be even number
inline constexpr unsigned RTCP_PORT = 5005;

#define HANDLE_INT_RESULT(result)                                                                                      \
    {                                                                                                                  \
        if (result == -1)                                                                                              \
        {                                                                                                              \
            char buf[512] = {};                                                                                        \
            sprintf(buf, "%s:%d", __FILE__, __LINE__);                                                                 \
            perror(buf);                                                                                               \
            exit(EXIT_FAILURE);                                                                                        \
        }                                                                                                              \
    }

class Server
{
    class Connection
    {
      public:
        Connection(int fd, sockaddr_in6 socket_address);
        ~Connection();

        Connection(const Connection &other) = delete;
        Connection operator=(const Connection &other) = delete;

        Connection(Connection &&other);
        Connection &operator=(Connection &&other);

        int get_fd() const;

        // get_client_ip
        // get_client_port

        void clear_message();
        void append_to_message(const char *b, size_t n);
        std::string_view get_rtsp_header();

      private:
        int fd = 0;
        sockaddr_in6 socket_address{};

        std::chrono::time_point<std::chrono::steady_clock> expires;

        std::string message_buffer;
    };

  public:
    void listen_and_serve();

  private:
    int listener_socket = 0, rtsp_socket = 0, rtp_socket = 0, rtcp_socket = 0;
    int interrupt_fd = 0;

    void start_listener();
    void serve();

    int get_expected_message_length(int connection, char *b, size_t n);

    std::unordered_map<int, Connection> connections;
};

// rtsp_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
// HANDLE_INT_RESULT(rtsp_socket);

// rtp_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
// HANDLE_INT_RESULT(rtp_socket);

// rtcp_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
// HANDLE_INT_RESULT(rtcp_socket);
