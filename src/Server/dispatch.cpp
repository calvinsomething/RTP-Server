#include "dispatch.h"

#include <optional>

#include "Exception.h"
#include "RTSPRequest.h"
#include "RTSPResponse.h"
#include "Server/Media.h"
#include "Session.h"
#include "util/NPT.h"
#include "util/misc.h"

// Type "g" designates general request headers to be found in both requests
// and responses, type "R" designates request headers, type "r" designates
// response headers, and type "e" designates entity header fields.
//
//  Header               type   support   methods
//
//  Accept               R      opt.      entity
//  Accept-Encoding      R      opt.      entity
//  Accept-Language      R      opt.      all
//  Allow                r      opt.      all
//  Authorization        R      opt.      all
//  Bandwidth            R      opt.      all
//  Blocksize            R      opt.      all but OPTIONS, TEARDOWN
//  Cache-Control        g      opt.      SETUP
//  Conference           R      opt.      SETUP
//  Connection           g      req.      all
//  Content-Base         e      opt.      entity
//  Content-Encoding     e      req.      SET_PARAMETER
//  Content-Encoding     e      req.      DESCRIBE, ANNOUNCE
//  Content-Language     e      req.      DESCRIBE, ANNOUNCE
//  Content-Length       e      req.      SET_PARAMETER, ANNOUNCE
//  Content-Length       e      req.      entity
//  Content-Location     e      opt.      entity
//  Content-Type         e      req.      SET_PARAMETER, ANNOUNCE
//  Content-Type         r      req.      entity
//  CSeq                 g      req.      all
//  Date                 g      opt.      all
//  Expires              e      opt.      DESCRIBE, ANNOUNCE
//  From                 R      opt.      all
//  If-Modified-Since    R      opt.      DESCRIBE, SETUP
//  Last-Modified        e      opt.      entity
//  Proxy-Authenticate
//  Proxy-Require        R      req.      all
//  Public               r      opt.      all
//  Range                R      opt.      PLAY, PAUSE, RECORD
//  Range                r      opt.      PLAY, PAUSE, RECORD
//  Referer              R      opt.      all
//  Require              R      req.      all
//  Retry-After          r      opt.      all
//  RTP-Info             r      req.      PLAY
//  Scale                Rr     opt.      PLAY, RECORD
//  Session              Rr     req.      all but SETUP, OPTIONS
//  Server               r      opt.      all
//  Speed                Rr     opt.      PLAY
//  Transport            Rr     req.      SETUP
//  Unsupported          r      req.      all
//  User-Agent           R      opt.      all
//  Via                  g      opt.      all
//  WWW-Authenticate     r      opt.      all

std::string_view get_endpoint_from_url(std::string_view url)
{
    size_t endpoint_index = util::find_nth_of(url, '/', 3);

    if (endpoint_index != std::string::npos && endpoint_index < url.size() - 1)
    {
        return std::string_view(url.begin() + endpoint_index + 1, url.end());
    }

    return "";
}

namespace Dispatch
{
RTSPResponse handle_describe(const RTSPRequest &request)
{
    RTSPResponse response;

    std::string_view endpoint = get_endpoint_from_url(request.get_url());

    if (!endpoint.empty())
    {
        auto media = Media::by_basename.find(endpoint);
        if (media != Media::by_basename.end())
        {
            if (request.get_header("accept").find("application/sdp") == std::string::npos)
            {
                response.set_status(RTSPResponse::StatusCode::NotAcceptable);
            }

            response.body = media->second->sdp;

            response.set_header("CSeq", request.get_header("cseq"));

            response.set_header("Date", util::get_date_string(std::chrono::system_clock::now()));

            response.set_header("Content-Type", "application/sdp");

            response.set_header("Content-Length", util::int_to_string(response.body.size()));

            response.set_status(RTSPResponse::StatusCode::OK);
        }
        else
        {
            response.set_status(RTSPResponse::StatusCode::NotFound);
        }
    }
    else
    {
        response.set_status(RTSPResponse::StatusCode::NotFound);
    }

    return response;
}

RTSPResponse handle_setup(const RTSPRequest &request)
{
    RTSPResponse response;

    auto url_parts = util::split(get_endpoint_from_url(request.get_url()), '/');
    if (url_parts.size() == 2)
    {
        std::string session_id = request.get_header("session");

        std::shared_ptr<Session> session = session_id.empty() ? Session::get() : Session::get(session_id);

        Track *track = 0;
        std::optional<Exception> first_exception;

        std::string transport_string = request.get_header("transport");

        // Try all transport header values before throwing exception
        for (std::string_view transport_value : util::split(transport_string, ','))
        {
            try
            {
                track = &session->emplace_track(request.client_addr, url_parts[0], url_parts[1], transport_value);
                break;
            }
            catch (Exception &e)
            {
                if (!first_exception.has_value())
                {
                    first_exception.emplace(e);
                }
            }
        }

        if (!track)
        {
            throw first_exception.value();
        }

        response.set_header("CSeq", request.get_header("cseq"));

        response.set_header("Date", util::get_date_string(std::chrono::system_clock::now()));

        response.set_header("Session", session->get_id());

        response.set_header("Transport", track->transport.get_string());

        response.set_status(RTSPResponse::StatusCode::OK);
    }
    else if (url_parts.size() == 1)
    {
        response.set_status(RTSPResponse::StatusCode::AggregateOperationNotAllowed);
    }
    else
    {
        response.set_status(RTSPResponse::StatusCode::NotFound);
    }

    return response;
}

RTSPResponse handle_options(const RTSPRequest &request)
{
    RTSPResponse response;

    response.set_header("CSeq", request.get_header("cseq"));
    response.set_header("Public", "DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE");

    response.set_status(RTSPResponse::StatusCode::OK);

    return response;
}

// Only supporting NPT format as permitted by the RFC:
// A media server only supporting playback MUST support the npt format
// and MAY support the clock and smpte formats.
RTSPResponse handle_play(const RTSPRequest &request)
{
    std::string session_id = request.get_header("session");

    std::shared_ptr<Session> session = Session::get(session_id);

    std::string range = request.get_header("range");

    std::pair<NPT, NPT> play_range;

    if (range.empty())
    {
        play_range = session->play();
    }
    else
    {
        auto f_range = NPT::parse_range_header(range);

        play_range = session->play(f_range.first, f_range.second);
    }

    RTSPResponse response;

    response.set_header("CSeq", request.get_header("cseq"));

    response.set_header("Date", util::get_date_string(std::chrono::system_clock::now()));

    response.set_header("Range", std::string("npt=") + NPT::format_range(play_range.first, play_range.second));

    response.set_status(RTSPResponse::StatusCode::OK);

    return response;
}

RTSPResponse handle_pause(const RTSPRequest &request)
{
    // RFC: If the Range header specifies a time outside
    // any currently pending PLAY requests, the error "457 Invalid Range" is
    // returned.

    std::string session_id = request.get_header("session");

    std::shared_ptr<Session> session = Session::get(session_id);

    session->pause();

    RTSPResponse response;

    response.set_header("CSeq", request.get_header("cseq"));

    response.set_header("Date", util::get_date_string(std::chrono::system_clock::now()));

    response.set_status(RTSPResponse::StatusCode::OK);

    return response;
}

RTSPResponse handle_teardown(const RTSPRequest &request)
{
    std::shared_ptr<Session> session = Session::get(request.get_header("session"));

    session->teardown();

    return RTSPResponse();
}
}; // namespace Dispatch
