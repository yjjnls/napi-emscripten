#include <utils/pipe_joint.hpp>

PipeJoint make_pipe_joint(const std::string &media_type, const std::string &name)
{
    std::string name_ = name;
    if (name_.empty()) {
        static int id = 0;
        name_ = std::to_string(id++);
    }

    GstElement *psink = gst_element_factory_make("proxysink", (name_ + "_proxysink").c_str());
    GstElement *psrc = gst_element_factory_make("proxysrc", (name_ + "_proxysrc").c_str());
    g_object_set(psrc, "proxysink", psink, nullptr);
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
    GstElement *psrc = gst_element_factory_make("proxysrc", nullptr);
    g_object_set(psrc, "proxysink", pipejoint->upstream_joint, nullptr);
    pipejoint->downstream_joint = psrc;
}
void destroy_pipe_joint(PipeJoint *pipejoint)
{
    pipejoint->upstream_joint = nullptr;
    pipejoint->downstream_joint = nullptr;
}