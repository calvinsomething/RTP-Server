#include "Server.h"

#include <unistd.h>

#include <chrono>
#include <cstring>

constexpr auto MIN_CONNECTION_LIFETIME = std::chrono::seconds(5);

Server::Connection::Connection(int fd, sockaddr_in6 socket_address) : fd(fd), socket_address(socket_address)
{
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
