#include <endpoint/webrtc.hpp>
#include <framework/app.hpp>
#include <gst/sdp/sdp.h>
#include <gst/webrtc/webrtc.h>

using json = nlohmann::json;

GST_DEBUG_CATEGORY_STATIC(my_category);
#define GST_CAT_DEFAULT my_category

int Webrtc::session_count_ = 0;

Webrtc::Webrtc(IApp *app, const std::string &id)
    : IEndpoint(app, id)
    , pipeline_(nullptr)
    , bin_(nullptr)
    , webrtc_(nullptr)
    , role_(kOffer)
{
    GST_DEBUG_CATEGORY_INIT(my_category, "stream_matrix", 2, "stream_matrix");
}


bool Webrtc::Initialize(Promise *promise)
{
    IEndpoint::Protocol() = "webrtc";
    const json &j = promise->data();
    if (j.find("role") != j.end()) {
        role_ = j["role"];
    }
    if (role_ != WebrtcRole::kOffer && role_ != WebrtcRole::kAnswer) {
        GST_ERROR("[%s] %p initialize failed, invalid role: %s.", uname().c_str(), webrtc_, role_ ? "answer" : "offer");
        return false;
    }
    if (j.find("launch") != j.end()) {
        launch_ = j["launch"];
    }
    // launch_ = "webrtcbin name=webrtc videotestsrc pattern=white ! timeoverlay valignment=3 halignment=4 time-mode=2 xpos=0 ypos=0 color=4278190080 font-desc=\"Sans 48\" draw-shadow=false draw-outline=true outline-color=4278190080 ! queue ! x264enc bitrate=512 key-int-max=5 ! rtph264pay name=pay0 ! queue ! application/x-rtp,media=video,encoding-name=H264,payload=96 ! webrtc. audiotestsrc ! alawenc ! rtppcmapay name=pay1 ! queue ! application/x-rtp,media=audio,encoding-name=PCMA,payload=8 ! webrtc.";

    if (!parse_launch()) {
        return false;
    }

    // gboolean sync = TRUE;
    // g_object_get(G_OBJECT(webrtc_), "sink-false", &sync, nullptr);
    // if (!sync) {
    //     GST_FIXME("[Webrtc] %p (%s) uses the fixed plugin.", webrtc_, role_ ? "answer" : "offer");
    // } else {
    //     GST_FIXME("[Webrtc] %p (%s) uses the origin plugin.", webrtc_, role_ ? "answer" : "offer");
    // }
    // g_object_set(G_OBJECT(webrtc_), "sink-false", TRUE, nullptr);

    // specific parameter
    if (app()->VideoEncoding() == "h264") {
        GstElement *payloader = gst_bin_get_by_name(GST_BIN(pipeline_), "pay0");
        g_object_set(G_OBJECT(payloader), "config-interval", -1, nullptr);
        gst_object_unref(payloader);
    }
    // if (role_ == kAnswer) {
    //     printf("====~~~~~~~~~~~~~====\n");
    //     GstElement *test_element = gst_bin_get_by_name_recurse_up(GST_BIN(pipeline_), "sink");
    //     GstPad *testpad = gst_element_get_static_pad(test_element, "sink");
    //     gst_pad_add_probe(testpad, GST_PAD_PROBE_TYPE_BUFFER, on_monitor_data, nullptr, nullptr);
    //     gst_object_unref(testpad);
    // }

    // VideoEncoding() and AudioEncoding() of analyzer will be empty.
    if (!app()->VideoEncoding().empty()) {
        GST_DEBUG("[%s] (%p %s) media constructed: video", uname().c_str(), webrtc_, role_ ? "answer" : "offer");

        static std::string video_media_type = "video";
        std::string video_output_pipejoint_name = std::string("webrtc_video_output_joint_") +
                                                  Id() +
                                                  std::to_string(session_count_);
        video_output_joint() = make_pipe_joint(video_media_type, video_output_pipejoint_name);
        g_warn_if_fail(gst_bin_add(GST_BIN(pipeline_), video_output_joint().downstream_joint));
        GstElement *video_pay = gst_bin_get_by_name_recurse_up(GST_BIN(pipeline_), "pay0");
        g_warn_if_fail(gst_element_link(video_output_joint().downstream_joint, video_pay));

        // GstPad *testpad = gst_element_get_static_pad(video_output_joint().upstream_joint, "sink");
        // gst_pad_add_probe(testpad, GST_PAD_PROBE_TYPE_BUFFER, on_monitor_data, nullptr, nullptr);
        // gst_object_unref(testpad);

        std::string video_input_pipejoint_name = std::string("webrtc_video_input_joint_") +
                                                 Id() +
                                                 std::to_string(session_count_);
        video_input_joint() = make_pipe_joint(video_media_type, video_input_pipejoint_name);
        app()->AddPipeJoint(video_output_joint().upstream_joint,
                            video_input_joint().downstream_joint);
    }
    if (!app()->AudioEncoding().empty()) {
        GST_DEBUG("[%s] (%p %s) media constructed: audio", uname().c_str(), webrtc_, role_ ? "answer" : "offer");

        static std::string audio_media_type = "audio";
        std::string audio_output_pipejoint_name = std::string("webrtc_audio_endpoint_joint_") +
                                                  Id() +
                                                  std::to_string(session_count_);
        audio_output_joint() = make_pipe_joint(audio_media_type, audio_output_pipejoint_name);
        g_warn_if_fail(gst_bin_add(GST_BIN(pipeline_), audio_output_joint().downstream_joint));
        GstElement *audio_pay = gst_bin_get_by_name_recurse_up(GST_BIN(pipeline_), "pay1");
        g_warn_if_fail(gst_element_link(audio_output_joint().downstream_joint, audio_pay));

        std::string audio_input_pipejoint_name = std::string("webrtc_audio_input_joint_") +
                                                 Id() +
                                                 std::to_string(session_count_);
        audio_input_joint() = make_pipe_joint(audio_media_type, audio_input_pipejoint_name);
        app()->AddPipeJoint(audio_output_joint().upstream_joint,
                            audio_input_joint().downstream_joint);
    }
    session_count_++;


    g_signal_connect(webrtc_, "on-ice-candidate", G_CALLBACK(Webrtc::on_ice_candidate), this);

    if (role_ == kOffer) {
        g_signal_connect(webrtc_, "on-negotiation-needed", G_CALLBACK(Webrtc::on_negotiation_needed), this);
    }
    // used as test
    // if (j.find("launch") == j.end()) {
    //     g_signal_connect(webrtc_, "pad-added", G_CALLBACK(Webrtc::on_webrtc_pad_added), this);
    // }

    GstStateChangeReturn ret = gst_element_set_state(pipeline_, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        GST_DEBUG("[%s] (%p %s) initialize failed.", uname().c_str(), webrtc_, role_ ? "answer" : "offer");
    }
    GST_DEBUG("[%s] (%p %s) initialize done.", uname().c_str(), webrtc_, role_ ? "answer" : "offer");

    return true;
}
bool Webrtc::parse_launch()
{
    if (launch_.empty()) {
        launch_ = "webrtcbin name=webrtc ";
        if (!app()->VideoEncoding().empty()) {
            std::string video_enc = app()->VideoEncoding();
            // launch += "rtspsrc location=rtsp://172.16.66.65/id=1 ! rtph264depay ! queue ! ";
            launch_ += "rtp" + video_enc + "pay name=pay0 ! queue ! " +
                       "application/x-rtp,media=video,encoding-name=" + uppercase(video_enc) +
                       ",payload=96 ! webrtc. ";
        }
        if (!app()->AudioEncoding().empty()) {
            std::string audio_enc = app()->AudioEncoding();
            launch_ += "rtp" + audio_enc + "pay name=pay1 ! queue ! " +
                       "application/x-rtp,media=audio,encoding-name=" + uppercase(audio_enc);
            if (audio_enc == "pcma") {
                launch_ += ",payload=8 ! webrtc. ";
            } else {
                launch_ += ",payload=97 ! webrtc. ";
            }
        }
    }
    printf("===============>  %s\n", launch_.c_str());
    GError *error = nullptr;
    bin_ = gst_parse_launch(launch_.c_str(), &error);

    if (error) {
        GST_ERROR("[%s] Failed to parse launch: %s", uname().c_str(), error->message);
        g_error_free(error);
        g_clear_object(&bin_);
        bin_ = nullptr;
        return false;
    }
    pipeline_ = gst_pipeline_new(nullptr);
    gst_bin_add(GST_BIN(pipeline_), bin_);
    g_assert_nonnull(pipeline_);
    webrtc_ = gst_bin_get_by_name(GST_BIN(pipeline_), "webrtc");
    return true;
}
void Webrtc::on_ice_candidate(GstElement *webrtc_element G_GNUC_UNUSED,
                              guint mlineindex,
                              gchar *candidate,
                              gpointer user_data G_GNUC_UNUSED)
{
    Webrtc *webrtc = static_cast<Webrtc *>(user_data);
    json data;
    data["candidate"] = candidate;
    data["sdpMLineIndex"] = mlineindex;

    std::string ice_candidate = data.dump();

    // printf("\nice:%s\n", ice_candidate.c_str());

    json meta;
    meta["topic"] = "webrtc";
    meta["origin"] = webrtc->app()->uname();
    meta["type"] = "ice";
    meta["id"] = webrtc->Id();

    GST_LOG("[%s] (%p %s) local candidate created.", webrtc->uname().c_str(), webrtc->webrtc_, webrtc->role_ ? "answer" : "offer");

    webrtc->app()->Notify(meta, data);
}
void Webrtc::on_sdp_created(GstPromise *promise, gpointer user_data)
{
    Webrtc *webrtc = static_cast<Webrtc *>(user_data);
    GstWebRTCSessionDescription *sdp = nullptr;
    const GstStructure *reply = gst_promise_get_reply(promise);
    gst_structure_get(reply, webrtc->role_ ? "answer" : "offer", GST_TYPE_WEBRTC_SESSION_DESCRIPTION, &sdp, nullptr);
    gst_promise_unref(promise);

    // TODO(yuanjunjie): only add in H264, if `m=audio` is firstly meet in sdp, it occurred error
    // gst_sdp_media_add_attribute((GstSDPMedia *)&g_array_index(sdp->sdp->medias, GstSDPMedia, 0),
    //                             "fmtp",
    //                             "96 profile-level-id=42e01f");
    for (int i = 0; i < sdp->sdp->medias->len; ++i) {
        GstSDPMedia *sdp_media = (GstSDPMedia *)&g_array_index(sdp->sdp->medias, GstSDPMedia, i);
        if (g_strcmp0(sdp_media->media, "video") == 0) {
            for (int j = 0; j < sdp_media->attributes->len; ++j) {
                GstSDPAttribute *arrtibute = (GstSDPAttribute *)&g_array_index(sdp_media->attributes,
                                                                               GstSDPAttribute,
                                                                               j);
                std::string target = "H264/90000";
                if (g_strcmp0(arrtibute->key, "rtpmap") == 0 &&
                    g_str_has_suffix(arrtibute->value, target.c_str())) {
                    std::string str(arrtibute->value);
                    std::size_t pos = str.find(target);
                    std::string prop = str.substr(0, pos);
                    prop += std::string("profile-level-id=42e01f");

                    gst_sdp_media_add_attribute(sdp_media,
                                                "fmtp",
                                                prop.c_str());
                }
            }
        }
    }

    /* Send sdp to peer */
    std::string sdp_offer(gst_sdp_message_as_text(sdp->sdp));
    json data;
    data["type"] = webrtc->role_ ? "answer" : "offer";
    data["sdp"] = sdp_offer;
    json meta;
    meta["topic"] = "webrtc";
    meta["origin"] = webrtc->app()->uname();
    meta["type"] = "sdp";
    meta["id"] = webrtc->Id();

    g_signal_emit_by_name(webrtc->webrtc_, "set-local-description", sdp, nullptr);
    GST_INFO("[%s] (%p %s) local description created.", webrtc->uname().c_str(), webrtc->webrtc_, webrtc->role_ ? "answer" : "offer");

    webrtc->app()->Notify(meta, data);

    gst_webrtc_session_description_free(sdp);
}
void Webrtc::on_negotiation_needed(GstElement *element, gpointer user_data)
{
    Webrtc *webrtc = static_cast<Webrtc *>(user_data);
    GstPromise *promise = gst_promise_new_with_change_func(on_sdp_created, user_data, nullptr);
    g_signal_emit_by_name(webrtc->webrtc_, "create-offer", nullptr, promise);
}
static GstPadProbeReturn cb_have_data(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    printf("-");
    return GST_PAD_PROBE_OK;
}
void Webrtc::on_webrtc_pad_added(GstElement *webrtc_element, GstPad *new_pad, gpointer user_data)
{
    // this function is just for test, it could only show h264 video.
    Webrtc *webrtc = static_cast<Webrtc *>(user_data);
    printf("=========on_webrtc_pad_added %s===========\n", webrtc->role_ ? "answer" : "offer");

    GstElement *out = nullptr;
    GstPad *sink = nullptr;
    GstCaps *caps;
    GstStructure *s;
    const gchar *encoding_name;

    if (GST_PAD_DIRECTION(new_pad) != GST_PAD_SRC)
        return;

    caps = gst_pad_get_current_caps(new_pad);
    if (!caps)
        caps = gst_pad_query_caps(new_pad, nullptr);
    // GST_ERROR_OBJECT(new_pad, "caps %" GST_PTR_FORMAT, caps);
    // g_assert(gst_caps_is_fixed(caps));
    s = gst_caps_get_structure(caps, 0);
    encoding_name = gst_structure_get_string(s, "encoding-name");

    if (g_strcmp0(encoding_name, "VP8") == 0) {
        GST_INFO("[%s] (%p %s) receive video: VP8.", webrtc->uname().c_str(), webrtc, webrtc->role_ ? "answer" : "offer");
        out = gst_parse_bin_from_description(
            // "rtpvp8depay ! tee name=local_tee allow-not-linked=true ! queue ! vp8dec ! videoconvert ! ximagesink sync=false",
            "rtpvp8depay ! tee name=local_tee allow-not-linked=true",
            TRUE,
            nullptr);

        g_warn_if_fail(gst_bin_add(GST_BIN(out), webrtc->video_input_joint().upstream_joint));
        GstElement *local_tee = gst_bin_get_by_name_recurse_up(GST_BIN(out), "local_tee");
        g_warn_if_fail(gst_element_link(local_tee, webrtc->video_input_joint().upstream_joint));
        gst_object_unref(local_tee);

        // webrtc->app()->AddPipeJoint(nullptr, webrtc->video_input_joint().downstream_joint);

    } else if (g_strcmp0(encoding_name, "H264") == 0) {
        GST_INFO("[%s] (%p %s) receive video: H264.", webrtc->uname().c_str(), webrtc, webrtc->role_ ? "answer" : "offer");
        out = gst_parse_bin_from_description(
            "rtph264depay ! tee name=local_tee allow-not-linked=true",
            TRUE,
            nullptr);

        g_warn_if_fail(gst_bin_add(GST_BIN(out), webrtc->video_input_joint().upstream_joint));
        GstElement *local_tee = gst_bin_get_by_name_recurse_up(GST_BIN(out), "local_tee");
        g_warn_if_fail(gst_element_link(local_tee, webrtc->video_input_joint().upstream_joint));
        gst_object_unref(local_tee);
        // webrtc->app()->AddPipeJoint(nullptr, webrtc->video_input_joint().downstream_joint);

    } else if (g_strcmp0(encoding_name, "PCMA") == 0) {
        GST_INFO("[%s] (%p %s) receive audio: PCMA.", webrtc->uname().c_str(), webrtc, webrtc->role_ ? "answer" : "offer");
        out = gst_parse_bin_from_description(
            // "rtppcmadepay ! tee name=local_audio_tee allow-not-linked=true ! queue ! alawdec ! audioconvert ! spectrascope shader=3 ! ximagesink sync=false",
            "rtppcmadepay ! tee name=local_audio_tee allow-not-linked=true",
            TRUE,
            nullptr);

        g_warn_if_fail(gst_bin_add(GST_BIN(out), webrtc->audio_input_joint().upstream_joint));
        GstElement *local_tee = gst_bin_get_by_name_recurse_up(GST_BIN(out), "local_audio_tee");
        g_warn_if_fail(gst_element_link(local_tee, webrtc->audio_input_joint().upstream_joint));

        // GstPad *pad = gst_element_get_static_pad(webrtc->audio_input_joint().upstream_joint, "sink");
        // gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, on_monitor_data, nullptr, nullptr);
        // gst_object_unref(pad);

        gst_object_unref(local_tee);
        // webrtc->app()->AddPipeJoint(nullptr, webrtc->audio_input_joint().downstream_joint);
    } else if (g_strcmp0(encoding_name, "OPUS") == 0) {
        GST_INFO("[%s] (%p %s) receive video: OPUS.", webrtc->uname().c_str(), webrtc, webrtc->role_ ? "answer" : "offer");
        out = gst_parse_bin_from_description(
            // "rtppcmadepay ! tee name=local_audio_tee allow-not-linked=true ! queue ! alawdec ! audioconvert ! spectrascope shader=3 ! ximagesink sync=false",
            "rtpopusdepay ! tee name=local_audio_tee allow-not-linked=true",
            TRUE,
            nullptr);

        g_warn_if_fail(gst_bin_add(GST_BIN(out), webrtc->audio_input_joint().upstream_joint));
        GstElement *local_tee = gst_bin_get_by_name_recurse_up(GST_BIN(out), "local_audio_tee");
        // g_warn_if_fail(gst_element_link(local_tee, webrtc->audio_input_joint().upstream_joint));
    } else {
        g_critical("Unknown encoding name %s", encoding_name);
        g_assert_not_reached();
        gst_caps_unref(caps);
        return;
    }
    gst_bin_add(GST_BIN(webrtc->bin_), out);
    gst_element_sync_state_with_parent(out);
    sink = (GstPad *)out->sinkpads->data;

    gst_pad_link(new_pad, sink);

    gst_caps_unref(caps);
}


