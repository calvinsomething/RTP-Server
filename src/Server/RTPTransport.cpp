#include "RTPTransport.h"

#include <algorithm>
#include <cctype>
#include <set>

#include "../util/misc.h"
#include "Exception.h"
#include "UDP.h"

// RFC:
// Transports are comma separated, listed in order of preference.
// Parameters may be added to each transport, separated by a semicolon.
// The syntax for the transport specifier is:
//		transport/profile/lower-transport

RTPTransport::RTPTransport(std::string_view request_header_value)
{
    size_t i = request_header_value.find(';');
    if (i == std::string::npos)
    {
        throw Exception("Invalid transport spec.");
    }

    std::string lower_case_header_value(request_header_value.size(), 0);
    std::transform(request_header_value.begin(), request_header_value.end(), lower_case_header_value.data(), tolower);

    std::string *parts_dst[] = {&protocol, &profile, &lower_transport};
    std::string_view specifier{lower_case_header_value.begin(), lower_case_header_value.begin() + i};

    auto parts_src = split(specifier, '/');
    for (size_t i = 0; i < parts_src.size(); ++i)
    {
        parts_dst[i]->assign(parts_src[i]);
    }

    if (protocol != "rtp")
    {
        throw Exception("Invalid transport protocol.");
    }

    if (profile != "avp")
    {
        throw Exception("Invalid transport profile.");
    }

    if (!lower_transport.empty() && lower_transport != "udp")
    {
        throw Exception("Invalid lower-transport.");
    }

    auto parameters = split({lower_case_header_value.begin() + i + 1, lower_case_header_value.end()}, ';');
    for (auto parameter : parameters)
    {
        set_parameter(parameter);
    }

    if (!unicast)
    {
        throw Exception("Missing transport delivery option: unicast");
    }

    bind();
}

void RTPTransport::bind()
{
    std::set<uint16_t> attempted_ports;
    while (!rtp_socket.bind() || (client_ports.second && !rtcp_socket.bind(rtp_socket.get_port() + 1)))
    {
        uint16_t rtp_port = rtp_socket.get_port();

        if (attempted_ports.contains(rtp_port))
        {
            throw Exception("Failed to bind UDP sockets.");
        }

        attempted_ports.insert(rtp_port);
    }

    server_ports.first = rtp_socket.get_port();
    server_ports.second = rtcp_socket.get_port();
}

void RTPTransport::set_parameter(std::string_view parameter)
{
    size_t i = parameter.find('=');

    if (i != std::string::npos)
    {
        std::string_view key(parameter.begin(), parameter.begin() + i),
            value(parameter.begin() + i + 1, parameter.end());

        if (key == "client_port")
        {
            set_client_ports(value);
        }
        else if (key == "mode" && value != "play")
        {
            throw Exception("Unsupported transport mode: ", value);
        }
    }
    else if (parameter == "unicast")
    {
        unicast = true;
    }
}

void RTPTransport::set_client_ports(std::string_view value)
{
    size_t i = value.find('-');

    if (i == std::string::npos)
    {
        i = value.size();
    }

    auto result = std::from_chars(value.begin(), value.begin() + i, client_ports.first);
    if (result.ec != std::errc{})
    {
        throw Exception("Invalid RTP client_port: ", result.ptr);
    }
    if (i != value.size())
    {
        auto result = std::from_chars(value.begin() + i + 1, value.end(), client_ports.second);
        if (result.ec != std::errc{} || client_ports.second != client_ports.first + 1)
        {
            throw Exception("Invalid RTCP client_port: ", result.ptr);
        }
    }

    if (client_ports.first % 2)
    {
        --client_ports.first;
        if (client_ports.second)
        {
            --client_ports.second;
        }
    }
}

std::string RTPTransport::get_string()
{
    std::string s(128, 0);

    std::string::iterator cursor = s.begin() + std::sprintf(s.data(), "%s/%s", protocol.c_str(), profile.c_str());

    if (!lower_transport.empty())
    {
        cursor += std::sprintf(cursor.base(), "/%s", lower_transport.c_str());
    }

    std::transform(s.begin(), cursor, s.begin(), toupper);

    cursor += std::sprintf(cursor.base(), ";unicast;client_port=%d", client_ports.first);
    if (client_ports.second)
    {
        cursor += std::sprintf(cursor.base(), "-%d", client_ports.second);
    }

    cursor += std::sprintf(cursor.base(), ";server_port=%d", server_ports.first);

    if (server_ports.second)
    {
        cursor += std::sprintf(cursor.base(), "-%d", server_ports.second);
    }

    s.resize(cursor - s.begin());

    return s;
}
