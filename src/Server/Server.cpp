#include "Server.h"

#include <asm-generic/socket.h>
#include <cerrno>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
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

#include "Exception.h"
#include "RTSPResponse.h"
#include "Server/Session.h"
#include "dispatch.h"
#include "util/Defer.h"

// Public
void Server::listen()
{
    interrupt_fd = eventfd(0, EFD_NONBLOCK | EFD_SEMAPHORE);
    HANDLE_INT_RESULT(interrupt_fd);

    listener_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    HANDLE_INT_RESULT(listener_socket);

    const int enabled = 1;
    HANDLE_INT_RESULT(setsockopt(listener_socket, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled)));

    int option = 0;
    HANDLE_INT_RESULT(setsockopt(listener_socket, IPPROTO_IPV6, IPV6_V6ONLY, &option, sizeof(option)));

    struct sockaddr_in6 address{};
    address.sin6_family = AF_INET6;
    address.sin6_port = htons(RTSP_PORT);
    address.sin6_addr = in6addr_any;

    HANDLE_INT_RESULT(bind(listener_socket, reinterpret_cast<sockaddr *>(&address), sizeof(address)));

    HANDLE_INT_RESULT(::listen(listener_socket, SOMAXCONN));

    epoll_fd = epoll_create1(0);
    HANDLE_INT_RESULT(epoll_fd);

    epoll_event event_config{};

    event_config.events = EPOLLIN;
    event_config.data.fd = interrupt_fd;
    HANDLE_INT_RESULT(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, interrupt_fd, &event_config));

    event_config.events = EPOLLIN | EPOLLET;
    event_config.data.fd = listener_socket;
    HANDLE_INT_RESULT(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listener_socket, &event_config));
}

void Server::serve()
{
    epoll_event incoming{};

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
                    Session::shutdown();
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
                if (connection == -1)
                {
                    int eno = errno;
                    if (eno == EAGAIN)
                    {
                        continue;
                    }
                    else
                    {
                        std::osyncstream(std::cout)
                            << "Thread [" << std::this_thread::get_id() << "] exiting due to errno " << eno << "\n";
                        return;
                    }
                }

                epoll_event event_config{};
                event_config.events = EPOLLIN | EPOLLONESHOT;
                event_config.data.fd = connection;
                HANDLE_INT_RESULT(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connection, &event_config));

                connections.write([=](decltype(connections)::type &connections) {
                    connections.insert_or_assign(connection, Connection(connection, connection_addr));
                });
            }
            else
            {
                std::unordered_map<int, Connection>::iterator c;
                bool found;

                connections.read([&](decltype(connections)::type &connections) {
                    c = connections.find(incoming.data.fd);
                    found = c != connections.end();
                });

                if (!found)
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

                c->second.lock();
                Defer _d1([&]() { c->second.unlock(); });

                char b[4096] = {};

                int n = recv(incoming.data.fd, b, sizeof(b), 0);
                HANDLE_INT_RESULT(n); // TODO throw an exception instead of exiting

                if (!n)
                {
                    // connection was shutdown/closed
                    HANDLE_INT_RESULT(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, incoming.data.fd, nullptr));

                    // TODO should not assume client closed connection -- set Connection::expires to now, then move this
                    // connection removal to expired connection cleanup
                    connections.write(
                        [&](decltype(connections)::type &connections) { connections.erase(incoming.data.fd); });

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

                // re-arm FD
                epoll_event event_config{};
                event_config.events = EPOLLIN | EPOLLONESHOT;
                event_config.data.fd = incoming.data.fd;
                HANDLE_INT_RESULT(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, event_config.data.fd, &event_config));
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

void Server::interrupt()
{
    size_t interrupt = 1;
    HANDLE_INT_RESULT(write(interrupt_fd, &interrupt, sizeof(interrupt)));
}

// Private
RTSPResponse Server::dispatch(const RTSPRequest &request)
{
    std::string method = request.get_method();

    auto h = Dispatch::rtsp.find(method);

    if (h == Dispatch::rtsp.end())
    {
        std::cout << "method = " << method << "\n";
        throw Exception(
            "Invalid method."); // TODO thrown Exceptions should be able to be used to write back error information
    }

    return h->second(request);
}
