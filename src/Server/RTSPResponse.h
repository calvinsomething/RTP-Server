#pragma once

#include <string>
#include <unordered_map>

namespace Status
{
constexpr std::string_view OK{"200 OK"};
};

class RTSPResponse
{
  public:
    RTSPResponse() = default;
    RTSPResponse(const std::string &method);

    const char *get_data() const;
    size_t get_length() const;

    void set_header(const std::string &key, const std::string &value);
    void append_header(const std::string &key, const std::string &value);

    void set_status(std::string_view s);

    void marshal_data();

  private:
    std::string buffer;
    std::string_view status;

    std::unordered_map<std::string, std::string> headers;
};
