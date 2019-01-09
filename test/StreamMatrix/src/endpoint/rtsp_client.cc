#include <endpoint/rtsp_client.hpp>
#include <framework/app.hpp>

using json = nlohmann::json;

GST_DEBUG_CATEGORY_STATIC(my_category);
#define GST_CAT_DEFAULT my_category

RtspClient::RtspClient(IApp *app, const std::string &id)
    : IEndpoint(app, id)
    , rtspsrc_(nullptr)
    , rtpdepay_video_(nullptr)
    , parse_video_(nullptr)
    , rtpdepay_audio_(nullptr)
    , add_to_pipeline_(false)
{
    GST_DEBUG_CATEGORY_INIT(my_category, "stream_matrix", 2, "stream_matrix");
}


bool RtspClient::Initialize(Promise *promise)
{
    const json &j = promise->data();
    const std::string &url = j["source_url"];
    GST_DEBUG("[%s] {%s} source url: %s", uname().c_str(), app()->uname().c_str(), url.c_str());
    IEndpoint::Protocol() = "rtspclient";

    rtspsrc_ = gst_element_factory_make("rtspsrc", "rtspsrc");
    g_object_set(G_OBJECT(rtspsrc_), "location", url.c_str(), nullptr);
    if (j.find("video_codec") != j.end()) {
        std::string video_codec = j["video_codec"];
        app()->VideoEncoding() = lowercase(std::string(video_codec.c_str()));
    }
    if (j.find("audio_codec") != j.end()) {
        const std::string audio_codec = j["audio_codec"];
        app()->AudioEncoding() = lowercase(std::string(audio_codec.c_str()));
    }

    return add_to_pipeline();
}

void RtspClient::Terminate()
{
    gst_bin_remove_many(GST_BIN(app()->Pipeline()), rtspsrc_, rtpdepay_video_, parse_video_, nullptr);
    gst_bin_remove_many(GST_BIN(app()->Pipeline()), rtpdepay_audio_, nullptr);
    GST_DEBUG("[%s] {%s} terminate done.", uname().c_str(), app()->uname().c_str());
}

bool RtspClient::add_to_pipeline()
{
    if (!add_to_pipeline_) {
        gst_bin_add(GST_BIN(app()->Pipeline()), rtspsrc_);
        g_signal_connect(rtspsrc_, "pad-added", (GCallback)on_rtspsrc_pad_added, this);
        g_signal_connect(rtspsrc_, "select-stream", (GCallback)on_rtspsrc_select_stream, this);
        add_to_pipeline_ = true;
    }

    if (!app()->VideoEncoding().empty()) {
        VideoEncodingType video_codec = get_video_encoding_type(app()->VideoEncoding());
        switch (video_codec) {
            case VideoEncodingType::H264:
                rtpdepay_video_ = gst_element_factory_make("rtph264depay", "depay");
                parse_video_ = gst_element_factory_make("h264parse", "parse");
                break;
            case VideoEncodingType::H265:
                rtpdepay_video_ = gst_element_factory_make("rtph265depay", "depay");
                parse_video_ = gst_element_factory_make("h265parse", "parse");
                break;
            default:
                GST_WARNING("[%s] {%s} invalid Video Codec: %s!",
                            uname().c_str(),
                            app()->uname().c_str(),
                            app()->VideoEncoding().c_str());
                return false;
        }
        g_warn_if_fail(rtpdepay_video_ && parse_video_);

        gst_bin_add_many(GST_BIN(app()->Pipeline()), rtpdepay_video_, parse_video_, nullptr);
        g_warn_if_fail(gst_element_link(rtpdepay_video_, parse_video_));
        GST_DEBUG("[%s] {%s} configured video: %s",
                  uname().c_str(),
                  app()->uname().c_str(),
                  app()->VideoEncoding().c_str());

        // g_signal_connect(rtspsrc_, "new-manager", (GCallback)on_get_new_rtpbin, this);
    }
    if (!app()->AudioEncoding().empty()) {
        AudioEncodingType audio_codec = get_audio_encoding_type(app()->AudioEncoding());
        switch (audio_codec) {
            case AudioEncodingType::PCMA:
                rtpdepay_audio_ = gst_element_factory_make("rtppcmadepay", "audio-depay");
                break;
            case AudioEncodingType::PCMU:
                rtpdepay_audio_ = gst_element_factory_make("rtppcmudepay", "audio-depay");
                break;
            case AudioEncodingType::OPUS:
                rtpdepay_audio_ = gst_element_factory_make("rtpopusdepay", "audio-depay");
                break;
            default:
                GST_WARNING("[%s] {%s} invalid Audio Codec: %s!",
                            uname().c_str(),
                            app()->uname().c_str(),
                            app()->VideoEncoding().c_str());
                return false;
        }

        g_warn_if_fail(rtpdepay_audio_);
        gst_bin_add_many(GST_BIN(app()->Pipeline()), rtpdepay_audio_, nullptr);
        GST_DEBUG("[%s] {%s} configured audio: %s",
                  uname().c_str(),
                  app()->uname().c_str(),
                  app()->AudioEncoding().c_str());
    }
    GST_DEBUG("[%s] {%s} initialize done.", uname().c_str(), app()->uname().c_str());

    return true;
}