void Webrtc::SetRemoteDescription(Promise *promise)
{
    const json &j = promise->data();
    std::string sdp_info = j["sdp"];
    std::string type = j["type"];
    // printf("\n%s\n", sdp_info.c_str());

    GstWebRTCSessionDescription *sdp;
    GstSDPMessage *sdp_msg;
    gst_sdp_message_new(&sdp_msg);
    gst_sdp_message_parse_buffer((guint8 *)sdp_info.c_str(), (guint)sdp_info.size(), sdp_msg);
    sdp = gst_webrtc_session_description_new(GST_WEBRTC_SDP_TYPE_ANSWER, sdp_msg);
    if (type == "offer")
        sdp->type = GST_WEBRTC_SDP_TYPE_OFFER;
    else
        sdp->type = GST_WEBRTC_SDP_TYPE_ANSWER;
    g_signal_emit_by_name(webrtc_, "set-remote-description", sdp, nullptr);
    GST_INFO("[%s] (%p %s) set remote description.", uname().c_str(), webrtc_, role_ ? "answer" : "offer");

    if (role_ == kAnswer) {
        GstPromise *promise = gst_promise_new_with_change_func(Webrtc::on_sdp_created, this, nullptr);
        g_signal_emit_by_name(webrtc_, "create-answer", nullptr, promise);
    }
}
void Webrtc::SetRemoteCandidate(Promise *promise)
{
    const json &j = promise->data();
    std::string candidate = j["candidate"];
    int sdpmlineindex = j["sdpMLineIndex"];
    g_signal_emit_by_name(webrtc_, "add-ice-candidate", sdpmlineindex, candidate.c_str());
    GST_LOG("[%s] (%p %s) set remote candidate.", uname().c_str(), webrtc_, role_ ? "answer" : "offer");
}


