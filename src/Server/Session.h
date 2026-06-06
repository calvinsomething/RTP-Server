#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

#include "../util/RNG.h"
#include "Track.h"

class Session
{
  private:
    struct PrivateKey
    {
    };

  public:
    static Session &get();
    static Session &get(const std::string &id);

    static void tick();

    Session(PrivateKey key, const std::string &id);
    void terminate();

    std::string get_id();

    Track &emplace_track(in6_addr client_address, const std::string &uri, std::string_view transport_value);

  private:
    static std::unordered_map<std::string, Session> sessions;
    static std::string generate_id();
    static RNG<uint64_t> rng;

    std::vector<Track> tracks;

    std::string id;
    std::chrono::time_point<std::chrono::steady_clock> expires_at;

    bool is_playing = 0;
};
