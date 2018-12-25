#ifndef STREAM_MATRIX_APPLICATION_RTSP_TEST_CLIENT_HPP
#define STREAM_MATRIX_APPLICATION_RTSP_TEST_CLIENT_HPP

#include <app/launcher.hpp>

class RtspTestClient : public Launcher
{
 public:
    RtspTestClient(const std::string &id, StreamMatrix *instance);

    ~RtspTestClient() {}
    // bool Initialize();

 protected:
    std::string uname() { return id() + "@RtspTestClient"; }
    void startup(Promise *promise);
    static gboolean on_save_image(gpointer user_data);
    static GstPadProbeReturn on_have_data(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);
    void on_message(GstBus *bus, GstMessage *message);
    void on_spectrum(const std::string &name, const GstStructure *message);

 private:
    GstElement *sink_;
    int frame_;
    int cur_frame_;
    std::vector<unsigned char *> bmps_;
};

#endif