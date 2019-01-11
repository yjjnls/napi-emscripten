#include <app/livestream.hpp>
#include <endpoint/rtsp_client.hpp>
#include <endpoint/rtsp_server.hpp>
#include <endpoint/webrtc.hpp>

using json = nlohmann::json;

GST_DEBUG_CATEGORY_STATIC(my_category);
#define GST_CAT_DEFAULT my_category

#define USE_AUTO_SINK false

LiveStream::LiveStream(const std::string &id, StreamMatrix *instance)
    : Connector(id, instance)
    , performer_(nullptr)
    , fake_video_queue_(nullptr)
    , fake_video_sink_(nullptr)
    , fake_audio_queue_(nullptr)
    , fake_audio_sink_(nullptr)
    , fake_video_decodec_(nullptr)
    , fake_audio_decodec_(nullptr)
    , fake_audio_convert_(nullptr)
    , fake_audio_resample_(nullptr)
{
    GST_DEBUG_CATEGORY_INIT(my_category, "stream_matrix", 2, "stream_matrix");
}


bool LiveStream::Initialize(Promise *promise)
{
    if (performer_ != nullptr) {
        GST_ERROR("[%s] there's already a performer!", uname().c_str());
        promise->reject("there's already a performer!");
        return false;
    }
    Connector::Initialize(promise);

    const json &j = promise->data();
    const std::string &id = j["id"];

    // create endpoint
    performer_ = new RtspClient(this, "default_performer");
    // initialize endpoint and add it to the pipeline
    bool rc = performer_->Initialize(promise);
    if (rc) {
        // link endpoint to video/audio tee
        if (on_add_endpoint(performer_)) {
            GST_DEBUG("[%s] add performer (type: %s)", uname().c_str(), performer_->Protocol().c_str());
            promise->resolve();
            return true;
        }
    }
    performer_->Terminate();
    delete performer_;
    performer_ = nullptr;
    GST_ERROR("[%s] add performer failed!", uname().c_str());
    promise->reject(id + " add performer failed!");
    return false;
}

