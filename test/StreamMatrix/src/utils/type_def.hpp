#ifndef STREAM_MATRIX_UTILS_TYPE_DEFINE_HPP
#define STREAM_MATRIX_UTILS_TYPE_DEFINE_HPP

#include <string>
// enum EndpointType
// {
//     RTSP_CLIENT = (1 << 0),
//     RTSP_SERVER = (1 << 1),
//     TEST_SINK = (1 << 2),
//     WEBRTC = (1 << 3)
// };
enum VideoEncodingType
{
    H264 = (1 << 0),
    VP8 = (1 << 1),
    H265 = (1 << 2)
};

enum AudioEncodingType
{
    PCMA = (1 << 0),
    PCMU = (1 << 1),
    OPUS = (1 << 2)
};

// EndpointType get_endpoint_type(const std::string &type);
VideoEncodingType get_video_encoding_type(const std::string &type);
AudioEncodingType get_audio_encoding_type(const std::string &type);

std::string uppercase(const std::string &target);

#endif