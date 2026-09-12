#include "NPT.h"

#include "Server/Exception.h"
#include <iostream>

#define DEC_1 10
#define DEC_2 100
#define DEC_3 1000
#define DEC_4 10000

#define DECIMAL_PLACES(value, n) (float(int(value * DEC_##n)) / DEC_##n)

// npt-range    =   ( npt-time "-" [ npt-time ] ) | ( "-" npt-time )
// npt-time     =   npt-sec | npt-hhmmss // "now" omited, as it may only be used for live streams
// npt-sec      =   1*DIGIT [ "." *DIGIT ]
// npt-hhmmss   =   npt-hh ":" npt-mm ":" npt-ss [ "." *DIGIT ]
// npt-hh       =   1*DIGIT     ; any positive number
// npt-mm       =   1*2DIGIT    ; 0-59
// npt-ss       =   1*2DIGIT    ; 0-59

NPT::NPT(float t) : t(t)
{
}

NPT::~NPT()
{
}

void NPT::set_mode(Mode mode)
{
    this->mode = mode;
}

std::string NPT::get_time() const
{
    switch (mode)
    {
    case Mode::HHMMSS:
        return hhmmss();
    default:
        return sec();
    }
}

std::string NPT::sec() const
{
    std::string s(15, 0);

    auto result = std::to_chars(s.data(), s.end().base(), DECIMAL_PLACES(t, 2));
    if (result.ec != std::errc{})
    {
        throw Exception(DEBUG_STR("Invalid npt value: %f"), t);
    }

    s.resize(s.size() - (s.end().base() - result.ptr));

    return s;
}

std::string NPT::hhmmss() const
{
    std::string s(15, 0);

    int t_int = t;

    int hh_mm_ss[3] = {t_int / 3600}, t_mod_h = t_int - hh_mm_ss[0] * 3600;
    hh_mm_ss[1] = t_mod_h / 60;
    hh_mm_ss[2] = t_mod_h - hh_mm_ss[1] * 60;

    char *begin = s.data(), *end = s.end().base() - 1;

    for (int segment : hh_mm_ss)
    {
        auto result = std::to_chars(begin, end, segment);
        if (result.ec != std::errc{})
        {
            throw Exception(DEBUG_STR("Invalid npt value: %d"), segment);
        }

        begin = result.ptr;
        *begin++ = ':';
    }

    s.resize(s.size() - (s.end().base() - begin) - 1);

    return s;
}

std::string NPT::format_range(const NPT &begin, const NPT &end)
{
    return begin.get_time() + '-' + end.get_time();
}