bool LiveStream::on_add_endpoint(IEndpoint *endpoint)
{
    // default: rtsp client
    if (!VideoEncoding().empty()) {
        GstElement *parse = gst_bin_get_by_name_recurse_up(GST_BIN(Pipeline()), "parse");
        g_warn_if_fail(parse);

        g_warn_if_fail(gst_element_link(parse, video_selector()));
        fake_video_queue_ = gst_element_factory_make("queue", "fake_video_queue");

        if (USE_AUTO_SINK) {
            fake_video_decodec_ = gst_element_factory_make("avdec_h264", "fake_video_decodec");
            fake_video_sink_ = gst_element_factory_make("autovideosink", "fake_video_sink");
            g_object_set(fake_video_sink_, "sync", FALSE, nullptr);
            gst_bin_add_many(GST_BIN(Pipeline()), fake_video_decodec_, fake_video_queue_, fake_video_sink_, nullptr);
            gst_element_link_many(fake_video_queue_, fake_video_decodec_, fake_video_sink_, nullptr);
        } else {
            fake_video_sink_ = gst_element_factory_make("fakesink", "fake_video_sink");
            g_object_set(fake_video_sink_, "sync", FALSE, nullptr);
            gst_bin_add_many(GST_BIN(Pipeline()), fake_video_queue_, fake_video_sink_, nullptr);
            gst_element_link_many(fake_video_queue_, fake_video_sink_, nullptr);
        }


        g_warn_if_fail(gst_element_link(video_tee(), fake_video_queue_));
        // GstPadTemplate *templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(video_tee()), "src_%u");
        // video_tee_pad_ = gst_element_request_pad(video_tee(), templ, nullptr, nullptr);
        // GstPad *sinkpad = gst_element_get_static_pad(fake_video_queue_, "sink");
        // g_warn_if_fail(gst_pad_link(video_tee_pad_, sinkpad) == GST_PAD_LINK_OK);
        // gst_object_unref(sinkpad);

        // monitor data probe
        GstPad *pad = gst_element_get_static_pad(fake_video_queue_, "sink");
        gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, LiveStream::on_monitor_video_data, this, nullptr);
        gst_object_unref(pad);
    }
    if (!AudioEncoding().empty()) {
        GstElement *audio_depay = gst_bin_get_by_name_recurse_up(GST_BIN(Pipeline()), "audio-depay");
        g_warn_if_fail(audio_depay);

        g_warn_if_fail(gst_element_link(audio_depay, audio_selector()));
        fake_audio_queue_ = gst_element_factory_make("queue", "fake_audio_queue");

        if (USE_AUTO_SINK) {
            fake_audio_decodec_ = gst_element_factory_make("alawdec", "fake_audio_decodec");
            fake_audio_sink_ = gst_element_factory_make("autoaudiosink", "fake_audio_sink");
            fake_audio_convert_ = gst_element_factory_make("audioconvert", "fake_audio_convert");
            fake_audio_resample_ = gst_element_factory_make("audioresample", "fake_audio_resample");
            g_object_set(fake_audio_sink_, "sync", FALSE, nullptr);
            gst_bin_add_many(GST_BIN(Pipeline()),
                             fake_audio_decodec_,
                             fake_audio_queue_,
                             fake_audio_sink_,
                             fake_audio_convert_,
                             fake_audio_resample_,
                             nullptr);
            gst_element_link_many(fake_audio_queue_,
                                  fake_audio_decodec_,
                                  fake_audio_convert_,
                                  fake_audio_resample_,
                                  fake_audio_sink_,
                                  nullptr);
        } else {
            fake_audio_sink_ = gst_element_factory_make("fakesink", "fake_audio_sink");
            g_object_set(fake_audio_sink_, "sync", FALSE, nullptr);
            gst_bin_add_many(GST_BIN(Pipeline()), fake_audio_queue_, fake_audio_sink_, nullptr);
            gst_element_link_many(fake_audio_queue_, fake_audio_sink_, nullptr);
        }


        g_warn_if_fail(gst_element_link(audio_tee(), fake_audio_queue_));
        // GstPadTemplate *templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(audio_tee()), "src_%u");
        // audio_tee_pad_ = gst_element_request_pad(audio_tee(), templ, nullptr, nullptr);
        // GstPad *sinkpad = gst_element_get_static_pad(fake_audio_queue_, "sink");
        // g_warn_if_fail(gst_pad_link(audio_tee_pad_, sinkpad) == GST_PAD_LINK_OK);
        // gst_object_unref(sinkpad);

        //monitor data probe
        GstPad *pad = gst_element_get_static_pad(fake_audio_queue_, "sink");
        gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, LiveStream::on_monitor_audio_data, this, nullptr);
        gst_object_unref(pad);
    }
}