GstPadProbeReturn Webrtc::on_monitor_data(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    if (!GST_BUFFER_FLAG_IS_SET(info->data, GST_BUFFER_FLAG_DELTA_UNIT)) {
        static int cnt = 0;
        GDateTime *date_time = g_date_time_new_now_local();
        gchar *s_date_time = g_date_time_format(date_time, "%H:%M:%S,%F");
        g_warning("Received keyframe(%u) @ (%s)", cnt++, s_date_time);
        g_free(s_date_time);
        g_date_time_unref(date_time);
    }
    g_print(".");
    return GST_PAD_PROBE_OK;
}


void Webrtc::Terminate()
{
    // dynamicly unlink
    if (!app()->VideoEncoding().empty() &&
        video_output_joint().upstream_joint != nullptr) {
        app()->RemovePipeJoint(video_output_joint().upstream_joint, video_input_joint().downstream_joint);
    }
    if (!app()->AudioEncoding().empty() &&
        audio_output_joint().upstream_joint != nullptr) {
        app()->RemovePipeJoint(audio_output_joint().upstream_joint, audio_input_joint().downstream_joint);
    }
    if (pipeline_) {
        gst_element_set_state(GST_ELEMENT(pipeline_), GST_STATE_NULL);
        gst_bin_remove(GST_BIN(pipeline_), bin_);
        gst_object_unref(pipeline_);
    }
    GST_DEBUG("[%s] (%p %s) terminate done.", uname().c_str(), webrtc_, role_ ? "answer" : "offer");
    if (webrtc_) {
        gst_object_unref(webrtc_);
        webrtc_ = nullptr;
    }
}