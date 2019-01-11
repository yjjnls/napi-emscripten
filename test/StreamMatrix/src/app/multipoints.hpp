
#ifndef STREAM_MATRIX_APPLICATION_MULTIPOINTS_HPP
#define STREAM_MATRIX_APPLICATION_MULTIPOINTS_HPP

#include <framework/connector.hpp>
#include <framework/endpoint.hpp>

class MultiPoints : public Connector
{
 public:
    MultiPoints(const std::string &id, StreamMatrix *instance);
    ~MultiPoints() {}

    bool Initialize(Promise *promise);
    void Destroy();
    void On(Promise *promise);

    std::string uname() { return "MultiPoints@" + id(); }

 protected:
    virtual void release_sources();

 private:
    std::list<IEndpoint *>::iterator find_member(const std::string &id);
    void add_member(Promise *promise);
    void remove_member(Promise *promise);
    void set_speaker(Promise *promise);
    void set_speaker_default();


    void set_remote_description(Promise *promise);
    void set_remote_candidate(Promise *promise);

    IEndpoint *speaker_;
    std::list<IEndpoint *> members_;
    GstElement *default_speaker_;
    GstElement *default_video_src_;
    GstElement *default_audio_src_;

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