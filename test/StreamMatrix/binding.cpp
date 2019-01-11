
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
    nlohmann::json meta;
    meta["action"] = "create";
    nlohmann::json param;
    param["type"] = kTestServer;
    param["id"] = id;
    param["protocol"] = ServerType::kRTSP20;
    param["path"] = path;
    param["port"] = port;
    param["launch"] = launch;

    obj.Call(meta, param, CALLBACK(cb));
}
void CreateLiveStream(StreamMatrix &obj,
                      const std::string &id,
                      const std::string &source_url,
                      const std::string &video_codec,
                      const std::string &audio_codec,
                      const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "create";
    nlohmann::json param;
    param["type"] = kLiveStream;
    param["id"] = id;
    param["source_url"] = source_url;
    param["video_codec"] = video_codec;
    param["audio_codec"] = audio_codec;

    obj.Call(meta, param, CALLBACK(cb));
}
void CreateMultiPoints(StreamMatrix &obj,
                       const std::string &id,
                       const std::string &video_codec,
                       const std::string &audio_codec,
                       const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "create";
    nlohmann::json param;
    param["type"] = kMultiPoints;
    param["id"] = id;
    param["video_codec"] = video_codec;
    param["audio_codec"] = audio_codec;

    obj.Call(meta, param, CALLBACK(cb));
}
void CreatePlayBack(StreamMatrix &obj,
                    const std::string &id,
                    const std::string &source_url,
                    const std::string &video_codec,
                    const std::string &audio_codec,
                    const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "create";
    nlohmann::json param;
    param["type"] = kPlayBack;
    param["id"] = id;
    param["source_url"] = source_url;
    param["video_codec"] = video_codec;
    param["audio_codec"] = audio_codec;

    obj.Call(meta, param, CALLBACK(cb));
}
// common
void DestroyApp(StreamMatrix &obj, const std::string &id, const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "destroy";
    nlohmann::json param;
    param["id"] = id;

    obj.Call(meta, param, CALLBACK(cb));
}
void StartUpApp(StreamMatrix &obj, const std::string &id, const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "startup";
    nlohmann::json param;
    param["id"] = id;

    obj.Call(meta, param, CALLBACK(cb));
}
void StopApp(StreamMatrix &obj, const std::string &id, const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "stop";
    nlohmann::json param;
    param["id"] = id;

    obj.Call(meta, param, CALLBACK(cb));
}
// livestream
void AddRtspAudience(StreamMatrix &obj,
                     const std::string &app_id,
                     const std::string &endpoint_id,
                     int port,
                     const std::string &path,
                     const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "add_audience";
    nlohmann::json param;
    param["id"] = app_id;
    param["endpoint_id"] = endpoint_id;
    param["protocol"] = kRtspServer;
    param["path"] = path;
    param["port"] = port;

    obj.Call(meta, param, CALLBACK(cb));
}
void AddWebrtcAudience(StreamMatrix &obj,
                       const std::string &app_id,
                       const std::string &endpoint_id,
                       const std::string &role,
                       const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "add_audience";
    nlohmann::json param;
    param["id"] = app_id;
    param["endpoint_id"] = endpoint_id;
    param["protocol"] = kWebrtc;
    param["role"] = (role == "offer" ? kOffer : kAnswer);
    obj.Call(meta, param, CALLBACK(cb));
}
void SetRemoteDescription(StreamMatrix &obj,
                          const std::string &app_id,
                          const std::string &endpoint_id,
                          const std::string &type,
                          const std::string &sdp,
                          const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "remote_sdp";
    nlohmann::json param;
    param["id"] = app_id;
    param["endpoint_id"] = endpoint_id;
    param["type"] = type;
    param["sdp"] = sdp;
    obj.Call(meta, param, CALLBACK(cb));
}
void SetRemoteCandidate(StreamMatrix &obj,
                        const std::string &app_id,
                        const std::string &endpoint_id,
                        const std::string &candidate,
                        int sdpMLineIndex,
                        const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "remote_candidate";
    nlohmann::json param;
    param["id"] = app_id;
    param["endpoint_id"] = endpoint_id;
    param["candidate"] = candidate;
    param["sdpMLineIndex"] = sdpMLineIndex;
    obj.Call(meta, param, CALLBACK(cb));
}
void AddHlsAudience(StreamMatrix &obj,
                    const std::string &app_id,
                    const std::string &endpoint_id,
                    const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "add_audience";
    nlohmann::json param;
    param["id"] = app_id;
    param["endpoint_id"] = endpoint_id;
    param["protocol"] = kHls;
    obj.Call(meta, param, CALLBACK(cb));
}
void RemoveAudience(StreamMatrix &obj,
                    const std::string &app_id,
                    const std::string &endpoint_id,
                    const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "remove_audience";
    nlohmann::json param;
    param["id"] = app_id;
    param["endpoint_id"] = endpoint_id;

    obj.Call(meta, param, CALLBACK(cb));
}
// multipoints
void AddMember(StreamMatrix &obj,
               const std::string &app_id,
               const std::string &endpoint_id,
               const std::string &role,
               const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "add_member";
    nlohmann::json param;
    param["id"] = app_id;
    param["endpoint_id"] = endpoint_id;
    param["protocol"] = kWebrtcSendRecv;
    param["role"] = (role == "offer" ? kOffer : kAnswer);
    obj.Call(meta, param, CALLBACK(cb));
}
void RemoveMember(StreamMatrix &obj,
                  const std::string &app_id,
                  const std::string &endpoint_id,
                  const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "remove_member";
    nlohmann::json param;
    param["id"] = app_id;
    param["endpoint_id"] = endpoint_id;

    obj.Call(meta, param, CALLBACK(cb));
}
void SetSpeaker(StreamMatrix &obj,
                const std::string &app_id,
                const std::string &endpoint_id,
                const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "set_speaker";
    nlohmann::json param;
    param["id"] = app_id;
    param["endpoint_id"] = endpoint_id;

    obj.Call(meta, param, CALLBACK(cb));
}

