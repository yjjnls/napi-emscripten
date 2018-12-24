#ifndef STREAM_MATRIX_FRAMEWORK_COMMON_HPP
#define STREAM_MATRIX_FRAMEWORK_COMMON_HPP

#include <vector>
#include <list>
#include <map>
#include <string>
#include <functional>
#include <algorithm>
#include <utils/nlohmann/json.hpp>
#include <gst/gst.h>

typedef std::function<void(int code, const std::string &data)> callback;
typedef std::function<void(const std::string &meta, const std::string &data)> notify_fn;

enum MediaType
{
    kTestServer = 0,
    kLiveStream,
    kMultiPoints,
    kPlayBack,
    kAnalyzer
};

enum TestServerType
{
    kRFC7826 = 0,  // RTSP 2.0 RFC7826
    kONVIF,
    kRTSP20 = kRFC7826,
    kHLS
};

enum AnalyzerType
{
    kRtsp = 0,
    kWebrtcRecv,
    kWebrtcSendRecv
};

enum EndpointType
{
    kRtspServer = 0,
    kRtspClient,
    kWebrtc,
    kHls
};

enum WebrtcRole
{
    kOffer = 0,
    kAnswer
};

#endif
