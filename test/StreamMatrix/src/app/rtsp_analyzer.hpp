#ifndef STREAM_MATRIX_APPLICATION_RTSP_ANALYZER_HPP
#define STREAM_MATRIX_APPLICATION_RTSP_ANALYZER_HPP

#include <app/launcher.hpp>

class Analyzer : public Launcher
{
 public:
    Analyzer(const std::string &id, StreamMatrix *instance);

    ~Analyzer();
    // bool Initialize();

 protected:
    void startup(Promise *promise);

    static gboolean on_save_image(gpointer user_data);
    static GstPadProbeReturn on_have_data(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);
    void on_message(GstBus *bus, GstMessage *message);
    void on_spectrum(const std::string &name, const GstStructure *message);

 private:
    GstElement *sink_;
    int frame_;
    int cur_video_frame_;
    int cur_audio_frame_;
    std::vector<unsigned char *> bmps_;
};

class RtspAnalyzer : public Analyzer
{
 public:
    RtspAnalyzer(const std::string &id, StreamMatrix *instance);

    ~RtspAnalyzer();

 protected:
    std::string uname() { return "RtspAnalyzer@" + id(); }
};

#endif