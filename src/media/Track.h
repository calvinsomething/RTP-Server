#pragma once

#include <unordered_map>

#include "../Server/RTPTransport.h"
#include "../util/Locker.h"

class Track
{
  public:
    Track(std::string_view uri, std::string_view transport_header_value);

    RTPTransport transport;

  private:
    static std::unordered_map<std::string_view, std::string_view> file_name_by_id;
    static RWLocker<std::unordered_map<std::string_view, int>> open_fd_by_id;

    std::string_view get_id_from_uri(std::string_view uri);

    int fd = 0;
};
