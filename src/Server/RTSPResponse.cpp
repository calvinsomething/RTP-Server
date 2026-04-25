#include "RTSPResponse.h"
#include "Exception.h"

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

void RTSPResponse::marshal()
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

    buffer.append(body);
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

std::string_view RTSPResponse::get_status_string(StatusCode sc)
{
    switch (sc)
    {
    case StatusCode::OK:
        return "200 OK";
    default:
        throw Exception("Invalid status code.");
    }
}

void RTSPResponse::set_status(StatusCode sc)
{
    status = get_status_string(sc);
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
