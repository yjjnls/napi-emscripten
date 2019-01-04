#include <app/rtsp_analyzer.hpp>
#include <gst/video/video.h>

using json = nlohmann::json;

GST_DEBUG_CATEGORY_STATIC(my_category);
#define GST_CAT_DEFAULT my_category


RtspAnalyzer::RtspAnalyzer(const std::string &id,
                           StreamMatrix *instance)
    : Launcher(id, instance)
    , sink_(nullptr)
    , frame_(10)
    , cur_video_frame_(0)
    , cur_audio_frame_(0)
{
    GST_DEBUG_CATEGORY_INIT(my_category, "stream_matrix", 2, "stream_matrix");
}
RtspAnalyzer::~RtspAnalyzer()
{
    for (auto it : bmps_) {
        delete[] it;
    }
}
void RtspAnalyzer::startup(Promise *promise)
{
    const json &j = promise->data();

    sink_ = gst_bin_get_by_name(GST_BIN(Pipeline()), "sink");
    if (sink_ == nullptr) {
        GST_INFO("[RtspAnalyzer] %s use multifilesink for test.", uname().c_str());
    } else {
        GST_INFO("[RtspAnalyzer] %s use raw image data for test.", uname().c_str());
        if (j.find("frame") != j.end()) {
            frame_ = j["frame"];
        }
        GstPad *pad = gst_element_get_static_pad(sink_, "sink");
        gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, RtspAnalyzer::on_have_data, this, nullptr);
        gst_object_unref(pad);
    }

    Launcher::startup(promise);
}

GstPadProbeReturn RtspAnalyzer::on_have_data(GstPad *pad,
                                             GstPadProbeInfo *info,
                                             gpointer user_data)
{
    RtspAnalyzer *app = static_cast<RtspAnalyzer *>(user_data);
    if (app->cur_video_frame_ <= (app->frame_ * 2)) {
        if (++app->cur_video_frame_ % 2 == 0)
            app->on_save_image(user_data);
        return GST_PAD_PROBE_OK;
    } else {
        return GST_PAD_PROBE_REMOVE;
    }
}
gboolean RtspAnalyzer::on_save_image(gpointer user_data)
{
    RtspAnalyzer *app = static_cast<RtspAnalyzer *>(user_data);

    GstSample *from_sample;
    g_object_get(app->sink_, "last-sample", &from_sample, nullptr);
    if (from_sample == nullptr) {
        GST_ERROR("[RtspAnalyzer] %s failed getting sample.", app->uname().c_str());
        return FALSE;
    }
    GstCaps *caps = gst_caps_from_string("image/bmp");

    if (caps == nullptr) {
        GST_ERROR("[RtspAnalyzer] %s failed getting caps.", app->uname().c_str());
        return FALSE;
    }

    GError *err = nullptr;
    GstSample *to_sample = gst_video_convert_sample(from_sample, caps, GST_CLOCK_TIME_NONE, &err);
    gst_caps_unref(caps);
    gst_sample_unref(from_sample);

    if (to_sample == nullptr && err) {
        GST_ERROR("[RtspAnalyzer] %s failed converting frame.", app->uname().c_str());
        GST_ERROR("[RtspAnalyzer] %s error : %s", app->uname().c_str(), err->message);
        return FALSE;
    }
    GstBuffer *buf = gst_sample_get_buffer(to_sample);

    GstClockTime stream_time = GST_BUFFER_TIMESTAMP(buf);
    GstClockTime duration = GST_BUFFER_DURATION(buf);

    GstMapInfo map_info;
    if (!gst_buffer_map(buf, &map_info, GST_MAP_READ)) {
        GST_ERROR("[RtspAnalyzer] %s could not get image data from gstbuffer", app->uname().c_str());
        gst_sample_unref(to_sample);

        return FALSE;
    }

    gst_sample_unref(to_sample);

    json data;
    data["duration"] = duration;
    data["stream-time"] = stream_time;

    unsigned char *bmp_buffer = new unsigned char[map_info.size];
    memcpy(bmp_buffer, map_info.data, map_info.size);
    app->bmps_.push_back(bmp_buffer);
    uint64_t pointer = reinterpret_cast<uint64_t>(bmp_buffer);
    // printf("%p\n%ld\n%p\n",bmp_buffer,pointer,reinterpret_cast<unsigned char *>(pointer));

    data["data"] = pointer;
    data["size"] = map_info.size;

    json meta;
    meta["topic"] = "image_data";
    meta["origin"] = app->uname();

    app->Notify(meta, data);
    return TRUE;
}

void RtspAnalyzer::on_message(GstBus *bus, GstMessage *message)
{
    if (message->type == GST_MESSAGE_ELEMENT) {
        const gchar *name = GST_MESSAGE_SRC_NAME(message);
        const GstStructure *s = gst_message_get_structure(message);
        const gchar *type = gst_structure_get_name(s);

        if (!g_strcmp0(type, "spectrum")) {
            on_spectrum(name, s);
        }
    }
}
void RtspAnalyzer::on_spectrum(const std::string &name, const GstStructure *message)
{
    GstClockTime endtime;
    const GValue *magnitudes;
    const GValue *phases;
    const GValue *mag, *phase;
    guint size;

    if (!gst_structure_get_clock_time(message, "endtime", &endtime))
        endtime = GST_CLOCK_TIME_NONE;

    magnitudes = gst_structure_get_value(message, "magnitude");
    phases = gst_structure_get_value(message, "phase");
    size = gst_value_list_get_size(magnitudes);
    json data;


    json::array_t mags;
    for (guint i = 0; i < size; ++i) {
        mag = gst_value_list_get_value(magnitudes, i);
        phase = gst_value_list_get_value(phases, i);
        mags.push_back(g_value_get_float(mag));
    }
    data["name"] = name;
    data["endtime"] = endtime;
    data["magnitude"] = mags;

    json meta;
    meta["topic"] = "spectrum";
    meta["origin"] = this->uname();

    if (cur_audio_frame_++ < frame_) {
        Notify(meta, data);
    }
}