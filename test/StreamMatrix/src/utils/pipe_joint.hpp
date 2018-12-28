#ifndef STREAM_MATRIX_UTILS_PIPEJOINT_HPP
#define STREAM_MATRIX_UTILS_PIPEJOINT_HPP

#include <gst/gst.h>
#include <string>

struct PipeJoint
{
    GstElement *upstream_joint = nullptr;
    GstElement *downstream_joint = nullptr;
};

PipeJoint make_pipe_joint(const std::string &media_type = "video",
                          const std::string &name = "");
void update_downstream_joint(PipeJoint *pipejoint);
void destroy_pipe_joint(PipeJoint *pipejoint);

#endif