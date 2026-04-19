#pragma once

#include <string>
#include <unordered_map>

class RTSPResponse
{
  public:
    enum class StatusCode
    {
        OK = 200,
    };

    RTSPResponse() = default;
    RTSPResponse(const std::string &method);

    const char *get_data() const;
    size_t get_length() const;

    void set_header(const std::string &key, const std::string &value);
    void append_header(const std::string &key, const std::string &value);

    void set_status(StatusCode sc);

    void marshal_data();

  private:
    std::string buffer;
    std::string_view status;

    std::string_view get_status_string(StatusCode sc);
    std::unordered_map<std::string, std::string> headers;
};
