#include "Session.h"

#include "Exception.h"

RNG<uint64_t> Session::rng;

// TODO atomic/locking map
std::unordered_map<std::string, Session> Session::sessions;

Session &Session::get(const std::string &id)
{
    auto s = sessions.find(id);
    if (s == sessions.end())
    {
        throw Exception("Invalid session error."); // TODO create exception types that can be handled accordingly
    }

    return s->second;
}

Session &Session::get()
{
    std::pair<std::unordered_map<std::string, Session>::iterator, bool> s;
    do
    {
        std::string id = generate_id();
        s = sessions.try_emplace(id, PrivateKey{}, id);
    } while (!s.second);

    return s.first->second;
}

std::string Session::generate_id()
{
    uint64_t bits = rng.get();

    unsigned char *start = reinterpret_cast<unsigned char *>(&bits);
    unsigned char *end = reinterpret_cast<unsigned char *>(&bits + 1);

    for (unsigned char *c = start; c != end; ++c)
    {
        *c = *c % 26 + 65;
    }

    return std::string(reinterpret_cast<char *>(start), 8);
}

Session::Session(PrivateKey key, const std::string &id) : id(id)
{
}

void Session::end()
{
    sessions.erase(id);
}

std::string Session::get_id()
{
    return id;
}
