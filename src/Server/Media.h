#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

#include "Server/Stream.h"

struct Media
{
    static std::unordered_map<std::string_view, Media *> by_basename;

    std::string basename, file_name, sdp;

    std::unordered_map<std::string_view, Stream::MediaType> stream_type_by_control_id;
};
