#ifndef STREAM_MATRIX_APPLICATION_WEBRTC_ANALYZER_HPP
#define STREAM_MATRIX_APPLICATION_WEBRTC_ANALYZER_HPP

#include <app/rtsp_analyzer.hpp>
#include <endpoint/webrtc.hpp>

class WebrtcAnalyzer : public Analyzer
{
 public:
    WebrtcAnalyzer(const std::string &id, StreamMatrix *instance);
    ~WebrtcAnalyzer();

    bool Initialize(Promise *promise);
    void Destroy();
    void On(Promise *promise);

    virtual GstElement *Pipeline()
    {
        if (webrtc_ep_)
            return webrtc_ep_->Pipeline();
        return nullptr;
    }

 protected:
    std::string uname() { return "WebrtcAnalyzer@" + id(); }

    void startup(Promise *promise);
    void stop(Promise *promise);
    void set_remote_description(Promise *promise);
    void set_remote_candidate(Promise *promise);

 private:
    Webrtc *webrtc_ep_;
};

#endif