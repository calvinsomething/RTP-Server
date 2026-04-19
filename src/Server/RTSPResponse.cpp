#include "RTSPResponse.h"

// Response    =     Status-Line         ; Section 7.1
//                  *(    general-header      ; Section 5
//                  |     response-header     ; Section 7.1.2
//                  |     entity-header )     ; Section 8.1
//                        CRLF
//                        [ message-body ]    ; Section 4.3

// TODO remove
RTSPResponse::RTSPResponse(const std::string &method)
{
    buffer = method;
}

void RTSPResponse::marshal_data()
{
    buffer.clear();

    buffer.reserve(80 + headers.size() * 80);

    buffer.append("RTSP/1.0 ");
    buffer.append(status);
    buffer.push_back('\n');

    for (auto &h : headers)
    {
        buffer.append(h.first + ": " + h.second);
        buffer.push_back('\n');
    }
    buffer.push_back('\n');
}

const char *RTSPResponse::get_data() const
{
    return buffer.data();
}

size_t RTSPResponse::get_length() const
{
    return buffer.size();
}

void RTSPResponse::set_header(const std::string &key, const std::string &value)
{
    headers.insert_or_assign(key, value);
}

void RTSPResponse::set_status(std::string_view s)
{
    status = s;
}

void RTSPResponse::append_header(const std::string &key, const std::string &value)
{
    auto h = headers.find(key);

    if (h == headers.end())
    {
        set_header(key, value);
    }
    else
    {
        std::string v = std::string(h->second + "," + value);
        set_header(key, v);
    }
}
