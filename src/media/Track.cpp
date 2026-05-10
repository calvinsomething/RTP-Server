#include "Track.h"

#include "fcntl.h"

#include "../Server/Exception.h"

std::unordered_map<std::string_view, std::string_view> Track::file_name_by_id{{"0", MEDIA_DIRECTORY "/video.mp4"}};
RWLocker<std::unordered_map<std::string_view, int>> Track::open_fd_by_id;

Track::Track(std::string_view uri, std::string_view transport_header_value) : transport(transport_header_value)
{
    std::string_view id = get_id_from_uri(uri);
    bool found_open = false;

    std::unordered_map<std::string_view, int>::iterator open_fd;
    open_fd_by_id.read([&](decltype(open_fd_by_id)::type &map) {
        open_fd = map.find(id);
        found_open = open_fd != map.end();
    });

    if (found_open)
    {
        fd = open_fd->second;
    }
    else
    {
        auto file = file_name_by_id.find(id);
        if (file == file_name_by_id.end())
        {
            throw Exception("Invalid track ID: ", id);
        }

        fd = open(std::string(file->second).c_str(), O_RDONLY);
        if (fd < 0)
        {
            throw Exception("Track::load_file failed: ", strerror(errno));
        }

        open_fd_by_id.write([&](decltype(open_fd_by_id)::type &map) { map.insert({id, fd}); });
    }
}

std::string_view Track::get_id_from_uri(std::string_view uri)
{
    std::string_view key_str("trackID=");

    size_t i = uri.find(key_str);
    if (i == std::string::npos || i + key_str.size() == uri.size())
    {
        throw Exception("Invalid URI: ", uri);
    }

    return std::string_view(uri.begin() + i + key_str.size(), uri.end());
}
