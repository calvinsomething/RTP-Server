#pragma once

#include <netinet/in.h>

#include <cstddef>
#include <cstdint>

namespace UDP
{
uint16_t get_even_port();
void insert_even_port(uint16_t port);

class Socket
{
  public:
    Socket();
    ~Socket();

    bool bind();
    bool bind(uint16_t port);

    void connect(in6_addr client_addr);
    void send(uint8_t *data, size_t size);

    uint16_t get_port();

  private:
    int socket_fd;
    uint16_t port;

    void reset_port();
};
}; // namespace UDP
