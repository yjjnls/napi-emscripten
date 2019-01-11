#include <app/multipoints.hpp>
#include <endpoint/webrtc.hpp>

using json = nlohmann::json;

GST_DEBUG_CATEGORY_STATIC(my_category);
#define GST_CAT_DEFAULT my_category

#define USE_AUTO_SINK false

MultiPoints::MultiPoints(const std::string &id, StreamMatrix *instance)
    : Connector(id, instance)
    , speaker_(nullptr)
    , default_video_src_(nullptr)
    , default_audio_src_(nullptr)
    , default_speaker_(nullptr)
{
    GST_DEBUG_CATEGORY_INIT(my_category, "stream_matrix", 2, "stream_matrix");
}

static GstPadProbeReturn on_monitor_data(GstPad *pad,
                                         GstPadProbeInfo *info,
                                         gpointer rtspclient)
{
    // static int count = 0;
    // RtspClient *rtsp_client = static_cast<RtspClient *>(rtspclient);
    // auto pipeline = rtsp_client->app();

    // printf(".%d", GST_STATE(pipeline->pipeline()));
    printf(".");
    return GST_PAD_PROBE_OK;
}
bool MultiPoints::Initialize(Promise *promise)
{
    const json &j = promise->data();
    if (j.find("video_codec") != j.end()) {
        const std::string video_codec = j["video_codec"];
        if (VideoEncoding().empty())
            VideoEncoding() = lowercase(video_codec);
    }
    if (j.find("audio_codec") != j.end()) {
        const std::string audio_codec = j["audio_codec"];
        if (AudioEncoding().empty())
            AudioEncoding() = lowercase(audio_codec);
    }

    Connector::Initialize(promise);

    // std::string launch = "videotestsrc ! timeoverlay valignment=3 halignment=4 time-mode=2 xpos=0 ypos=0 color=4278190080 font-desc=\"Sans 48\" draw-shadow=false draw-outline=true outline-color=4278190080 ! x264enc ! rtph264pay config-interval=-1 ! rtph264depay name=default_video_src audiotestsrc ! alawenc ! rtppcmapay ! rtppcmadepay name=default_audio_src";  // NOLINT

    // GError *error = nullptr;
    // GstElement *bin = gst_parse_launch(launch.c_str(), &error);
    // if (error) {
    //     g_printerr("Failed to parse launch: %s\n", error->message);
    //     g_error_free(error);
    //     g_clear_object(&bin);
    //     bin = nullptr;
    //     return false;
    // }

    // gst_bin_add(GST_BIN(Pipeline()), bin);

    // default_video_src_ = gst_bin_get_by_name(GST_BIN(bin), "default_video_src");
    // g_warn_if_fail(gst_element_link(default_video_src_, video_selector()));
    // default_audio_src_ = gst_bin_get_by_name(GST_BIN(bin), "default_audio_src");
    // g_warn_if_fail(gst_element_link(default_audio_src_, audio_selector()));


    GstElement *video_test_src = gst_element_factory_make("videotestsrc", nullptr);
    GstElement *video_enc = gst_element_factory_make("x264enc", nullptr);
    GstElement *video_pay = gst_element_factory_make("rtph264pay", nullptr);
    g_object_set(G_OBJECT(video_pay), "config-interval", -1, nullptr);
    default_video_src_ = gst_element_factory_make("rtph264depay", nullptr);

    GstElement *audio_test_src = gst_element_factory_make("audiotestsrc", nullptr);
    GstElement *audio_enc = gst_element_factory_make("alawenc", nullptr);
    GstElement *audio_pay = gst_element_factory_make("rtppcmapay", nullptr);
    default_audio_src_ = gst_element_factory_make("rtppcmadepay", nullptr);

    gst_bin_add_many(GST_BIN(Pipeline()), video_test_src, video_enc, video_pay, default_video_src_, audio_test_src, audio_enc, audio_pay, default_audio_src_, nullptr);
    g_warn_if_fail(gst_element_link_many(video_test_src, video_enc, video_pay, default_video_src_, video_selector(), nullptr));
    g_warn_if_fail(gst_element_link_many(audio_test_src, audio_enc, audio_pay, default_audio_src_, audio_selector(), nullptr));

    // GstElement *queue = gst_element_factory_make("queue", nullptr);
    // GstElement *sink = gst_element_factory_make("fakesink", nullptr);
    // g_object_set(G_OBJECT(sink), "sync", false, nullptr);

    // gst_bin_add_many(GST_BIN(Pipeline()), queue, sink, nullptr);
    // g_warn_if_fail(gst_element_link_many(video_tee(), queue, sink, nullptr));

    // GstPad *testpad = gst_element_get_static_pad(queue, "sink");
    // gst_pad_add_probe(testpad, GST_PAD_PROBE_TYPE_BUFFER, on_monitor_data, nullptr, nullptr);
    // gst_object_unref(testpad);

    // set_speaker_default();

    return true;
}

