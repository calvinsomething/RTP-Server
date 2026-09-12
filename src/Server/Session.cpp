#include "Session.h"

#include <iostream>
#include <syncstream>
#include <thread>

#include "Exception.h"

// Static
//
std::atomic<bool> Session::is_live(true);

RNG<uint64_t> Session::rng;

Locker<std::vector<Session::Group>> Session::groups;
Locker<std::unordered_map<std::string, std::shared_ptr<Session>>> Session::sessions;

void Session::shutdown()
{
    is_live.store(false);
}

std::shared_ptr<Session> Session::get(const std::string &id)
{
    decltype(sessions)::type::iterator s;

    sessions.use([&](decltype(sessions)::type &sessions) {
        s = sessions.find(id);
        if (s == sessions.end() || !s->second->is_active)
        {
            throw Exception("Invalid session error."); // TODO create exception types that can be handled accordingly
        }
    });

    return s->second;
}

std::shared_ptr<Session> Session::get()
{
    Group *dest = 0;
    std::string id;

    decltype(sessions)::type::iterator s;

    std::shared_ptr<Session> session = 0;

    groups.use([&](decltype(groups)::type &groups) {
        for (auto &g : groups)
        {
            if (!dest || g.sessions.size() < dest->sessions.size())
            {
                dest = &g;
            }
        }
    });

    sessions.use([&](decltype(sessions)::type &sessions) {
        do
        {
            id = generate_id();
            s = sessions.find(id);
        } while (s != sessions.end());

        auto result = sessions.insert({id, std::make_shared<Session>(Session(PrivateKey{}, id))});
        session = result.first->second;
    });

    dest->add_session(session);

    return session;
}

void Session::Group::watch_streams()
{
    std::queue<std::vector<std::shared_ptr<Session>>::iterator> to_remove;

    while (is_live.load())
    {
        for (auto it = sessions.begin(); it != sessions.end(); ++it)
        {
            if ((*it)->is_active.load())
            {
                (*it)->tick();
            }
            else if ((*it).use_count() == 1) // use_count is not thread safe; however, if we do not make shared_ptr
                                             // copies after Session::is_active is false, this should be okay
            {
                to_remove.push(it);
            }
        }

        while (!to_remove.empty())
        {
            sessions.erase(to_remove.front());
            to_remove.pop();
        }

        newly_added.use([&](decltype(newly_added)::type &v) {
            while (!v.empty())
            {
                sessions.push_back(v.front());
                v.pop();
            }
        });
    }
}

Session::Group::Group(const Group &other)
{
    newly_added.use([&](decltype(newly_added)::type &v) {
        const_cast<Group &>(other).newly_added.use([&](decltype(other.newly_added)::type &other) { v = other; });
    });
}

Session::Group::Group(Group &&other) : newly_added(std::move(other.newly_added))
{
}

void Session::watch_streams(unsigned worker_count)
{
    static std::vector<std::jthread> thread_pool;
    thread_pool.reserve(worker_count);

    groups = decltype(groups)(worker_count, Group{});

    groups.use([&](decltype(groups)::type &v) {
        for (auto &g : v)
        {
            thread_pool.emplace_back([&]() { g.watch_streams(); });
        }
    });
}

std::string Session::generate_id()
{
    uint64_t bits = rng.get();

    unsigned char *start = reinterpret_cast<unsigned char *>(&bits);
    unsigned char *end = reinterpret_cast<unsigned char *>(&bits + 1);

    for (unsigned char *c = start; c != end; ++c)
    {
        *c = *c % 26 + 'A'; // restricted to A-Z
    }

    return std::string(reinterpret_cast<char *>(start), 8);
}

// Group
//
void Session::Group::add_session(std::shared_ptr<Session> session)
{
    newly_added.use([&](decltype(newly_added)::type &v) { v.push(session); });
}

// Non-Static
//
Session::Session(PrivateKey key, const std::string &id) : id(id), reference_count(0), is_active(1)
{
}

Session::Session(Session &&other) noexcept
    : id(std::move(other.id)), reference_count(other.reference_count.load()), is_active(other.is_active.load())
{
}

void Session::teardown()
{
    is_active.store(0);
}

std::string Session::get_id()
{
    return id;
}

Track &Session::emplace_track(in6_addr client_address, const std::string &uri, std::string_view transport_value)
{
    return tracks.emplace_back(client_address, uri, transport_value);
}

std::pair<float, float> Session::play()
{
    is_playing.store(true);

    return play_range;
}

std::pair<float, float> Session::play(float npt_begin, float npt_end)
{
    is_playing.store(true);

    for (auto &t : tracks)
    {
        play_range.first = t.set_play_time(npt_begin);

        if (npt_end)
        {
            play_range.second = t.set_play_range_end(npt_end);
        }
    }

    return play_range;
}

// The PAUSE request may contain a Range header specifying when the stream or
// presentation is to be halted. We refer to this point as the "pause point".
//
// The PAUSE request causes the stream delivery to be interrupted
// (halted) temporarily. If the request URL names a stream, only
// playback and recording of that stream is halted. For example, for
// audio, this is equivalent to muting.
//
// A PAUSE request discards all queued PLAY requests. However, the pause
// point in the media stream MUST be maintained. A subsequent PLAY
// request without Range header resumes from the pause point.
//
// If the pause NPT comes before the current NPT, play stops immediately.
void Session::pause(float npt)
{
    is_playing.store(false);

    for (auto &t : tracks)
    {
        t.set_play_time(npt);
    }
}

void Session::pause()
{
    is_playing.store(false);
}

void Session::tick()
{
    if (is_playing.load())
    {
        bool any_sent = false;
        for (auto &t : tracks)
        {
            any_sent = any_sent || t.send_frames();
        }

        if (!any_sent)
        {
            is_playing = false;
        }
    }
}
