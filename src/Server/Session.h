#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "Track.h"
#include "util/Locker.h"
#include "util/RNG.h"

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

    void play();
    void play(float npt_begin, float npt_end);
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

    std::chrono::time_point<std::chrono::steady_clock> expires_at;

    std::pair<float, float> play_range = {};

    std::atomic<bool> is_active = 1;
    bool is_playing = 0;

    // methods
    void tick();
};
