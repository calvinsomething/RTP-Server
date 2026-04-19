#include "dispatch.h"
#include "RTSPResponse.h"

namespace Dispatch
{
RTSPResponse handle_describe(const RTSPRequest &request)
{
    return RTSPResponse(request.get_method());
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

    response.set_status(Status::OK);

    response.marshal_data();

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
