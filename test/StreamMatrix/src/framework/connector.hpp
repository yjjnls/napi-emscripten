#ifndef STREAM_MATRIX_FRAMEWORK_CONNECTOR_HPP
#define STREAM_MATRIX_FRAMEWORK_CONNECTOR_HPP

#include <framework/app.hpp>


/*
          ┌------------------------------------------------------------------------------------------┐
          ┆          ┌─pipejoint─┐                                          ┌─pipejoint─┐            ┆
pipeline──┼(upstream_joint) (downstream_joint)───┐            ┌───(upstream_joint) (downstream_joint)┼──pipeline 
          ┆                                      |            |                                      ┆
pipeline──┼(upstream_joint) (downstream_joint)───selector───tee───(upstream_joint) (downstream_joint)┼──pipeline 
          ┆                                      |            |                                      ┆
pipeline──┼(upstream_joint) (downstream_joint)───┘            └───(upstream_joint) (downstream_joint)┼──pipeline 
          ┆                                                                                          ┆
          └------------------------------------------------------------------------------------------┘
*/
class Connector : public IApp
{
 public:
    Connector(const std::string &id, StreamMatrix *instance);
    virtual ~Connector() {}

    virtual void AddPipeJoint(GstElement *upstream_joint, GstElement *downstream_joint);
    virtual void RemovePipeJoint(GstElement *upstream_joint, GstElement *downstream_joint);

 protected:
 private:
    void link_stream_output_joint(GstElement *upstream_joint);
    void remove_stream_output_joint(GstElement *upstream_joint);
    void link_stream_input_joint(GstElement *downstream_joint);
    void remove_stream_input_joint(GstElement *downstream_joint);
    static GstPadProbeReturn
    on_request_pad_remove_video_probe(GstPad *pad,
                                      GstPadProbeInfo *probe_info,
                                      gpointer data);

    static GstPadProbeReturn
    on_request_pad_remove_audio_probe(GstPad *pad,
                                      GstPadProbeInfo *probe_info,
                                      gpointer data);

    GstElement *video_tee_;
    GstElement *audio_tee_;
    GstElement *video_selector_;
    GstElement *audio_selector_;
    std::list<sink_link *> tee_sinks_;  // all the request pad of tee,
                                        // release when removing from
                                        // pipeline
    std::list<sink_link *> selector_sinks_;
    static std::mutex tee_mutex_;
    static std::mutex selector_mutex_;
};
#endif