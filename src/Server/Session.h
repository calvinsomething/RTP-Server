#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "Track.h"
#include "util/Locker.h"
#include "util/RNG.h"

extern "C"
{
    void do_stream(int);
}

class Session
{
  private:
    struct PrivateKey
    {
    };

  public:
    static void init();
    static std::shared_ptr<Session> get();
    static std::shared_ptr<Session> get(const std::string &id);
    static void watch_streams();
    static void shutdown();

    Session(PrivateKey key, const std::string &id);
    Session(Session &&other) noexcept;
    Session(const Session &other) = delete;
    void teardown();

    void play();
    void play(float npt_begin, float npt_end);
    void pause(float npt);

    std::string get_id();

    Track &emplace_track(in6_addr client_address, const std::string &uri, std::string_view transport_value);

  private:
    static bool is_live;

    static Locker<std::unordered_map<std::string, std::shared_ptr<Session>>> sessions;
    static std::string generate_id();
    static RNG<uint64_t> rng;

    static Locker<std::vector<std::vector<std::shared_ptr<Session>> *>> groups;
    std::vector<Session *> *session_group;

    std::string id;
    std::atomic<int> reference_count;

    std::vector<Track> tracks;

    std::queue<std::pair<float, float>> play_ranges_queue;

    std::chrono::time_point<std::chrono::steady_clock> expires_at;

    std::pair<float, float> play_range = {};

    std::atomic<bool> is_active = 1;
    bool is_playing = 0;

    // methods
    void tick();
};
