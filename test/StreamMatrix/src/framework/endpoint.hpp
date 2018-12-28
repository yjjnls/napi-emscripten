#ifndef STREAM_MATRIX_FRAMEWORK_ENDPOINT_HPP
#define STREAM_MATRIX_FRAMEWORK_ENDPOINT_HPP

#include <framework/promise.hpp>

class IApp;
class IEndpoint
{
 public:
    IEndpoint(IApp *app, const std::string &id)
        : app_(app)
        , id_(id)
    {
    }
    virtual ~IEndpoint() {}

    virtual bool Initialize(Promise *promise) { return true; }
    virtual void Terminate() {}

    const std::string &Protocol() const { return protocol_; }
    std::string &Protocol() { return protocol_; }

 protected:
    IApp *app() { return app_; }
    const std::string &id() { return id_; }

    PipeJoint &video_input_joint() { return video_input_joint_; }
    PipeJoint &video_output_joint() { return video_output_joint_; }
    PipeJoint &audio_input_joint() { return audio_input_joint_; }
    PipeJoint &audio_output_joint() { return audio_output_joint_; }

 private:
    IApp *app_;
    std::string id_;
    std::string protocol_;

    PipeJoint video_input_joint_;
    PipeJoint video_output_joint_;
    PipeJoint audio_input_joint_;
    PipeJoint audio_output_joint_;
};
#endif