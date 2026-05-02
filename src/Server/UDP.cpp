#include "UDP.h"

#include <cstdint>
#include <netinet/in.h>
#include <queue>
#include <sys/socket.h>
#include <unistd.h>

#include "../util/Locker.h"

static Locker<std::queue<uint16_t>> even_ports;
struct InitPorts
{
    InitPorts()
    {
        even_ports.use([](decltype(even_ports)::type &ep) {
            // 15 pairs of ports
            for (uint16_t port : {
                     5004,
                     5006,
                     5008,
                     5010,
                     5012,
                     5014,
                     5016,
                     5018,
                     5020,
                     5022,
                     5024,
                     5026,
                     5028,
                     5030,
                     5032,
                     5034,
                 })
            {
                ep.push(port);
            }
        });
    }
};
static InitPorts __init_ports;

namespace UDP
{
uint16_t get_even_port()
{
    uint16_t port = 0;

    even_ports.use([&port](decltype(even_ports)::type &ep) {
        if (!ep.empty())
        {
            port = ep.front();
            ep.pop();
        }
    });

    return port;
}

void insert_even_port(uint16_t port)
{
    even_ports.use([=](decltype(even_ports)::type &ep) { ep.push(port); });
}

// Socket
Socket::Socket()
{
}

Socket::~Socket()
{
    if (socket_fd)
    {
        close(socket_fd);
        reset_port();
    }
}

void Socket::reset_port()
{
    if (port && port % 2 == 0)
    {
        insert_even_port(port);
        port = 0;
    }
}

bool Socket::bind()
{
    return bind(get_even_port());
}

bool Socket::bind(uint16_t port)
{
    reset_port();

    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    // check err

    sockaddr_in address{};
    // TODO ...

    Socket::port = port;
    return true;
}

uint16_t Socket::get_port()
{
    return port;
}

}; // namespace UDP
