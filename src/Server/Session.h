#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

#include "../media/Track.h"
#include "../util/RNG.h"

class Session
{
  private:
    struct PrivateKey
    {
    };

  public:
    static Session &get();
    static Session &get(const std::string &id);

    Session(PrivateKey key, const std::string &id);
    void end();

    std::string get_id();

    std::vector<Track> tracks;

  private:
    static std::unordered_map<std::string, Session> sessions;
    static std::string generate_id();
    static RNG<uint64_t> rng;

    std::string id;
    std::chrono::time_point<std::chrono::steady_clock> expires_at;
};
