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

// RFC: The time parameter may be used to aid in synchronization
// of streams obtained from different sources.

class Session
{
  private:
    struct PrivateKey
    {
    };

    class Group
    {
      public:
        Group() = default;
        Group(const Group &other);
        Group(Group &&other);

        void add_session(std::shared_ptr<Session> session);
        void watch_streams();

        std::vector<std::shared_ptr<Session>> sessions;

      private:
        Locker<std::queue<std::shared_ptr<Session>>> newly_added;
    };

  public:
    static void init();
    static std::shared_ptr<Session> get();
    static std::shared_ptr<Session> get(const std::string &id);
    static void watch_streams(unsigned worker_count);
    static void shutdown();

    Session(PrivateKey key, const std::string &id);
    Session(Session &&other) noexcept;
    Session(const Session &other) = delete;
    void teardown();

    std::pair<float, float> play();
    std::pair<float, float> play(float npt_begin, float npt_end);

    void pause(float npt);

    std::string get_id();

    Track &emplace_track(in6_addr client_address, const std::string &uri, std::string_view transport_value);

  private:
    static std::atomic<bool> is_live;
    static Locker<std::unordered_map<std::string, std::shared_ptr<Session>>> sessions;
    static std::string generate_id();
    static RNG<uint64_t> rng;

    static Locker<std::vector<Session::Group>> groups;
    Group *group;

    std::string id;
    std::atomic<int> reference_count;

    std::vector<Track> tracks;

    std::queue<std::pair<float, float>> play_ranges_queue;

    // TODO Session: header's timeout= parameter (default 60s per RFC 2326 if the client doesn't negotiate otherwise)
    // should match Connection::expires
    std::chrono::time_point<std::chrono::steady_clock> expires_at;

    // play_range.second == 0 -> undefined end value
    std::pair<float, float> play_range = {};

    std::atomic<bool> is_active = 1, is_playing = 0;

    // methods
    void tick();
};
