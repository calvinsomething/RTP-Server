#pragma once

#include <functional>
#include <mutex>
#include <shared_mutex>

template <typename T> class Locker
{
  public:
    using type = T;

    void use(std::function<void(T)> fn) = delete;
    void use(std::function<void(T &)> fn)
    {
        std::lock_guard lock(mutex);
        fn(t);
    }

  private:
    std::mutex mutex;
    T t;
};

template <typename T> class RWLocker
{
  public:
    using type = T;

    void read(std::function<void(T)> fn) = delete;
    void read(std::function<void(T &)> fn)
    {
        std::shared_lock lock(mutex);
        fn(t);
    }

    void write(std::function<void(T)> fn) = delete;
    void write(std::function<void(T &)> fn)
    {
        std::lock_guard lock(mutex);
        fn(t);
    }

  private:
    std::shared_mutex mutex;
    T t;
};
