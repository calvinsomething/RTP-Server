#include "dispatch.h"

#include <optional>

#include "Exception.h"
#include "RTSPRequest.h"
#include "RTSPResponse.h"
#include "Session.h"
#include "util/misc.h"

namespace Dispatch
{
RTSPResponse handle_describe(const RTSPRequest &request)
{
    RTSPResponse response;

    response.body = HARD_CODED_SDP;

    response.set_header("CSeq", request.get_header("cseq"));

    response.set_header("Date", get_date_string(std::chrono::system_clock::now()));

    response.set_header("Content-Type", "application/sdp");

    response.set_header("Content-Length", int_to_string(response.body.size()));

    response.set_status(RTSPResponse::StatusCode::OK);

    response.marshal();

    return response;
}

RTSPResponse handle_setup(const RTSPRequest &request)
{
    std::string session_id = request.get_header("session");

    std::shared_ptr<Session> session = session_id.empty() ? Session::get() : Session::get(session_id);

    Track *track = 0;
    std::optional<Exception> first_exception;

    std::string transport_string = request.get_header("transport");

    // Try all transport header values before throwing exception
    for (std::string_view transport_value : split(transport_string, ','))
    {
        try
        {
            track = &session->emplace_track(request.client_addr, request.get_uri(), transport_value);
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

    RTSPResponse response;

    response.set_header("CSeq", request.get_header("cseq"));

    response.set_header("Date", get_date_string(std::chrono::system_clock::now()));

    response.set_header("Session", session->get_id());

    response.set_header("Transport", track->transport.get_string());

    response.set_status(RTSPResponse::StatusCode::OK);

    response.marshal();

    return response;
}

RTSPResponse handle_options(const RTSPRequest &request)
{
    RTSPResponse response;

    response.set_header("CSeq", request.get_header("cseq"));
    response.set_header("Public", "DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE");

    response.set_status(RTSPResponse::StatusCode::OK);

    response.marshal();

    return response;
}

RTSPResponse handle_play(const RTSPRequest &request)
{
    std::string session_id = request.get_header("session");

    std::shared_ptr<Session> session = Session::get(session_id);

    std::string range = request.get_header("range");

    if (range.empty())
    {
        session->play();
    }
    else
    {
        auto parts = split(range, '-');

        float begin;
        auto result = std::from_chars(parts[0].cbegin(), parts[0].cend(), begin);
        THROW_IF_FALSE((result.ec == std::errc{}), parts[0]);

        float end = 0;
        if (parts.size() > 1)
        {
            result = std::from_chars(parts[0].cbegin(), parts[0].cend(), end);
            THROW_IF_FALSE((result.ec == std::errc{}), parts[0]);
        }

        session->play(begin, end);
    }

    return RTSPResponse();
}

RTSPResponse handle_pause(const RTSPRequest &request)
{
    return RTSPResponse();
}

RTSPResponse handle_teardown(const RTSPRequest &request)
{
    std::shared_ptr<Session> session = Session::get(request.get_header("session"));

    session->teardown();

    return RTSPResponse();
}
}; // namespace Dispatch