void MultiPoints::release_sources()
{
    set_speaker_default();
    for (auto ep : members_) {
        ep->Terminate();
        GST_DEBUG("[%s] remove member: %s (type: %s)",
                  uname().c_str(),
                  ep->Id().c_str(),
                  ep->Protocol().c_str());
        delete ep;
    }
    members_.clear();
}
void MultiPoints::Destroy()
{
    Connector::Destroy();
}

void MultiPoints::set_speaker_default()
{
    // video
    // g_warn_if_fail(gst_element_link(default_video_src_, video_selector_));
    GstPad *video_output_src_pad = gst_element_get_static_pad(default_video_src_, "src");
    GstPad *video_selector_sink_pad = gst_pad_get_peer(video_output_src_pad);
    g_object_set(G_OBJECT(video_selector()), "active-pad", video_selector_sink_pad, nullptr);
    gst_object_unref(video_output_src_pad);
    // audio
    // g_warn_if_fail(gst_element_link(default_audio_src_, audio_selector_));
    GstPad *audio_output_src_pad = gst_element_get_static_pad(default_audio_src_, "src");
    GstPad *audio_selector_sink_pad = gst_pad_get_peer(audio_output_src_pad);
    g_object_set(G_OBJECT(audio_selector()), "active-pad", audio_selector_sink_pad, nullptr);
    gst_object_unref(audio_output_src_pad);

    speaker_ = nullptr;
}

void MultiPoints::On(Promise *promise)
{
    const json &j = promise->meta();
    std::string action = j["action"];
    if (action == "add_member") {
        add_member(promise);
    } else if (action == "remove_member") {
        remove_member(promise);
    } else if (action == "set_speaker") {
        set_speaker(promise);
    } else if (action == "startup") {
        startup(promise);
    } else if (action == "stop") {
        stop(promise);
    } else if (action == "remote_sdp") {
        set_remote_description(promise);
    } else if (action == "remote_candidate") {
        set_remote_candidate(promise);
    } else {
        GST_ERROR("[%s] action: %s is not supported!", uname().c_str(), action.c_str());
        promise->reject("action: " + action + " is not supported!");
    }
}

std::list<IEndpoint *>::iterator MultiPoints::find_member(const std::string &id)
{
    return std::find_if(members_.begin(),
                        members_.end(),
                        [&id](IEndpoint *ep) {
                            return ep->Id() == id;
                        });
}

