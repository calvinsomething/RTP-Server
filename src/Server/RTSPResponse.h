#pragma once

#include <string>

class RTSPResponse
{
  public:
    RTSPResponse() = default;
    RTSPResponse(const std::string &method);

    const char *get_data() const;
    size_t get_length() const;

  private:
    std::string buffer;
};
