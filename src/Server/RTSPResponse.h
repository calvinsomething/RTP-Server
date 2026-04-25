#pragma once

#include <string>
#include <unordered_map>

constexpr std::string_view HARD_CODED_SDP = "v=0\n"
                                            "o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4\n"
                                            "s=SDP Seminar\n"
                                            "i=A Seminar on the session description protocol\n"
                                            "u=http://www.cs.ucl.ac.uk/staff/M.Handley/sdp.03.ps\n"
                                            "e=mjh@isi.edu (Mark Handley)\n"
                                            "c=IN IP4 224.2.17.12/127\n"
                                            "t=2873397496 2873404696\n"
                                            "a=recvonly\n"
                                            "m=audio 3456 RTP/AVP 0\n"
                                            "m=video 2232 RTP/AVP 31\n"
                                            "m=whiteboard 32416 UDP WB\n"
                                            "a=orient:portrait\n\n";

class RTSPResponse
{
  public:
    enum class StatusCode
    {
        OK = 200,
    };

    std::string body;

    RTSPResponse() = default;
    RTSPResponse(const std::string &method);

    const char *get_data() const;
    size_t get_length() const;

    void set_header(const std::string &key, const std::string &value);
    void append_header(const std::string &key, const std::string &value);

    void set_status(StatusCode sc);

    void marshal();

  private:
    std::string buffer;
    std::string_view status;

    std::string_view get_status_string(StatusCode sc);
    std::unordered_map<std::string, std::string> headers;
};
