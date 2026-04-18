#pragma once

#include <chrono>
#include <string>

class RTSPSession
{
  public:
    RTSPSession();

  private:
    static std::string generate_id();

    std::string id;
    std::chrono::time_point<std::chrono::steady_clock> expires_at;
};
