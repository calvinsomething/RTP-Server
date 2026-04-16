#include "RTSPRequest.h"

#include <charconv>
#include <cstddef>
#include <vector>

#include "Exception.h"

// Method         =         "DESCRIBE"              ; Section 10.2
//                 |         "ANNOUNCE"              ; Section 10.3
//                 |         "GET_PARAMETER"         ; Section 10.8
//                 |         "OPTIONS"               ; Section 10.1
//                 |         "PAUSE"                 ; Section 10.6
//                 |         "PLAY"                  ; Section 10.5
//                 |         "RECORD"                ; Section 10.11
//                 |         "REDIRECT"              ; Section 10.10
//                 |         "SETUP"                 ; Section 10.4
//                 |         "SET_PARAMETER"         ; Section 10.9
//                 |         "TEARDOWN"              ; Section 10.7
//
//                 request-header  =          Accept                   ; Section 12.1
//                  |          Accept-Encoding          ; Section 12.2
//                  |          Accept-Language          ; Section 12.3
//                  |          Authorization            ; Section 12.5
//                  |          From                     ; Section 12.20
//                  |          If-Modified-Since        ; Section 12.23
//                  |          Range                    ; Section 12.29
//                  |          Referer                  ; Section 12.30
//                  |          User-Agent               ; Section 12.41

#define _TO_STR(val) #val
#define TO_STR(val) _TO_STR(val)

#define THROW_IF_FALSE(condition, msg)                                                                                 \
    {                                                                                                                  \
        if (!condition)                                                                                                \
        {                                                                                                              \
            throw Exception(__FILE__ ":" TO_STR(__LINE__) ": ", msg);                                                  \
        }                                                                                                              \
    }

/*
From RFC: "Lines are terminated by CRLF, but receivers should be prepared
to also interpret CR and LF by themselves as line terminators."
*/
const std::string RTSPRequest::delimiters[] = {"\r\n", "\r", "\n"};

const std::set<std::string> RTSPRequest::unique_headers = {"cseq", "content-length", "session", "timestamp",
                                                           "content-type"};

void RTSPRequest::parse_request(const std::string &message)
{
    if (ready())
    {
        return;
    }

    std::vector<std::string_view> parts;
    parts.reserve(10);

    size_t header_end = 0;

    while (1)
    {
        size_t delimiter_size = 0;
        size_t part_end = message.size();

        for (std::string delimiter : delimiters)
        {
            size_t pos = message.find(delimiter, cursor);
            if (pos != message.npos && pos < part_end)
            {
                part_end = pos;
                delimiter_size = delimiter.size();
            }
        }

        if (!delimiter_size)
        {
            break;
        }

        if (part_end == cursor)
        {
            // 2 delimiters in a row
            header_end = part_end + delimiter_size;
            break;
        }
        else
        {
            parts.emplace_back(message.begin() + cursor, message.begin() + part_end);
        }

        cursor = part_end + delimiter_size;
    }

    for (size_t i = 0; i < parts.size(); ++i)
    {
        if (i)
        {
            add_header(parts[i]);
        }
        else
        {
            set_request_line_parts(parts[i]);
        }
    }

    if (header_end)
    {
        auto body_length = headers.find("content-length");
        if (body_length != headers.end())
        {
            size_t length;

            auto err = std::from_chars(body_length->second.begin().base(), body_length->second.end().base(), length);
            THROW_IF_FALSE(!err.ptr, err.ptr);

            if (header_end + length > message.size())
            {
                cursor = header_end;
                return;
            }

            auto body_start = message.begin() + header_end;
            body.assign(body_start, body_start + length);
        }

        cursor = PARSE_COMPLETE;
    }
}

RTSPRequest::RTSPRequest(const std::string &message)
{
    parse_request(message);
}

bool RTSPRequest::ready() const
{
    return cursor == PARSE_COMPLETE;
}

void RTSPRequest::add_header(std::string_view line)
{
    size_t i = line.find(": ");
    THROW_IF_FALSE((i != std::string::npos), line);

    auto key_end = line.begin() + i, value_start = key_end + 2;

    std::string key(i, 0);
    std::transform(line.begin(), key_end, key.begin(), tolower);

    auto h = headers.find(key);

    if (h == headers.end())
    {
        headers.emplace(std::piecewise_construct, std::forward_as_tuple(std::move(key)),
                        std::forward_as_tuple(value_start, line.end()));
    }
    else if (!unique_headers.contains(key))
    {
        h->second.reserve(h->second.size() + line.size() - i);
        h->second.append(", ");
        h->second.append(value_start, line.end());
    }
}

// Request-Line = Method SP Request-URI SP RTSP-Version CRLF
// (Request-URI = "*" | absolute_URI)
void RTSPRequest::set_request_line_parts(std::string_view line)
{
    std::string *parts[3] = {&method, &uri, &version};

    size_t i = 0, j = 0;
    for (auto p : parts)
    {
        if (p == &version)
        {
            j = line.size();
        }
        else
        {
            j = line.find(' ', i);
            THROW_IF_FALSE((j != std::string::npos), line);
        }

        p->assign(line.begin() + i, line.begin() + j);

        i = j + 1;
    }
}

std::string RTSPRequest::get_method() const
{
    return method;
}

std::string RTSPRequest::get_uri() const
{
    return uri;
}

std::string RTSPRequest::get_version() const
{
    return version;
}

const std::string &RTSPRequest::get_body() const
{
    return body;
}

std::string RTSPRequest::get_header(std::string key) const
{
    auto h = headers.find(key);
    if (h != headers.end())
    {
        return h->second;
    }

    return "";
}
