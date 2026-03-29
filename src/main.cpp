#include <netinet/in.h>
#include <signal.h>
#include <syncstream>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>

#include "Defer.h"

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

// 554 is default, but would require root user priveleges
constexpr unsigned RTSP_PORT = 8554;

// RFC dictates these must be a consecutive pair
constexpr unsigned RTP_PORT = 5004; // must be even number
constexpr unsigned RTCP_PORT = 5005;

int listener_socket = 0, rtsp_socket = 0, rtp_socket = 0, rtcp_socket = 0;
int interrupt_fd = 0;

// Signal Handling
extern "C"
{
    void interrupt_handler(int signal_number)
    {
        // swallow
    }
}

void set_up_signal_handlers()
{
    struct sigaction input{};
    input.sa_handler = interrupt_handler;

    struct sigaction output{};

    HANDLE_INT_RESULT(sigaction(SIGINT, &input, &output));
}

void start_listener()
{
    interrupt_fd = eventfd(0, EFD_NONBLOCK | EFD_SEMAPHORE);
    HANDLE_INT_RESULT(interrupt_fd);

    listener_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    HANDLE_INT_RESULT(listener_socket);

    int option = 0;
    HANDLE_INT_RESULT(setsockopt(listener_socket, IPPROTO_IPV6, IPV6_V6ONLY, &option, sizeof(option)));

    struct sockaddr_in6 address{};
    address.sin6_family = AF_INET6;
    address.sin6_port = htons(RTSP_PORT);
    address.sin6_addr = in6addr_any;

    HANDLE_INT_RESULT(bind(listener_socket, reinterpret_cast<sockaddr *>(&address), sizeof(address)));

    HANDLE_INT_RESULT(listen(listener_socket, SOMAXCONN));
}

void do_work()
{
    Defer _d1([&]() {
        std::osyncstream(std::cout) << "Closing thread [" << std::this_thread::get_id() << "] gracefully." << std::endl;
    });

    int epoll_fd = epoll_create1(0);
    HANDLE_INT_RESULT(epoll_fd);

    epoll_event event_config{}, incoming{};

    event_config.events = EPOLLIN;
    event_config.data.fd = interrupt_fd;
    HANDLE_INT_RESULT(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, interrupt_fd, &event_config));

    event_config.events = EPOLLIN | EPOLLEXCLUSIVE;
    event_config.data.fd = listener_socket;
    HANDLE_INT_RESULT(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listener_socket, &event_config));

    struct sockaddr connection_addr;
    socklen_t conn_addr_size = sizeof(connection_addr);

    while (1)
    {
        // wait on exactly 1 event
        int n = epoll_wait(epoll_fd, &incoming, 1, -1);
        if (n == -1)
        {
            if (errno == EINTR)
            {
                size_t interrupt = 1;
                HANDLE_INT_RESULT(write(interrupt_fd, &interrupt, sizeof(interrupt)));

                return;
            }

            HANDLE_INT_RESULT(n);
        }

        if (incoming.data.fd == interrupt_fd)
        {
            return;
        }
        else if (incoming.data.fd == listener_socket)
        {
            connection_addr = {};

            int connection = accept(listener_socket, &connection_addr, &conn_addr_size);
            HANDLE_INT_RESULT(connection);

            epoll_event event_config{};
            event_config.events = EPOLLIN | EPOLLONESHOT;
            event_config.data.fd = connection;
            HANDLE_INT_RESULT(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connection, &event_config));
        }
        else
        {
            char b[4096] = {};

            n = recv(incoming.data.fd, b, sizeof(b), MSG_PEEK);
            HANDLE_INT_RESULT(n); // TODO need an error handler that doesn't exit

            int expected_length =
                -1; // try to find delimiter in message -- should also set the expected length of the body here somehow
                    // maybe have a map with the connection FDs where I'm expecting a body

            if (n == expected_length)
            {
                // consume msg
                n = recv(incoming.data.fd, b, expected_length, 0);
                HANDLE_INT_RESULT(n);
            }
            else
            {
                epoll_event event_config{};
                event_config.events =
                    EPOLLIN | EPOLLONESHOT | EPOLLET; // edge-triggered so we don't immediately peek this again
                event_config.data.fd = incoming.data.fd;
                HANDLE_INT_RESULT(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, incoming.data.fd, &event_config));
            }
        }
    }
}

int main()
{
    set_up_signal_handlers();

    start_listener();

    size_t worker_count = std::thread::hardware_concurrency() - 1;

    std::vector<std::jthread> thread_pool;
    thread_pool.reserve(worker_count);

    for (size_t i = 0; i < worker_count; ++i)
    {
        thread_pool.emplace_back(do_work);
    }

    do_work();

    rtsp_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    HANDLE_INT_RESULT(rtsp_socket);

    rtp_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    HANDLE_INT_RESULT(rtp_socket);

    rtcp_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    HANDLE_INT_RESULT(rtcp_socket);
}