GstPadProbeReturn RtspClient::on_monitor_data(GstPad *pad,
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
void RtspClient::on_rtspsrc_pad_added(GstElement *src,
                                      GstPad *src_pad,
                                      gpointer rtspclient)
{
    RtspClient *rtsp_client = static_cast<RtspClient *>(rtspclient);
    GstCaps *caps = gst_pad_query_caps(src_pad, nullptr);
    GstStructure *stru = gst_caps_get_structure(caps, 0);
    const GValue *media_type = gst_structure_get_value(stru, "media");

    if (g_str_equal(g_value_get_string(media_type), "video")) {
        if (!rtsp_client->app()->VideoEncoding().empty()) {
            GstPad *sink_pad = gst_element_get_static_pad(GST_ELEMENT_CAST(rtsp_client->rtpdepay_video_), "sink");
            GstPadLinkReturn ret = gst_pad_link(src_pad, sink_pad);
            g_warn_if_fail(ret == GST_PAD_LINK_OK);
            gst_object_unref(sink_pad);

            json meta;
            meta["topic"] = "rtspclient@" + rtsp_client->Id();
            meta["origin"] = rtsp_client->app()->uname();
            json data;
            data["msg"] = "video channel connected";
            rtsp_client->app()->Notify(meta, data);
            // gst_pad_add_probe(src_pad, GST_PAD_PROBE_TYPE_BUFFER, on_monitor_data, rtspclient, nullptr);
        }
    } else if (g_str_equal(g_value_get_string(media_type), "audio")) {
        if (!rtsp_client->app()->AudioEncoding().empty()) {
            GstPad *sink_pad = gst_element_get_static_pad(GST_ELEMENT_CAST(rtsp_client->rtpdepay_audio_), "sink");
            GstPadLinkReturn ret = gst_pad_link(src_pad, sink_pad);
            g_warn_if_fail(ret == GST_PAD_LINK_OK);
            gst_object_unref(sink_pad);

            json meta;
            meta["topic"] = "rtspclient@" + rtsp_client->Id();
            meta["origin"] = rtsp_client->app()->uname();
            json data;
            data["msg"] = "audio channel connected";
            rtsp_client->app()->Notify(meta, data);
            // gst_pad_add_probe(src_pad, GST_PAD_PROBE_TYPE_BUFFER, on_monitor_data, rtspclient, nullptr);
        }
    } else {
        g_warn_if_reached();
    }
}
gboolean RtspClient::on_rtspsrc_select_stream(GstElement *src,
                                              guint stream_id,
                                              GstCaps *stream_caps,
                                              gpointer rtspclient)
{
    RtspClient *rtsp_client = static_cast<RtspClient *>(rtspclient);
    GstStructure *stru = gst_caps_get_structure(stream_caps, 0);
    std::string media_type(gst_structure_get_string(stru, "media"));
    if (media_type == "video") {
        if (!rtsp_client->app()->VideoEncoding().empty())
            return TRUE;
    }
    if (media_type == "audio") {
        if (!rtsp_client->app()->AudioEncoding().empty())
            return TRUE;
    }
    // if (gst_structure_has_field(stru, "a-recvonly")) {
    //     // back channel
    // }
    return FALSE;
}