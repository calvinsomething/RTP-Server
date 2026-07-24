#include "Session.h"

#include "Exception.h"

// Static
//
bool Session::is_live = true;

RNG<uint64_t> Session::rng;

// Must not lock Session::groups while Session::sessions is locked.
Locker<std::vector<std::vector<std::shared_ptr<Session>> *>> Session::groups;
Locker<std::unordered_map<std::string, std::shared_ptr<Session>>>
    Session::sessions; // TODO ! Session * can dangle if group re-allocates... use std::shared_ptr and call unique()
                       // before destroying...

void Session::shutdown()
{
    is_live = false;
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
    std::vector<std::shared_ptr<Session>> *dest = 0;
    std::string id;

    decltype(sessions)::type::iterator s;

    std::shared_ptr<Session> session = 0;

    groups.use([&](decltype(groups)::type &groups) {
        for (auto g : groups)
        {
            if (!dest || g->size() < dest->size())
            {
                dest = g;
            }
        }

        sessions.use([&](decltype(sessions)::type &sessions) {
            do
            {
                id = generate_id();
                s = sessions.find(id);
            } while (s != sessions.end());
        });

        dest->emplace_back(std::make_shared<Session>(Session(PrivateKey{}, id)));

        session = dest->back();
    });

    return session;
}

void Session::watch_streams()
{
    std::vector<std::shared_ptr<Session>> these_sessions;

    groups.use([&](decltype(groups)::type &v) { v.push_back(&these_sessions); });

    std::vector<std::vector<std::shared_ptr<Session>>::iterator> to_remove;

    while (is_live)
    {
        // TODO
        // don't lock groups at all, make access exclusive to this thread/function
        // create a synced "inbox" for adding new sessions that can be checked once per iteration of this loop
        groups.use([&](decltype(groups)::type &v) {
            for (auto it = these_sessions.begin(); it != these_sessions.end(); ++it)
            {
                if ((*it)->is_active.load())
                {
                    (*it)->tick();
                }
                else if ((*it).use_count() == 1) // use_count is not thread safe; however, if we do not make copies
                                                 // after Session::is_active is false, this should be okay
                {
                    to_remove.push_back(it);
                }
            }

            for (auto it = to_remove.rbegin(); it != to_remove.rend(); ++it)
            {
                these_sessions.erase(*it);
            }
        });
    }

    groups.use([&](decltype(groups)::type &v) {
        auto it = v.begin();
        for (; it != v.end(); ++it)
        {
            if (*it == &these_sessions)
            {
                break;
            }
        }

        v.erase(it);
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

void Session::play()
{
    is_playing = true;
}

void Session::play(float npt_begin, float npt_end)
{
    is_playing = true;

    play_range = {npt_begin, npt_end};

    if (npt_end)
    {
        for (auto &t : tracks)
        {
            t.set_play_range_end(npt_end);
        }
    }
}

void Session::pause(float npt)
{
    is_playing = false;

    for (auto &t : tracks)
    {
        t.set_play_time(npt);
    }
}

void Session::tick()
{
    if (is_playing)
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
