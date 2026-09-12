#include "Server.h"

#include <iostream>
#include <netinet/tcp.h>
#include <unistd.h>

#include <chrono>
#include <cstring>

constexpr auto MIN_CONNECTION_LIFETIME = std::chrono::seconds(5);

Server::Connection::Connection(int fd, sockaddr_in6 socket_address) : fd(fd), socket_address(socket_address)
{
    int enabled = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &enabled, sizeof(enabled)) < 0)
    {
        // TODO throw exception
        std::cout << __LINE__ << " setsockopt failed\n";
    }

    // TCP_KEEPCNT (since Linux 2.4)
    //        The maximum number of keepalive probes TCP should send before dropping the connection.  This option should
    //        not be used in code intended to be portable.

    // TCP_KEEPIDLE (since Linux 2.4)
    //        The  time  (in  seconds)  the connection needs to remain idle before TCP starts sending keepalive probes,
    //        if the socket option SO_KEEPALIVE has been set on this socket.  This option should not be used in code
    //        intended to be portable.

    // TCP_KEEPINTVL (since Linux 2.4)
    //        The time (in seconds) between individual keepalive probes.  This option should not be used in code
    //        intended to be portable.

    int keep_alive_sec = 30;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &keep_alive_sec, sizeof(keep_alive_sec)) < 0)
    {
        // TODO throw exception
        std::cout << __LINE__ << " setsockopt failed\n";
    }

    // TODO update expires every time something happens
    expires = std::chrono::steady_clock::now() + MIN_CONNECTION_LIFETIME;
    request.client_addr = socket_address.sin6_addr;
}

Server::Connection::Connection(Connection &&other)
{
    fd = other.fd;
    message_buffer = std::move(other.message_buffer);

    other.fd = 0;
}

Server::Connection &Server::Connection::operator=(Connection &&other)
{
    std::swap(*this, other);
    return *this;
}

Server::Connection::~Connection()
{
    if (fd)
    {
        HANDLE_INT_RESULT(close(fd));
    }
}

int Server::Connection::get_fd() const
{
    return fd;
}

const RTSPRequest &Server::Connection::get_request() const
{
    return request;
}

void Server::Connection::clear_message()
{
    message_buffer.clear();
    request = {};
    request.client_addr = socket_address.sin6_addr;
}

void Server::Connection::append_to_message(const char *b, size_t n)
{
    message_buffer.append(b, n);

    request.parse_request(message_buffer);
}

void Server::Connection::lock()
{
    mutex.lock();
}

void Server::Connection::unlock()
{
    mutex.unlock();
}
