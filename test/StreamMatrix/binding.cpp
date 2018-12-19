
#include <stream_matrix.hpp>
#include <emscripten/bind.h>

using namespace emscripten;

namespace binding_utils {

#define CALLBACK(cb) specialization<std::function<void(int code, const std::string &data)>>(cb)

#ifdef NAPI_VERSION
template <typename T>
T specialization(const emscripten::val &callback)
{
    return callback.functor<T>();
}
#else
template <typename T>
emscripten::val specialization(const emscripten::val &callback)
{
    return callback;
}
#endif

void Initialize(StreamMatrix &obj, const emscripten::val &cb)
{
    obj.Initialize(CALLBACK(cb));
}

void Terminate(StreamMatrix &obj, const emscripten::val &cb)
{
    obj.Terminate(CALLBACK(cb));
}
void CreateRtspTestServer(StreamMatrix &obj,
                          const std::string &id,
                          int port,
                          const std::string &path,
                          const std::string &launch,
                          const emscripten::val &cb)
{
    nlohmann::json param;
    param["id"] = id;
    param["protocal"] = TestServerType::kRTSP20;
    param["path"] = path;
    param["port"] = port;
    param["launch"] = launch;

    obj.CreateApp(kTestServer, id, param.dump(), CALLBACK(cb));
}
void CreateLiveStream(StreamMatrix &obj,
                      const std::string &id,
                      const std::string &source_url,
                      const std::string &video_codec,
                      const std::string &audio_codec,
                      const emscripten::val &cb)
{
    nlohmann::json param;
    param["id"] = id;
    param["source_url"] = source_url;
    param["video_codec"] = video_codec;
    param["audio_codec"] = audio_codec;
    obj.CreateApp(kLiveStream, id, param.dump(), CALLBACK(cb));
}
void CreateMultiPoints(StreamMatrix &obj,
                       const std::string &id,
                       const std::string &video_codec,
                       const std::string &audio_codec,
                       const emscripten::val &cb)
{
    nlohmann::json param;
    param["id"] = id;
    param["video_codec"] = video_codec;
    param["audio_codec"] = audio_codec;
    obj.CreateApp(kMultiPoints, id, param.dump(), CALLBACK(cb));
}
void CreatePlayBack(StreamMatrix &obj,
                    const std::string &id,
                    const std::string &source_url,
                    const std::string &video_codec,
                    const std::string &audio_codec,
                    const emscripten::val &cb)
{
    nlohmann::json param;
    param["id"] = id;
    param["source_url"] = source_url;
    param["video_codec"] = video_codec;
    param["audio_codec"] = audio_codec;
    obj.CreateApp(kPlayBack, id, param.dump(), CALLBACK(cb));
}
// common
void DestroyApp(StreamMatrix &obj, const std::string &id, const emscripten::val &cb)
{
    obj.DestroyApp(id, CALLBACK(cb));
}
void StartUpApp(StreamMatrix &obj, const std::string &id, const emscripten::val &cb)
{
    nlohmann::json param;
    param["action"] = "startup";
    obj.OperateApp(id, param.dump(), CALLBACK(cb));
}
void StopApp(StreamMatrix &obj, const std::string &id, const emscripten::val &cb)
{
    nlohmann::json param;
    param["action"] = "stop";
    obj.OperateApp(id, param.dump(), CALLBACK(cb));
}
// livestream
void AddRtspAudience(StreamMatrix &obj,
                     const std::string &id,
                     int port,
                     const std::string &path,
                     const emscripten::val &cb)
{
    nlohmann::json param;
    param["action"] = "add_audience";
    param["protocal"] = EndpointType::kRtspServer;
    param["path"] = path;
    param["port"] = port;
    obj.OperateApp(id, param.dump(), CALLBACK(cb));
}
void AddWebrtcAudience(StreamMatrix &obj,
                       const std::string &id,
                       const std::string &signal_bridge,
                       const std::string &role,
                       const std::string &connection_id,
                       const emscripten::val &cb)
{
    nlohmann::json param;
    param["action"] = "add_audience";
    param["protocal"] = EndpointType::kWebrtc;
    param["signal_bridge"] = signal_bridge;
    param["connection_id"] = connection_id;
    param["role"] = (role == "Offer" ? WebrtcRole::kOffer : WebrtcRole::kAnswer);
    obj.OperateApp(id, param.dump(), CALLBACK(cb));
}
void AddHlsAudience(StreamMatrix &obj,
                    const std::string &id,
                    const emscripten::val &cb)
{
    nlohmann::json param;
    param["action"] = "add_audience";
    param["protocal"] = EndpointType::kHls;
    obj.OperateApp(id, param.dump(), CALLBACK(cb));
}
void RemoveAudience(StreamMatrix &obj, const std::string &id, const emscripten::val &cb)
{
    nlohmann::json param;
    param["action"] = "remove_audience";
    obj.OperateApp(id, param.dump(), CALLBACK(cb));
}
// multipoints
void AddMember(StreamMatrix &obj,
               const std::string &id,
               const std::string &signal_bridge,
               const std::string &role,
               const std::string &connection_id,
               const emscripten::val &cb)
{
    nlohmann::json param;
    param["action"] = "add_member";
    param["protocal"] = EndpointType::kWebrtc;
    param["signal_bridge"] = signal_bridge;
    param["connection_id"] = connection_id;
    param["role"] = (role == "Offer" ? WebrtcRole::kOffer : WebrtcRole::kAnswer);
    obj.OperateApp(id, param.dump(), CALLBACK(cb));
}
void RemoveMember(StreamMatrix &obj, const std::string &id, const emscripten::val &cb)
{
    nlohmann::json param;
    param["action"] = "remove_audience";
    obj.OperateApp(id, param.dump(), CALLBACK(cb));
}
void SetSpeaker(StreamMatrix &obj, const std::string &id, const emscripten::val &cb)
{
    nlohmann::json param;
    param["action"] = "set_speaker";
    obj.OperateApp(id, param.dump(), CALLBACK(cb));
}

void Seek(StreamMatrix &obj, const std::string &id, int pos, const emscripten::val &cb)
{
    nlohmann::json param;
    param["action"] = "seek";
    param["pos"] = "pos";
    obj.OperateApp(id, param.dump(), CALLBACK(cb));
}

void CreateRtspAnalyzer(StreamMatrix &obj,
                        const std::string &id,
                        const std::string &url,
                        const std::string &launch,
                        const emscripten::val &cb)
{
    nlohmann::json param;
    param["protocal"] = AnalyzerType::kRtsp;
    param["url"] = url;
    param["launch"] = launch;
    obj.CreateApp(kAnalyzer, id, param.dump(), CALLBACK(cb));
}

void CreateWebrtcAnalyzer(StreamMatrix &obj,
                          const std::string &id,
                          const std::string &signal_bridge,
                          const std::string &role,
                          const std::string &connection_id,
                          const std::string &launch,
                          const emscripten::val &cb)
{
    nlohmann::json param;
    param["protocal"] = AnalyzerType::kWebrtc;
    param["signal_bridge"] = signal_bridge;
    param["connection_id"] = connection_id;
    param["launch"] = launch;
    param["role"] = (role == "Offer" ? WebrtcRole::kOffer : WebrtcRole::kAnswer);
    obj.CreateApp(kAnalyzer, id, param.dump(), CALLBACK(cb));
}

void SetNotification(StreamMatrix &obj,
                     const emscripten::val &notify,
                     const emscripten::val &cb)
{
    obj.SetNotification(specialization<std::function<notify_fn>>(notify),
                        specialization<std::function<callback>>(cb));
}
}  // namespace binding_utils

