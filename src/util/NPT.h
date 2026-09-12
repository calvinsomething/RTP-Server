#include <string>

class NPT
{
  public:
    enum class Mode
    {
        SEC,
        HHMMSS,
    };

    static std::pair<float, float> parse_range_header(std::string header_value);
    static std::string format_range(const NPT &begin, const NPT &end);

    NPT(float t = 0);
    ~NPT();

    std::string get_time() const;

    void set_mode(Mode mode);

  private:
    float t = 0;
    Mode mode = Mode::SEC;

    std::string sec() const;
    std::string hhmmss() const;
};
