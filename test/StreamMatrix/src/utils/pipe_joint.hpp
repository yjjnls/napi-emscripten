#ifndef STREAM_MATRIX_UTILS_PIPEJOINT_HPP
#define STREAM_MATRIX_UTILS_PIPEJOINT_HPP

#include <gst/gst.h>
#include <string>

struct PipeJoint
{
    GstElement *upstream_joint = NULL;
    GstElement *downstream_joint = NULL;
};

PipeJoint make_pipe_joint(const std::string &media_type = "video",
                          const std::string &name = "")
{
    std::string name_ = name;
    if (name_.empty()) {
        static int id = 0;
        name_ = std::to_string(id++);
    }

    GstElement *psink = gst_element_factory_make("proxysink", (name_ + "_proxysink").c_str());
    GstElement *psrc = gst_element_factory_make("proxysrc", (name_ + "_proxysrc").c_str());
    g_object_set(psrc, "proxysink", psink, NULL);
    g_object_set_data(G_OBJECT(psink), "media-type", (gchar *)media_type.c_str());
    g_object_set_data(G_OBJECT(psrc), "media-type", (gchar *)media_type.c_str());

    ///-----------------
    PipeJoint pipejoint;
    pipejoint.upstream_joint = psink;
    pipejoint.downstream_joint = psrc;
    return pipejoint;
}
void update_downstream_joint(PipeJoint *pipejoint)
{
    GstElement *psrc = gst_element_factory_make("proxysrc", NULL);
    g_object_set(psrc, "proxysink", pipejoint->upstream_joint, NULL);
    pipejoint->downstream_joint = psrc;
}
void destroy_pipe_joint(PipeJoint *pipejoint)
{
    pipejoint->upstream_joint = NULL;
    pipejoint->downstream_joint = NULL;
}

#endif