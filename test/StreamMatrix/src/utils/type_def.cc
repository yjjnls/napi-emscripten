
#include <string>
#include <map>
#include <utils/type_def.hpp>


// std::map<std::string, EndpointType> endpoint_type = {{"RTSPCLIENT", EndpointType::RTSP_CLIENT},
//                                                      {"RTSPSERVER", EndpointType::RTSP_SERVER},
//                                                      {"TESTSINK", EndpointType::TEST_SINK},
//                                                      {"WEBRTC", EndpointType::WEBRTC}};
// EndpointType get_endpoint_type(const std::string &type)
// {
//     return endpoint_type[type];
// }
std::map<std::string, VideoEncodingType> video_encoding_type = {{"H264", VideoEncodingType::H264},
                                                                {"VP8", VideoEncodingType::VP8},
                                                                {"H265", VideoEncodingType::H265}};
VideoEncodingType get_video_encoding_type(const std::string &type)
{
    return video_encoding_type[type];
}
std::map<std::string, AudioEncodingType> audio_encoding_type = {{"PCMA", AudioEncodingType::PCMA},
                                                                {"PCMU", AudioEncodingType::PCMU},
                                                                {"OPUS", AudioEncodingType::OPUS}};
AudioEncodingType get_audio_encoding_type(const std::string &type)
{
    return audio_encoding_type[type];
}

std::string uppercase(const std::string &target)
{
    std::string result = target;
    for (int i = 0; i < result.size(); ++i) {
        if (result[i] >= 97 && result[i] <= 122) {
            result[i] -= 32;
        }
    }
    return result;
}
