#include <framework/connector.hpp>

Connector::Connector(const std::string &id, StreamMatrix *instance)
    : IApp(id, instance)
{
}

void Connector::AddPipeJoint(GstElement *upstream_joint, GstElement *downstream_joint)
{
    if (upstream_joint != NULL)
        link_stream_output_joint(upstream_joint);
    if (downstream_joint != NULL)
        link_stream_input_joint(downstream_joint);
}

void Connector::RemovePipeJoint(GstElement *upstream_joint, GstElement *downstream_joint)
{
    if (upstream_joint != NULL)
        remove_stream_output_joint(upstream_joint);
    if (downstream_joint != NULL)
        remove_stream_input_joint(downstream_joint);
}

void Connector::link_stream_output_joint(GstElement *upstream_joint)
{
    g_assert_nonnull(upstream_joint);
    std::lock_guard<std::mutex> lck(tee_mutex_);

    gchar *media_type = (gchar *)g_object_get_data(G_OBJECT(upstream_joint), "media-type");
    g_assert_nonnull(media_type);
    if (g_str_equal(media_type, "video")) {
        GstPadTemplate *templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(video_tee_), "src_%u");
        GstPad *pad = gst_element_request_pad(video_tee_, templ, NULL, NULL);
        sink_link *info = new sink_link(pad, upstream_joint, this);

        g_warn_if_fail(gst_bin_add(GST_BIN(pipeline()), upstream_joint));
        gst_element_sync_state_with_parent(upstream_joint);

        GstPad *sinkpad = gst_element_get_static_pad(upstream_joint, "sink");

        // GstPad *testpad = gst_element_get_static_pad(upstream_joint, "sink");
        // gst_pad_add_probe(testpad, GST_PAD_PROBE_TYPE_BUFFER, on_monitor_data, NULL, NULL);
        // gst_object_unref(testpad);

        GstPadLinkReturn ret = gst_pad_link(pad, sinkpad);
        g_warn_if_fail(ret == GST_PAD_LINK_OK);
        gst_object_unref(sinkpad);

        tee_sinks_.push_back(info);
        GST_DEBUG("[Connector] link output video.");
    } else if (g_str_equal(media_type, "audio")) {
        GstPadTemplate *templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(audio_tee_), "src_%u");
        GstPad *pad = gst_element_request_pad(audio_tee_, templ, NULL, NULL);
        sink_link *info = new sink_link(pad, upstream_joint, this);

        g_warn_if_fail(gst_bin_add(GST_BIN(pipeline()), upstream_joint));
        gst_element_sync_state_with_parent(upstream_joint);

        GstPad *sinkpad = gst_element_get_static_pad(upstream_joint, "sink");
        GstPadLinkReturn ret = gst_pad_link(pad, sinkpad);
        g_warn_if_fail(ret == GST_PAD_LINK_OK);
        gst_object_unref(sinkpad);

        tee_sinks_.push_back(info);
        GST_DEBUG("[Connector] link output audio.");
        // g_print("~~~~~~~~~~link output audio~~~~~~~\n");
    }
}
void Connector::remove_stream_output_joint(GstElement *upstream_joint)
{
    std::lock_guard<std::mutex> lck(tee_mutex_);


    auto it = tee_sinks_.begin();
    for (; it != tee_sinks_.end(); ++it) {
        if ((*it)->joint == upstream_joint) {
            break;
        }
    }
    if (it == tee_sinks_.end()) {
        g_warn_if_reached();
        // TODO(yuanjunjie) notify application
        return;
    }


    gchar *media_type = (gchar *)g_object_get_data(G_OBJECT(upstream_joint), "media-type");
    if (g_str_equal(media_type, "video")) {
        (*it)->video_probe_invoke_control = TRUE;
        gst_pad_add_probe((*it)->request_pad, GST_PAD_PROBE_TYPE_IDLE, on_request_pad_remove_video_probe, *it, NULL);
        GST_DEBUG("[Connector] remove output video.");
    } else if (g_str_equal(media_type, "audio")) {
        (*it)->audio_probe_invoke_control = TRUE;
        gst_pad_add_probe((*it)->request_pad, GST_PAD_PROBE_TYPE_IDLE, on_request_pad_remove_audio_probe, *it, NULL);
        GST_DEBUG("[Connector] remove output audio.");
    }
    tee_sinks_.erase(it);
}
void Connector::link_stream_input_joint(GstElement *downstream_joint)
{
    g_assert_nonnull(downstream_joint);
    std::lock_guard<std::mutex> lck(selector_mutex_);

    gchar *media_type = (gchar *)g_object_get_data(G_OBJECT(downstream_joint), "media-type");
    g_assert_nonnull(media_type);

    if (g_str_equal(media_type, "video")) {
        GstPadTemplate *templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(video_selector_), "sink_%u");
        GstPad *pad = gst_element_request_pad(video_selector_, templ, NULL, NULL);
        sink_link *info = new sink_link(pad, downstream_joint, this, false);

        g_warn_if_fail(gst_bin_add(GST_BIN(pipeline()), downstream_joint));
        gst_element_sync_state_with_parent(downstream_joint);


        GstPad *srcpad = gst_element_get_static_pad(downstream_joint, "src");

        // GstPad *testpad = gst_element_get_static_pad(downstream_joint, "src");
        // gst_pad_add_probe(testpad, GST_PAD_PROBE_TYPE_BUFFER, on_monitor_data, NULL, NULL);
        // gst_object_unref(testpad);

        GstPadLinkReturn ret = gst_pad_link(srcpad, pad);
        g_warn_if_fail(ret == GST_PAD_LINK_OK);
        gst_object_unref(srcpad);

        selector_sinks_.push_back(info);
        GST_DEBUG("[Connector] link input video.");
    } else if (g_str_equal(media_type, "audio")) {
        GstPadTemplate *templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(audio_selector_), "sink_%u");
        GstPad *pad = gst_element_request_pad(audio_selector_, templ, NULL, NULL);

        sink_link *info = new sink_link(pad, downstream_joint, this, false);

        g_warn_if_fail(gst_bin_add(GST_BIN(pipeline()), downstream_joint));
        gst_element_sync_state_with_parent(downstream_joint);

        GstPad *srcpad = gst_element_get_static_pad(downstream_joint, "src");

        // GstPad *testpad = gst_element_get_static_pad(downstream_joint, "src");
        // gst_pad_add_probe(testpad, GST_PAD_PROBE_TYPE_BUFFER, on_monitor_data, NULL, NULL);
        // gst_object_unref(testpad);

        GstPadLinkReturn ret = gst_pad_link(srcpad, pad);
        g_warn_if_fail(ret == GST_PAD_LINK_OK);
        gst_object_unref(srcpad);

        selector_sinks_.push_back(info);
        GST_DEBUG("[Connector] link input audio.");
        // printf("~~~~~~~~~audio ---> selector\n");
    }
}
void Connector::remove_stream_input_joint(GstElement *downstream_joint)
{
    std::lock_guard<std::mutex> lck(selector_mutex_);

    auto it = selector_sinks_.begin();
    for (; it != selector_sinks_.end(); ++it) {
        if ((*it)->joint == downstream_joint) {
            break;
        }
    }
    if (it == selector_sinks_.end()) {
        g_warn_if_reached();
        // TODO(yuanjunjie) notify application
        return;
    }

    gchar *media_type = (gchar *)g_object_get_data(G_OBJECT(downstream_joint), "media-type");
    if (g_str_equal(media_type, "video")) {
        (*it)->video_probe_invoke_control = TRUE;
        gst_pad_add_probe((*it)->request_pad, GST_PAD_PROBE_TYPE_IDLE, on_request_pad_remove_video_probe, *it, NULL);
        GST_DEBUG("[Connector] remove input video.");
    } else if (g_str_equal(media_type, "audio")) {
        (*it)->audio_probe_invoke_control = TRUE;
        gst_pad_add_probe((*it)->request_pad, GST_PAD_PROBE_TYPE_IDLE, on_request_pad_remove_audio_probe, *it, NULL);
        GST_DEBUG("[Connector] remove input audio.");
    }
    selector_sinks_.erase(it);
}
GstPadProbeReturn Connector::on_request_pad_remove_video_probe(GstPad *teepad, GstPadProbeInfo *probe_info, gpointer data)
{
    sink_link *info = static_cast<sink_link *>(data);
    if (!g_atomic_int_compare_and_exchange(&info->video_probe_invoke_control, TRUE, FALSE)) {
        return GST_PAD_PROBE_OK;
    }

    GstElement *joint = info->joint;
    Connector *pipeline = static_cast<Connector *>(info->pipeline);

    // remove pipeline dynamicaly
    if (info->is_output) {
        GstPad *sinkpad = gst_element_get_static_pad(joint, "sink");
        gst_pad_unlink(info->request_pad, sinkpad);
        gst_object_unref(sinkpad);
        gst_element_set_state(joint, GST_STATE_NULL);
        g_warn_if_fail(gst_bin_remove(GST_BIN(pipeline->pipeline()), joint));
        gst_element_release_request_pad(pipeline->video_tee_, info->request_pad);
        gst_object_unref(info->request_pad);
        // printf("~~~~~~~remove video tee pad\n");
    } else {
        GstPad *srckpad = gst_element_get_static_pad(joint, "src");
        gst_pad_unlink(srckpad, info->request_pad);
        gst_object_unref(srckpad);
        gst_element_set_state(joint, GST_STATE_NULL);
        g_warn_if_fail(gst_bin_remove(GST_BIN(pipeline->pipeline()), joint));
        gst_element_release_request_pad(pipeline->video_selector_, info->request_pad);
        gst_object_unref(info->request_pad);
        // printf("~~~~~~~remove video selector pad\n");
    }
    delete static_cast<sink_link *>(data);
    return GST_PAD_PROBE_REMOVE;
}
GstPadProbeReturn Connector::on_request_pad_remove_audio_probe(GstPad *teepad, GstPadProbeInfo *probe_info, gpointer data)
{
    sink_link *info = static_cast<sink_link *>(data);
    if (!g_atomic_int_compare_and_exchange(&info->audio_probe_invoke_control, TRUE, FALSE)) {
        return GST_PAD_PROBE_OK;
    }

    GstElement *joint = info->joint;
    Connector *pipeline = static_cast<Connector *>(info->pipeline);

    // remove pipeline dynamicaly
    if (info->is_output) {
        GstPad *sinkpad = gst_element_get_static_pad(joint, "sink");
        gst_pad_unlink(info->request_pad, sinkpad);
        gst_object_unref(sinkpad);
        gst_element_set_state(joint, GST_STATE_NULL);
        g_warn_if_fail(gst_bin_remove(GST_BIN(pipeline->pipeline()), joint));
        gst_element_release_request_pad(pipeline->audio_tee_, info->request_pad);
        gst_object_unref(info->request_pad);
        // printf("~~~~~~~remove audio tee pad\n");
    } else {
        GstPad *srckpad = gst_element_get_static_pad(joint, "src");
        gst_pad_unlink(srckpad, info->request_pad);
        gst_object_unref(srckpad);
        gst_element_set_state(joint, GST_STATE_NULL);
        g_warn_if_fail(gst_bin_remove(GST_BIN(pipeline->pipeline()), joint));
        gst_element_release_request_pad(pipeline->audio_selector_, info->request_pad);
        gst_object_unref(info->request_pad);
        // printf("~~~~~~~remove audio selector pad\n");
    }
    delete static_cast<sink_link *>(data);
    return GST_PAD_PROBE_REMOVE;
}