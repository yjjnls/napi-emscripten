#ifndef STREAM_MATRIX_ENDPOINT_WEBRTC_HPP
#define STREAM_MATRIX_ENDPOINT_WEBRTC_HPP

#include <framework/endpoint.hpp>

class Webrtc : public IEndpoint
{
 public:
    Webrtc(IApp *app, const std::string &id);
    ~Webrtc() {}

    bool Initialize(Promise *promise);
    void Terminate();

    void SetRemoteDescription(Promise *promise);
    void SetRemoteCandidate(Promise *promise);

    std::string uname() { return "Webrtc@" + Id(); }
    GstElement *Pipeline() { return pipeline_; }

    GstElement *VideoInputPipejoint() { return video_input_joint().downstream_joint; }
    GstElement *AudioInputPipejoint() { return audio_input_joint().downstream_joint; }

 private:
    bool parse_launch();
    static void on_ice_candidate(GstElement *webrtc G_GNUC_UNUSED,
                                 guint mlineindex,
                                 gchar *candidate,
                                 gpointer user_data G_GNUC_UNUSED);
    static void on_sdp_created(GstPromise *promise, gpointer user_data);
    static void on_negotiation_needed(GstElement *element, gpointer user_data);
    static void on_webrtc_pad_added(GstElement *webrtc, GstPad *new_pad, gpointer user_data);

    static GstPadProbeReturn on_monitor_data(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);

    GstElement *pipeline_;
    GstElement *bin_;
    GstElement *webrtc_;

    WebrtcRole role_;
    std::string launch_;
    EndpointType protocol_;
    static int session_count_;
};


#endif