EMSCRIPTEN_BINDINGS(binding_utils)
{
    emscripten::class_<StreamMatrix>("StreamMatrix")
        .constructor<>()
        // common function
        .function("Initialize", select_overload<void(StreamMatrix &, const emscripten::val &)>(&inding_utils::Initialize))
        .function("Terminate", select_overload<void(StreamMatrix &, const emscripten::val &)>(&binding_utils::Terminate)
        .function("DestroyApp",select_overload<void(StreamMatrix &, const std::string &, const emscripten::val &)>(&binding_utils::DestroyApp))
        .function("StartUpApp",select_overload<void(StreamMatrix &, const std::string &, const emscripten::val &)>(&binding_utils::StartUpApp))
        .function("StopApp",select_overload<void(StreamMatrix &, const std::string &, const emscripten::val &)>(&binding_utils::StopApp))
        // test server
        .function("CreateRtspTestServer", select_overload<void(StreamMatrix &, const std::string &, int, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::CreateRtspTestServer))
        // livestream
        .function("CreateLiveStream", select_overload<void(StreamMatrix &, const std::string &, const std::string &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::CreateLiveStream))
        .function("AddRtspAudience",select_overload<void(StreamMatrix &, const std::string &, int, const std::string &, const emscripten::val &)>(&binding_utils::AddRtspAudience))
        .function("AddWebrtcAudience",select_overload<void(StreamMatrix &, const std::string &, const std::string &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::AddWebrtcAudience))
        .function("AddHlsAudience",select_overload<void(StreamMatrix &, const std::string &, const emscripten::val &)>(&binding_utils::AddHlsAudience))
        .function("RemoveAudience",select_overload<void(StreamMatrix &, const std::string &, const emscripten::val &)>(&binding_utils::RemoveAudience))
        // multipoints
        .function("CreateMultiPoints", select_overload<void(StreamMatrix &, const std::string &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::CreateMultiPoints))
        .function("AddMember",select_overload<void(StreamMatrix &, const std::string &, const std::string &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::AddMember))
        .function("RemoveMember",select_overload<void(StreamMatrix &, const std::string &, const emscripten::val &)>(&binding_utils::RemoveMember))
        .function("SetSpeaker",select_overload<void(StreamMatrix &, const std::string &, const emscripten::val &)>(&binding_utils::SetSpeaker))
        // playback
        .function("CreatePlayBack", select_overload<void(StreamMatrix &, const std::string &, const std::string &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::CreatePlayBack))
        .function("Seek",select_overload<void(StreamMatrix &, const std::string &, int, const emscripten::val &)>(&binding_utils::Seek))

        // Analyzer
        .function("CreateRtspAnalyzer",select_overload<void(StreamMatrix &, const std::string &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::CreateRtspAnalyzer))
        .function("CreateWebrtcAnalyzer",select_overload<void(StreamMatrix &, const std::string &, const std::string &, const std::string &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::CreateWebrtcAnalyzer))

        .function("SetNotification",select_overload<void(StreamMatrix &, const emscripten::val &, const emscripten::val &)>(&binding_utils::SetNotification));
}