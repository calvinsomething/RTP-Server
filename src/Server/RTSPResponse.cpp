#include "RTSPResponse.h"

RTSPResponse::RTSPResponse(const std::string &method)
{
    buffer = method;
}

const char *RTSPResponse::get_data() const
{
    return buffer.data();
}

size_t RTSPResponse::get_length() const
{
    return buffer.size();
}
