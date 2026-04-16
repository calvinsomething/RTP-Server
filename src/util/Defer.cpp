#include "Defer.h"

Defer::Defer(std::function<void()> fn) : fn(fn)
{
}

Defer::~Defer()
{
    fn();
}
