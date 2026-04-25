#include "dispatch.h"

#include "../util/misc.h"
#include "RTSPResponse.h"

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
    return RTSPResponse();
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
    return RTSPResponse();
}

RTSPResponse handle_pause(const RTSPRequest &request)
{
    return RTSPResponse();
}

RTSPResponse handle_teardown(const RTSPRequest &request)
{
    return RTSPResponse();
}
}; // namespace Dispatch
