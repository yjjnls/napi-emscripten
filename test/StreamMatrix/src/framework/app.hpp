#ifndef STREAM_MATRIX_FRAMEWORK_APP_HPP
#define STREAM_MATRIX_FRAMEWORK_APP_HPP

#include <framework/promise.hpp>

class StreamMatrix;
struct sink_link
{
    GstElement *joint;
    GstPad *request_pad;
    void *pipeline;
    gboolean video_probe_invoke_control;
    gboolean audio_probe_invoke_control;
    bool is_output;

    sink_link(GstPad *pad, GstElement *joint_element, void *pipe, bool output = true)
        : joint(joint_element)
        , request_pad(pad)
        , pipeline(pipe)
        , video_probe_invoke_control(FALSE)
        , audio_probe_invoke_control(FALSE)
        , is_output(output)
    {
    }
};
class IApp
{
 public:
    IApp(const std::string &id, StreamMatrix *instance)
        : id_(id)
        , pipeline_(NULL)
        , stream_matrix_(instance)

    {
    }
    virtual ~IApp() {}

    virtual bool Initialize(Promise *promise);
    /*
     * return false if the destroy not complete
    */
    virtual bool Destroy(Promise *promise);

    virtual void On(Promise *promise) = 0;

    virtual void Notify(const nlohmann::json &data, const nlohmann::json &meta);

 protected:
    // virtual const char *type() const = 0;
    virtual std::string uname() = 0;

    StreamMatrix &stream_matrix() { return *stream_matrix_; }
    GstElement *pipeline() { return pipeline_; }
    std::string id() { return id_; }
    const std::string &video_encoding() const { return video_encoding_; }
    std::string &video_encoding() { return video_encoding_; }
    const std::string &audio_encoding() const { return audio_encoding_; }
    std::string &audio_encoding() { return audio_encoding_; }

    virtual void add_pipe_joint(GstElement *upstream_joint, GstElement *downstream_joint) {}
    virtual void remove_pipe_joint(GstElement *upstream_joint, GstElement *downstream_joint) {}

 private:
    std::string id_;
    GstElement *pipeline_;  // main pipeline
    StreamMatrix *stream_matrix_;

    std::string video_encoding_;
    std::string audio_encoding_;
};

#endif
