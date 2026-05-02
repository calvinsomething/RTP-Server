#include "Server.h"

#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <charconv>
#include <cstring>
#include <iostream>
#include <syncstream>
#include <thread>
#include <utility>
#include <vector>

#include "../util/Defer.h"
#include "Exception.h"
#include "RTSPResponse.h"
#include "dispatch.h"

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
        try
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

                int connection =
                    accept(listener_socket, reinterpret_cast<sockaddr *>(&connection_addr), &conn_addr_size);
                HANDLE_INT_RESULT(connection);

                epoll_event event_config{};
                event_config.events = EPOLLIN | EPOLLONESHOT;
                event_config.data.fd = connection;
                HANDLE_INT_RESULT(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connection, &event_config));

                connections.insert_or_assign(
                    connection,
                    Connection(connection, connection_addr)); // TODO should be an atomic/locking map, also have to deal
                                                              // with destroying connections when the FD closes
            }
            else
            {
                auto c = connections.find(incoming.data.fd);
                if (c == connections.end())
                {
                    char message[128] = {"Connection not in map (fd = "};
                    size_t n = std::strlen(message);

                    auto res = std::to_chars(message + n, message + n + 10, incoming.data.fd);
                    if (res.ptr)
                    {
                        throw Exception(res.ptr);
                    }
                    else
                    {
                        message[n + std::strlen(message + n)] = ')';
                        throw Exception(message);
                    }
                }

                char b[4096] = {};

                int n = recv(incoming.data.fd, b, sizeof(b), 0);
                HANDLE_INT_RESULT(n); // TODO use an exception instead of this handler that exits

                if (!n)
                {
                    // connection was closed
                    HANDLE_INT_RESULT(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, incoming.data.fd, nullptr));

                    connections.erase(incoming.data.fd);

                    continue;
                }

                c->second.append_to_message(b, n);

                auto &request = c->second.get_request();

                if (request.ready())
                {
                    RTSPResponse response = dispatch(request);

                    HANDLE_INT_RESULT(send(incoming.data.fd, response.get_data(), response.get_length(), 0));

                    c->second.clear_message();
                }
                else
                {
                    // re-arm FD
                    epoll_event event_config{};
                    event_config.events = EPOLLIN | EPOLLONESHOT;
                    event_config.data.fd = incoming.data.fd;
                    HANDLE_INT_RESULT(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event_config.data.fd, &event_config));
                }
            }
        }
        catch (std::exception &e)
        {
            std::osyncstream(std::cout) << e.what() << std::endl;
        }
        catch (...)
        {
            std::osyncstream(std::cout) << "Unknown exception.\n";
        }
    }
}

RTSPResponse Server::dispatch(const RTSPRequest &request)
{
    std::string method = request.get_method();

    auto h = Dispatch::rtsp.find(method);

    if (h == Dispatch::rtsp.end())
    {
        throw Exception(
            "Invalid method."); // TODO thrown Exceptions should be able to be used to write back error information
    }

    return h->second(request);
}
