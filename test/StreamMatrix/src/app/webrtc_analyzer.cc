#include <app/webrtc_analyzer.hpp>

using json = nlohmann::json;

GST_DEBUG_CATEGORY_STATIC(my_category);
#define GST_CAT_DEFAULT my_category


WebrtcAnalyzer::WebrtcAnalyzer(const std::string &id,
                               StreamMatrix *instance)
    : Analyzer(id, instance)
    , webrtc_ep_(nullptr)
{
    GST_DEBUG_CATEGORY_INIT(my_category, "stream_matrix", 2, "stream_matrix");
}

WebrtcAnalyzer::~WebrtcAnalyzer()
{
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
bool WebrtcAnalyzer::Initialize(Promise *promise)
{
    if (webrtc_ep_ == nullptr) {
        webrtc_ep_ = new Webrtc(this, "analyzer");
    }
    if (!webrtc_ep_->Initialize(promise))
        return false;

    // GstElement *video_test_src = gst_element_factory_make("videotestsrc", nullptr);
    // GstElement *video_enc = gst_element_factory_make("x264enc", nullptr);
    // GstElement *video_pay = gst_element_factory_make("rtph264pay", nullptr);
    // g_object_set(G_OBJECT(video_pay), "config-interval", -1, nullptr);
    // GstElement *video_depay = gst_element_factory_make("rtph264depay", nullptr);

    // GstElement *audio_test_src = gst_element_factory_make("audiotestsrc", nullptr);
    // GstElement *audio_enc = gst_element_factory_make("alawenc", nullptr);
    // GstElement *audio_pay = gst_element_factory_make("rtppcmapay", nullptr);
    // GstElement *audio_depay = gst_element_factory_make("rtppcmadepay", nullptr);

    // GstElement *pipeline_ = gst_pipeline_new(nullptr);
    // gst_bin_add_many(GST_BIN(pipeline_), video_test_src, video_enc, video_pay, video_depay, audio_test_src, audio_enc, audio_pay, audio_depay, nullptr);
    // g_warn_if_fail(gst_element_link_many(video_test_src, video_enc, video_pay, video_depay, nullptr));
    // g_warn_if_fail(gst_element_link_many(audio_test_src, audio_enc, audio_pay, audio_depay, nullptr));
    // gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    // //////////////////////////////////////////
    // GstElement *pay0 = gst_bin_get_by_name_recurse_up(GST_BIN(webrtc_ep_->Pipeline()), "pay0");
    // GstElement *pay1 = gst_bin_get_by_name_recurse_up(GST_BIN(webrtc_ep_->Pipeline()), "pay1");
    // //////////////////////////////////////////

    // int session_count_ = 0;
    // static std::string video_media_type = "video";
    // std::string video_output_pipejoint_name = std::string("webrtc_analyzer_video_output_joint_");
    // PipeJoint video_joint = make_pipe_joint(video_media_type, video_output_pipejoint_name);
    // g_warn_if_fail(gst_bin_add(GST_BIN(webrtc_ep_->Pipeline()), video_joint.downstream_joint));
    // g_warn_if_fail(gst_bin_add(GST_BIN(pipeline_), video_joint.upstream_joint));
    // g_warn_if_fail(gst_element_link(video_joint.downstream_joint, pay0));
    // g_warn_if_fail(gst_element_link(video_depay, video_joint.upstream_joint));
    // gst_element_sync_state_with_parent(video_joint.upstream_joint);

    // static std::string audio_media_type = "audio";
    // std::string audio_output_pipejoint_name = std::string("webrtc_analyzer_audio_output_joint_");
    // PipeJoint audio_joint = make_pipe_joint(audio_media_type, audio_output_pipejoint_name);
    // g_warn_if_fail(gst_bin_add(GST_BIN(webrtc_ep_->Pipeline()), audio_joint.downstream_joint));
    // g_warn_if_fail(gst_bin_add(GST_BIN(pipeline_), audio_joint.upstream_joint));
    // g_warn_if_fail(gst_element_link(audio_joint.downstream_joint, pay1));
    // g_warn_if_fail(gst_element_link(audio_depay, audio_joint.upstream_joint));
    // gst_element_sync_state_with_parent(audio_joint.upstream_joint);


    // GstPad *testpad = gst_element_get_static_pad(pay0, "sink");
    // gst_pad_add_probe(testpad, GST_PAD_PROBE_TYPE_BUFFER, on_monitor_data, nullptr, nullptr);
    // gst_object_unref(testpad);
    return true;
}
void WebrtcAnalyzer::Destroy()
{
    if (webrtc_ep_ != nullptr) {
        webrtc_ep_->Terminate();
        delete webrtc_ep_;
        webrtc_ep_ = nullptr;
    }
}
void WebrtcAnalyzer::On(Promise *promise)
{
    const json &j = promise->meta();
    std::string action = j["action"];
    if (action == "startup") {
        startup(promise);
    } else if (action == "stop") {
        stop(promise);
    } else if (action == "remote_sdp") {
        set_remote_description(promise);
    } else if (action == "remote_candidate") {
        set_remote_candidate(promise);
    }
}

void WebrtcAnalyzer::startup(Promise *promise)
{
    Analyzer::startup(promise);
}

void WebrtcAnalyzer::stop(Promise *promise)
{
    promise->resolve();
}
void WebrtcAnalyzer::set_remote_description(Promise *promise)
{
    if (webrtc_ep_) {
        webrtc_ep_->SetRemoteDescription(promise);
        promise->resolve();
        return;
    }
    promise->reject("WebrtcAnalyzer set_remote_description failed!");
}
void WebrtcAnalyzer::set_remote_candidate(Promise *promise)
{
    if (webrtc_ep_) {
        webrtc_ep_->SetRemoteCandidate(promise);
        promise->resolve();
        return;
    }
    promise->reject("WebrtcAnalyzer set_remote_candidate failed!");
}