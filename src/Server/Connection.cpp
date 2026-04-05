#include "Server.h"

#include <cstring>
#include <unistd.h>

#include <chrono>

const auto MIN_CONNECTION_LIFETIME = std::chrono::seconds(5);

Server::Connection::Connection(int fd, sockaddr_in6 socket_address) : fd(fd), socket_address(socket_address)
{
    expires = std::chrono::steady_clock::now() + MIN_CONNECTION_LIFETIME;
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

std::string_view Server::Connection::get_rtsp_header()
{
    for (std::string delimiter : {"\r\n\r\n", "\r\r", "\n\n"})
    {
        size_t pos = message_buffer.find(delimiter);
        if (pos != message_buffer.npos)
        {
            return std::string_view(message_buffer.begin(), message_buffer.begin() + pos + delimiter.size());
        }
    }

    return {};
}

void Server::Connection::clear_message()
{
    message_buffer.clear();
}

void Server::Connection::append_to_message(const char *b, size_t n)
{
    message_buffer.append(b, n);
}