void MultiPoints::add_member(Promise *promise)
{
    // get endpoint protocol
    json &j = promise->data();
    if (j.find("protocol") == j.end()) {
        GST_ERROR("[%s] no protocol in member.", uname().c_str());
        promise->reject("no protocol in member.");
        return;
    }
    const std::string &id = j["endpoint_id"];
    EndpointType protocol = j["protocol"];
    if (find_member(id) != members_.end()) {
        GST_ERROR("[%s] member: %s has been added.", uname().c_str(), id.c_str());
        promise->reject("member: " + id + " has been added.");
        return;
    }
    // create endpoint
    IEndpoint *ep = new Webrtc(this, id);

    bool rc = ep->Initialize(promise);
    if (rc) {
        // add endpoint to pipeline and link with tee
        members_.push_back(ep);
        GST_INFO("[%s] add member: %s (type: %s)", uname().c_str(), id.c_str(), ep->Protocol().c_str());
        promise->resolve();
        return;
    }
    ep->Terminate();
    delete ep;
    ep = nullptr;
    GST_ERROR("[%s] add member: %s failed!", uname().c_str(), id.c_str());
    promise->reject("add member " + id + " failed!");
}
void MultiPoints::remove_member(Promise *promise)
{
    const json &j = promise->data();
    const std::string &id = j["endpoint_id"];
    auto it = find_member(id);
    if (it == members_.end()) {
        GST_ERROR("[%s] member: %s has not been added.", uname().c_str(), id.c_str());
        promise->reject(" member: " + id + " has not been added.");
        return;
    }
    set_speaker_default();

    IEndpoint *ep = *it;
    ep->Terminate();
    members_.erase(it);

    GST_INFO("[%s] remove member: %s (type: %s)", uname().c_str(), id.c_str(), ep->Protocol().c_str());

    delete ep;
    promise->resolve();
}
void MultiPoints::set_speaker(Promise *promise)
{
    if (speaker_ != nullptr) {
        GST_ERROR("[%s] there's already a speaker!", uname().c_str());
        promise->reject("there's already a speaker!");
        return;
    }
    // create endpoint
    const json &j = promise->data();
    const std::string &id = j["endpoint_id"];

    auto it = find_member(id);
    if (it == members_.end()) {
        GST_ERROR("[%s] member: %s has not been added.", uname().c_str(), id.c_str());
        promise->reject("member: " + id + " has not been added.");
        return;
    }
    if (*it != speaker_) {
        Webrtc *ep = static_cast<Webrtc *>(*it);
        // video
        GstElement *video_downstream_joint = ep->VideoInputPipejoint();
        GstPad *video_output_src_pad = gst_element_get_static_pad(video_downstream_joint, "src");
        GstPad *video_selector_sink_pad = gst_pad_get_peer(video_output_src_pad);
        g_assert_nonnull(video_selector_sink_pad);
        g_object_set(G_OBJECT(video_selector()), "active-pad", video_selector_sink_pad, nullptr);

        gst_pad_send_event(video_output_src_pad,
                           gst_event_new_custom(GST_EVENT_CUSTOM_UPSTREAM,
                                                gst_structure_new("GstForceKeyUnit",
                                                                  "all-headers",
                                                                  G_TYPE_BOOLEAN,
                                                                  TRUE,
                                                                  nullptr)));
        gst_object_unref(video_output_src_pad);
        // audio
        GstElement *audio_downstream_joint = ep->AudioInputPipejoint();
        GstPad *audio_output_src_pad = gst_element_get_static_pad(audio_downstream_joint, "src");
        GstPad *audio_selector_sink_pad = gst_pad_get_peer(audio_output_src_pad);
        g_assert_nonnull(audio_selector_sink_pad);
        g_object_set(G_OBJECT(audio_selector()), "active-pad", audio_selector_sink_pad, nullptr);
        gst_object_unref(audio_output_src_pad);

        speaker_ = ep;
    }
    GST_INFO("[%s] set speaker: %s.", uname().c_str(), id.c_str());
    promise->resolve();
    return;
}
void MultiPoints::set_remote_description(Promise *promise)
{
    const json &j = promise->data();

    const std::string &endpoint_id = j["endpoint_id"];
    auto it = find_member(endpoint_id);
    if (it == members_.end()) {
        GST_ERROR("[%s] member: %s has not been added.", uname().c_str(), endpoint_id.c_str());
        promise->reject(" member: " + endpoint_id + " has not been added.");
        return;
    }
    Webrtc *ep = static_cast<Webrtc *>(*it);
    ep->SetRemoteDescription(promise);

    promise->resolve();
}
void MultiPoints::set_remote_candidate(Promise *promise)
{
    const json &j = promise->data();

    const std::string &endpoint_id = j["endpoint_id"];
    auto it = find_member(endpoint_id);
    if (it == members_.end()) {
        GST_ERROR("[%s] member: %s has not been added.", uname().c_str(), endpoint_id.c_str());
        promise->reject("member: " + endpoint_id + " has not been added.");
        return;
    }
    Webrtc *ep = static_cast<Webrtc *>(*it);
    ep->SetRemoteCandidate(promise);

    promise->resolve();
}