void Seek(StreamMatrix &obj, const std::string &id, int pos, const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "seek";
    nlohmann::json param;
    param["id"] = id;
    param["pos"] = "pos";

    obj.Call(meta, param, CALLBACK(cb));
}

void CreateRtspAnalyzer(StreamMatrix &obj,
                        const std::string &id,
                        const std::string &launch,
                        const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "create";
    nlohmann::json param;
    param["type"] = kAnalyzer;
    param["id"] = id;
    param["protocol"] = kRtspClient;
    param["launch"] = launch;

    obj.Call(meta, param, CALLBACK(cb));
}

void CreateWebrtcAnalyzer(StreamMatrix &obj,
                          const std::string &id,
                          const std::string &role,
                          const std::string &launch,
                          const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "create";
    nlohmann::json param;
    param["type"] = kAnalyzer;
    param["id"] = id;
    param["protocol"] = kWebrtc;
    param["launch"] = launch;
    param["role"] = (role == "offer" ? kOffer : kAnswer);

    obj.Call(meta, param, CALLBACK(cb));
}

void SetNotification(StreamMatrix &obj,
                     const emscripten::val &notify,
                     const emscripten::val &cb)
{
    obj.SetNotification(specialization<notify_fn>(notify), specialization<callback>(cb));
}
void GetVersion(StreamMatrix &obj, const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "version";
    obj.Call(meta, nlohmann::json(), CALLBACK(cb));
}
void CreateLauncher(StreamMatrix &obj, const std::string &id, const std::string &launch, const emscripten::val &cb)
{
    nlohmann::json meta;
    meta["action"] = "create";
    nlohmann::json param;
    param["type"] = kLauncher;
    param["id"] = id;
    param["launch"] = launch;

    obj.Call(meta, param, CALLBACK(cb));
}
}  // namespace binding_utils

EMSCRIPTEN_BINDINGS(binding_utils)
{
    emscripten::class_<StreamMatrix>("StreamMatrix")
        .constructor<>()

        .function("Initialize", select_overload<void(StreamMatrix &, const emscripten::val &)>(&binding_utils::Initialize))
        .function("Terminate", select_overload<void(StreamMatrix &, const emscripten::val &)>(&binding_utils::Terminate))
        .function("Destroy", select_overload<void(StreamMatrix &, const std::string &, const emscripten::val &)>(&binding_utils::DestroyApp))
        .function("StartUp", select_overload<void(StreamMatrix &, const std::string &, const emscripten::val &)>(&binding_utils::StartUpApp))
        .function("Stop", select_overload<void(StreamMatrix &, const std::string &, const emscripten::val &)>(&binding_utils::StopApp))

        .function("CreateRtspTestServer", select_overload<void(StreamMatrix &, const std::string &, int, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::CreateRtspTestServer))

        .function("CreateLiveStream", select_overload<void(StreamMatrix &, const std::string &, const std::string &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::CreateLiveStream))
        .function("AddRtspAudience", select_overload<void(StreamMatrix &, const std::string &, const std::string &, int, const std::string &, const emscripten::val &)>(&binding_utils::AddRtspAudience))
        .function("AddWebrtcAudience", select_overload<void(StreamMatrix &, const std::string &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::AddWebrtcAudience))
        .function("SetRemoteDescription", select_overload<void(StreamMatrix &, const std::string &, const std::string &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::SetRemoteDescription))
        .function("SetRemoteCandidate", select_overload<void(StreamMatrix &, const std::string &, const std::string &, const std::string &, int, const emscripten::val &)>(&binding_utils::SetRemoteCandidate))
        .function("AddHlsAudience", select_overload<void(StreamMatrix &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::AddHlsAudience))
        .function("RemoveAudience", select_overload<void(StreamMatrix &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::RemoveAudience))

        .function("CreateMultiPoints", select_overload<void(StreamMatrix &, const std::string &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::CreateMultiPoints))
        .function("AddMember", select_overload<void(StreamMatrix &, const std::string &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::AddMember))
        .function("RemoveMember", select_overload<void(StreamMatrix &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::RemoveMember))
        .function("SetSpeaker", select_overload<void(StreamMatrix &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::SetSpeaker))

        .function("CreatePlayBack", select_overload<void(StreamMatrix &, const std::string &, const std::string &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::CreatePlayBack))
        .function("Seek", select_overload<void(StreamMatrix &, const std::string &, int, const emscripten::val &)>(&binding_utils::Seek))


        .function("CreateRtspAnalyzer", select_overload<void(StreamMatrix &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::CreateRtspAnalyzer))
        .function("CreateWebrtcAnalyzer", select_overload<void(StreamMatrix &, const std::string &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::CreateWebrtcAnalyzer))

        .function("SetNotification", select_overload<void(StreamMatrix &, const emscripten::val &, const emscripten::val &)>(&binding_utils::SetNotification))
        .function("version", select_overload<void(StreamMatrix &, const emscripten::val &)>(&binding_utils::GetVersion))
        .function("CreateLauncher", select_overload<void(StreamMatrix &, const std::string &, const std::string &, const emscripten::val &)>(&binding_utils::CreateLauncher));
}