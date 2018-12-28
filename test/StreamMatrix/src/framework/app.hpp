#ifndef STREAM_MATRIX_FRAMEWORK_APP_HPP
#define STREAM_MATRIX_FRAMEWORK_APP_HPP

#include <framework/promise.hpp>

class StreamMatrix;
class IApp
{
 public:
    IApp(const std::string &id, StreamMatrix *instance)
        : id_(id)
        , pipeline_(nullptr)
        , stream_matrix_(instance)

    {
    }
    virtual ~IApp() {}

    virtual bool Initialize(Promise *promise);
    virtual void Destroy();
    virtual void On(Promise *promise) = 0;
    virtual void Notify(const nlohmann::json &data, const nlohmann::json &meta);

    virtual void AddPipeJoint(GstElement *upstream_joint, GstElement *downstream_joint){};
    virtual void RemovePipeJoint(GstElement *upstream_joint, GstElement *downstream_joint){};

    const std::string &VideoEncoding() const { return video_encoding_; }
    std::string &VideoEncoding() { return video_encoding_; }
    const std::string &AudioEncoding() const { return audio_encoding_; }
    std::string &AudioEncoding() { return audio_encoding_; }
    GstElement *Pipeline() { return pipeline_; }

    virtual std::string uname() = 0;

 protected:
    // virtual const char *type() const = 0;

    StreamMatrix &stream_matrix() { return *stream_matrix_; }
    std::string id() { return id_; }


 private:
    std::string id_;
    GstElement *pipeline_;  // main pipeline
    StreamMatrix *stream_matrix_;

    std::string video_encoding_;
    std::string audio_encoding_;
};

#endif
