#pragma once

#include <string>
#include <unordered_map>

class RTSPResponse
{
  public:
    enum class StatusCode // must add status string to get_status_string
    {
        OK = 200,

        NotFound = 404,
        NotAcceptable = 406,
        AggregateOperationNotAllowed = 459,
        OnlyAggregateOperationAllowed = 460,
    };

    std::string body;

    RTSPResponse() = default;
    RTSPResponse(const std::string &method);

    const char *get_data() const;
    size_t get_length() const;

    void set_header(const std::string &key, const std::string &value);
    void append_header(const std::string &key, const std::string &value);

    void set_status(StatusCode sc);

    void marshal();

  private:
    std::string buffer;
    std::string_view status;

    std::string_view get_status_string(StatusCode sc);
    std::unordered_map<std::string, std::string> headers;
};
