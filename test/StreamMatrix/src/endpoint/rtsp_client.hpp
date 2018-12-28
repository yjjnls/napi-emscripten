#ifndef STREAM_MATRIX_ENDPOINT_RTSP_CLIENT_HPP
#define STREAM_MATRIX_ENDPOINT_RTSP_CLIENT_HPP

#include <framework/endpoint.hpp>

class RtspClient : public IEndpoint
{
 public:
    RtspClient(IApp *app, const std::string &id);
    ~RtspClient() {}

    bool Initialize(Promise *promise);
    void Terminate();

 private:
    bool add_to_pipeline();

    static void on_rtspsrc_pad_added(GstElement *src,
                                     GstPad *src_pad,
                                     gpointer depay);
    static gboolean on_rtspsrc_select_stream(GstElement *src,
                                             guint stream_id,
                                             GstCaps *stream_caps,
                                             gpointer rtspclient);
    // only for test
    static GstPadProbeReturn on_monitor_data(GstPad *pad,
                                             GstPadProbeInfo *info,
                                             gpointer rtspclient);

    GstElement *rtspsrc_;
    GstElement *rtpdepay_video_;
    GstElement *parse_video_;
    GstElement *rtpdepay_audio_;
    bool add_to_pipeline_;
};
#endif