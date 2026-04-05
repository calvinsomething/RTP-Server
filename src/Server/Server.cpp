#include "Server.h"

#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstring>
#include <iostream>
#include <syncstream>
#include <thread>
#include <utility>
#include <vector>

#include "../Defer.h"

// Public
void Server::listen_and_serve()
{
    start_listener();

    size_t worker_count = std::thread::hardware_concurrency() - 1;

    std::vector<std::jthread> thread_pool;
    thread_pool.reserve(worker_count);

    for (size_t i = 0; i < worker_count; ++i)
    {
        thread_pool.emplace_back([this]() { serve(); });
    }
    serve();
}

// Private
void Server::start_listener()
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

void Server::serve()
{
    Defer _d1([&]() {
        std::osyncstream(std::cout) << "Closing thread " << std::this_thread::get_id() << " gracefully." << std::endl;
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
            sockaddr_in6 connection_addr{};
            socklen_t conn_addr_size = sizeof(connection_addr);

            int connection = accept(listener_socket, reinterpret_cast<sockaddr *>(&connection_addr), &conn_addr_size);
            HANDLE_INT_RESULT(connection);

            epoll_event event_config{};
            event_config.events = EPOLLIN | EPOLLONESHOT;
            event_config.data.fd = connection;
            HANDLE_INT_RESULT(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connection, &event_config));

            connections.insert_or_assign(connection, Connection(connection, connection_addr));
        }
        else
        {
            auto c = connections.find(incoming.data.fd);
            if (c == connections.end())
            {
                std::cout << "Connection not in map (fd = " << incoming.data.fd << ")\n";
                return;
            }

            char b[4096] = {};

            int n = recv(incoming.data.fd, b, sizeof(b), 0);
            HANDLE_INT_RESULT(n); // TODO need an error handler that doesn't exit

            c->second.append_to_message(b, n);

            auto header = c->second.get_rtsp_header();
            if (header.empty())
            {
                // re-arm FD
                epoll_event event_config{};
                event_config.events = EPOLLIN | EPOLLONESHOT;
                event_config.data.fd = incoming.data.fd;
                HANDLE_INT_RESULT(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event_config.data.fd, &event_config));
            }
            else
            {
                HANDLE_INT_RESULT(send(incoming.data.fd, "OK", 2, 0));

                c->second.clear_message();
            }
        }
    }
}

int Server::get_expected_message_length(int connection, char *b, size_t n)
{
    // TODO keep all throwing types/functions in another module with try/catch blocks
    static std::unordered_map<int, size_t> body_length_by_connection;

    auto body_length = body_length_by_connection.find(connection);
    if (body_length != body_length_by_connection.end())
    {
        return body_length->second;
    }

    int expected_length = -1;

    // static std::regex re("\r\n\r\n|\r\r|\n\n", std::regex_constants::basic);

    // std::match_results<char *> m;

    // if (std::regex_search(b, b + n, m, re))
    // {
    // 	return m[0].first - b;
    // }

    std::string_view sv(b, n);

    for (std::string delimiter : {"\r\n\r\n", "\r\r", "\n\n"})
    {
        size_t pos = sv.find(delimiter);
        if (pos != sv.npos)
        {
            return pos + delimiter.size();
        }
    }

    return expected_length;
}
