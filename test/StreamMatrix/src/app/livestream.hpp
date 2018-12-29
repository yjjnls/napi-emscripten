#ifndef STREAM_MATRIX_APPLICATION_LIVESTREAM_HPP
#define STREAM_MATRIX_APPLICATION_LIVESTREAM_HPP

#include <framework/connector.hpp>
#include <framework/endpoint.hpp>

class LiveStream : public Connector
{
 public:
    LiveStream(const std::string &id, StreamMatrix *instance);
    ~LiveStream() {}

    bool Initialize(Promise *promise);
    void On(Promise *promise);

    std::string uname() { return "LiveStream@" + id(); }

 protected:
 private:
    bool on_add_endpoint(IEndpoint *endpoint);
    std::list<IEndpoint *>::iterator find_audience(const std::string &id);
    void add_audience(Promise *promise);
    void remove_audience(Promise *promise);
    void set_remote_description(Promise *promise);
    void set_remote_candidate(Promise *promise);

    IEndpoint *performer_;
    std::list<IEndpoint *> audiences_;

    // GstPad *video_tee_pad_;
    // GstPad *audio_tee_pad_;
    GstElement *fake_video_queue_;
    GstElement *fake_video_sink_;
    GstElement *fake_audio_queue_;
    GstElement *fake_audio_sink_;

    GstElement *fake_video_decodec_;
    GstElement *fake_audio_decodec_;
    GstElement *fake_audio_convert_;
    GstElement *fake_audio_resample_;
};
#endif