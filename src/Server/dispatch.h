#pragma once

#include <string>
#include <unordered_map>

#include "RTSPRequest.h"
#include "RTSPResponse.h"

namespace Dispatch
{
typedef RTSPResponse (*RTSPHandler)(const RTSPRequest &);

RTSPResponse handle_describe(const RTSPRequest &request);

RTSPResponse handle_setup(const RTSPRequest &request);

RTSPResponse handle_options(const RTSPRequest &request);

RTSPResponse handle_play(const RTSPRequest &request);

RTSPResponse handle_pause(const RTSPRequest &request);

RTSPResponse handle_teardown(const RTSPRequest &request);

static const std::unordered_map<std::string, RTSPHandler> rtsp{
    {"DESCRIBE", handle_describe}, //
    {"SETUP", handle_setup},       //
    {"OPTIONS", handle_options},   //
    {"PLAY", handle_play},         //
    {"PAUSE", handle_pause},       //
    {"TEARDOWN", handle_teardown}, //
};
}; // namespace Dispatch
