#include <set>
#include <string>
#include <unordered_map>

class RTSPRequest
{
  public:
    RTSPRequest() = default;
    RTSPRequest(const std::string &message);

    void parse_request(const std::string &message);

    bool ready() const;

    std::string get_method() const;
    std::string get_uri() const;
    std::string get_version() const;
    std::string get_header(std::string key) const;

    const std::string &get_body() const;

  private:
    static constexpr size_t PARSE_COMPLETE = -1;

    static const std::string delimiters[3];
    static const std::set<std::string> unique_headers;

    std::string method, uri, version, body;
    std::unordered_map<std::string, std::string> headers;

    void set_request_line_parts(std::string_view line);
    void add_header(std::string_view line);

    size_t cursor = 0;
};
