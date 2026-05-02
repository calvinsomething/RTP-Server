#include "Track.h"

Track::Track(std::string_view uri, std::string_view transport_header_value) : transport(transport_header_value)
{
    // TODO dynamically set ports
    rtpPort = 5004;
    rtcpPort = 5005;
}

std::pair<unsigned, unsigned> Track::get_ports()
{
    return {rtpPort, rtcpPort};
}
