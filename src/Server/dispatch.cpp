#include "dispatch.h"

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
    return RTSPResponse();
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
