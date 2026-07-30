#pragma once

#include <functional>
#include <mutex>
#include <shared_mutex>

template <typename T> class Locker
{
  public:
    Locker() = default;
    Locker(Locker &&other) noexcept : t(std::move(other.t))
    {
    }

    template <typename... Args> Locker(Args &&...args) : t(std::forward<Args>(args)...)
    {
    }

    Locker &operator=(Locker &&other)
    {
        std::scoped_lock lock(mutex, other.mutex);

        std::swap(t, other.t);

        return *this;
    }

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
