#pragma once

#include <netinet/in.h>

#include <string>

#include "UDP.h"

class RTPTransport
{
  public:
    RTPTransport(in6_addr client_addr, std::string_view request_header_value);

    std::string get_string();

    void send(uint8_t *data, size_t size);

  private:
    bool unicast = false;
    std::pair<uint16_t, uint16_t> client_ports, server_ports;

    uint32_t SSRC;

    std::string protocol, profile, lower_transport;

    void set_parameter(std::string_view parameter);
    void set_client_ports(std::string_view value);

    void bind();

    UDP::Socket rtp_socket, rtcp_socket;
};