void LiveStream::release_sources()
{
    for (auto ep : audiences_) {
        ep->Terminate();
        GST_DEBUG("[%s] remove audience: %s (type: %s)",
                  uname().c_str(),
                  ep->Id().c_str(),
                  ep->Protocol().c_str());
        delete ep;
    }
    audiences_.clear();
}
void LiveStream::Destroy()
{
    gst_element_set_state(Pipeline(), GST_STATE_NULL);
    performer_->Terminate();
    delete performer_;
    performer_ = nullptr;

    Connector::Destroy();
    GST_DEBUG("[%s] remove performer!", uname().c_str());
}
void LiveStream::On(Promise *promise)
{
    const json &j = promise->meta();
    std::string action = j["action"];
    if (action == "add_audience") {
        add_audience(promise);
    } else if (action == "remove_audience") {
        remove_audience(promise);
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


std::list<IEndpoint *>::iterator LiveStream::find_audience(const std::string &id)
{
    return std::find_if(audiences_.begin(),
                        audiences_.end(),
                        [&id](IEndpoint *ep) {
                            return ep->Id() == id;
                        });
}

void LiveStream::add_audience(Promise *promise)
{
    // get endpoint protocol
    json &j = promise->data();
    if (j.find("protocol") == j.end()) {
        GST_ERROR("[%s] no protocol in audience.", uname().c_str());
        promise->reject("no protocol in audience.");
        return;
    }
    const std::string &id = j["endpoint_id"];
    EndpointType protocol = j["protocol"];
    if (find_audience(id) != audiences_.end()) {
        GST_ERROR("[%s] audience: %s has been added.", uname().c_str(), id.c_str());
        promise->reject("audience: " + id + " has been added.");
        return;
    }
    // create endpoint
    IEndpoint *ep = nullptr;
    switch (protocol) {
        case EndpointType::kRtspServer: {
            ep = new RtspServer(this, id);
            // set launch
            std::string launch = "( ";
            if (!VideoEncoding().empty())
                launch += "rtp" + VideoEncoding() + "pay pt=96 name=pay0";
            if (!AudioEncoding().empty())
                launch += "  rtp" + AudioEncoding() + "pay pt=97 name=pay1";
            launch += " )";
            j["launch"] = launch;
        } break;
        case EndpointType::kWebrtc: {
            ep = new Webrtc(this, id);
        } break;
        default: {
            GST_ERROR("[%s] protocol: %s not supported.", uname().c_str(), ep->Protocol().c_str());
            promise->reject("protocol: " + ep->Protocol() + " not supported.");
            return;
        }
    }
    bool rc = ep->Initialize(promise);
    if (rc) {
        // add endpoint to pipeline and link with tee
        audiences_.push_back(ep);
        GST_INFO("[%s] add audience: %s (type: %s)", uname().c_str(), id.c_str(), ep->Protocol().c_str());
        promise->resolve();
        return;
    }
    ep->Terminate();
    delete ep;
    ep = nullptr;
    GST_ERROR("[%s] add audience: %s failed!", uname().c_str(), id.c_str());
    promise->reject("add audience " + id + " failed!");
}
void LiveStream::remove_audience(Promise *promise)
{
    const json &j = promise->data();
    const std::string &id = j["endpoint_id"];
    auto it = find_audience(id);
    if (it == audiences_.end()) {
        GST_ERROR("[%s] audience: %s has not been added.", uname().c_str(), id.c_str());
        promise->reject(" audience: " + id + " has not been added.");
        return;
    }
    IEndpoint *ep = *it;
    ep->Terminate();
    audiences_.erase(it);

    GST_INFO("[%s] remove audience: %s (type: %s)", uname().c_str(), id.c_str(), ep->Protocol().c_str());

    delete ep;
    promise->resolve();
}

void LiveStream::set_remote_description(Promise *promise)
{
    const json &j = promise->data();

    const std::string &endpoint_id = j["endpoint_id"];
    auto it = find_audience(endpoint_id);
    if (it == audiences_.end()) {
        GST_ERROR("[%s] audience: %s has not been added.", uname().c_str(), endpoint_id.c_str());
        promise->reject(" audience: " + endpoint_id + " has not been added.");
        return;
    }
    Webrtc *ep = static_cast<Webrtc *>(*it);
    ep->SetRemoteDescription(promise);

    promise->resolve();
}
void LiveStream::set_remote_candidate(Promise *promise)
{
    const json &j = promise->data();

    const std::string &endpoint_id = j["endpoint_id"];
    auto it = find_audience(endpoint_id);
    if (it == audiences_.end()) {
        GST_ERROR("[%s] audience: %s has not been added.", uname().c_str(), endpoint_id.c_str());
        promise->reject("audience: " + endpoint_id + " has not been added.");
        return;
    }
    Webrtc *ep = static_cast<Webrtc *>(*it);
    ep->SetRemoteCandidate(promise);

    promise->resolve();
}

GstPadProbeReturn LiveStream::on_monitor_video_data(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    LiveStream *instance = static_cast<LiveStream *>(user_data);
    json meta;
    meta["topic"] = "livestream@default_audience";
    meta["origin"] = instance->uname();
    json data;
    data["msg"] = "video data received";
    instance->Notify(meta, data);
    // return GST_PAD_PROBE_OK;

    return GST_PAD_PROBE_REMOVE;
}

GstPadProbeReturn LiveStream::on_monitor_audio_data(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    LiveStream *instance = static_cast<LiveStream *>(user_data);
    json meta;
    meta["topic"] = "livestream@default_audience";
    meta["origin"] = instance->uname();
    json data;
    data["msg"] = "audio data received";
    instance->Notify(meta, data);
    // return GST_PAD_PROBE_OK;
    return GST_PAD_PROBE_REMOVE;
}