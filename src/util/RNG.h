#pragma once

#include <random>
#include <type_traits>

template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>> class RNG
{
  public:
    static T get()
    {
        return distribution(generator);
    }

  private:
    static std::mt19937_64 generator;
    static std::uniform_int_distribution<T> distribution;
};

template <typename T, typename E> std::mt19937_64 RNG<T, E>::generator(std::random_device{}());

template <typename T, typename E> std::uniform_int_distribution<T> RNG<T, E>::distribution;
