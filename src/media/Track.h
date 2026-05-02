#pragma once

#include "../Server/RTPTransport.h"

class Track
{
  public:
    Track(std::string_view uri, std::string_view transport_header_value);

    std::pair<unsigned, unsigned> get_ports();

    RTPTransport transport;

  private:
    unsigned rtpPort, rtcpPort;
};
