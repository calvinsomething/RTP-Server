#include <functional>

class Defer
{
  public:
    Defer(std::function<void()> fn);
    ~Defer();

  private:
    std::function<void()> fn;
};
