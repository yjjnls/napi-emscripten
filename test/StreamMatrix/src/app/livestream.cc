// #include <app/livestream.hpp>
// #include <endpoint/rtsp_client.hpp>

// using json = nlohmann::json;

// GST_DEBUG_CATEGORY_STATIC(my_category);
// #define GST_CAT_DEFAULT my_category

// #define USE_AUTO_SINK false

// LiveStream::LiveStream(const std::string &id, StreamMatrix *instance)
//     : Connector(id, instance)
// {
//     GST_DEBUG_CATEGORY_INIT(my_category, "stream_matrix", 2, "stream_matrix");
// }


// bool LiveStream::Initialize(Promise *promise)
// {
//     if (performer_ != nullptr) {
//         GST_ERROR("[LiveStream] there's already a performer!");
//         promise->reject("there's already a performer!");
//         return false;
//     }
//     Connector::Initialize(promise);

//     const json &j = promise->data();
//     const std::string &id = j["id"];

//     // create endpoint
//     performer_ = new RtspClient(this, "default_performer");
//     // initialize endpoint and add it to the pipeline
//     bool rc = performer_->Initialize(promise);
//     if (rc) {
//         // link endpoint to video/audio tee
//         if (on_add_endpoint(performer_)) {
//             GST_INFO("[LiveStream] %s add performer (type: %s)", id.c_str(), performer_->Protocol().c_str());
//             promise->resolve();
//             return true;
//         }
//     }
//     performer_->Terminate();
//     delete performer_;
//     performer_ = nullptr;
//     GST_ERROR("[LiveStream] %s add performer failed!", id.c_str());
//     promise->reject(id + " add performer failed!");
//     return false;
// }

// bool LiveStream::on_add_endpoint(IEndpoint *endpoint)
// {
//     // default: rtsp client
//     if (!VideoEncoding().empty()) {
//         GstElement *parse = gst_bin_get_by_name_recurse_up(GST_BIN(Pipeline()), "parse");
//         g_warn_if_fail(parse);

//         g_warn_if_fail(gst_element_link(parse, video_selector()));
//         fake_video_queue_ = gst_element_factory_make("queue", "fake_video_queue");

//         if (USE_AUTO_SINK) {
//             fake_video_decodec_ = gst_element_factory_make("avdec_h264", "fake_video_decodec");
//             fake_video_sink_ = gst_element_factory_make("autovideosink", "fake_video_sink");
//             g_object_set(fake_video_sink_, "sync", FALSE, nullptr);
//             gst_bin_add_many(GST_BIN(Pipeline()), fake_video_decodec_, fake_video_queue_, fake_video_sink_, nullptr);
//             gst_element_link_many(fake_video_queue_, fake_video_decodec_, fake_video_sink_, nullptr);
//         } else {
//             fake_video_sink_ = gst_element_factory_make("fakesink", "fake_video_sink");
//             g_object_set(fake_video_sink_, "sync", FALSE, nullptr);
//             gst_bin_add_many(GST_BIN(Pipeline()), fake_video_queue_, fake_video_sink_, nullptr);
//             gst_element_link_many(fake_video_queue_, fake_video_sink_, nullptr);
//         }


//         g_warn_if_fail(gst_element_link(video_tee(), fake_video_queue_));
//         // GstPadTemplate *templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(video_tee()), "src_%u");
//         // video_tee_pad_ = gst_element_request_pad(video_tee(), templ, nullptr, nullptr);
//         // GstPad *sinkpad = gst_element_get_static_pad(fake_video_queue_, "sink");
//         // g_warn_if_fail(gst_pad_link(video_tee_pad_, sinkpad) == GST_PAD_LINK_OK);
//         // gst_object_unref(sinkpad);

//         // monitor data probe
//         // GstPad *pad = gst_element_get_static_pad(fake_video_queue_, "src");
//         // gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, on_monitor_data, this, nullptr);
//         // gst_object_unref(pad);
//     }
//     if (!AudioEncoding().empty()) {
//         GstElement *audio_depay = gst_bin_get_by_name_recurse_up(GST_BIN(Pipeline()), "audio-depay");
//         g_warn_if_fail(audio_depay);

//         g_warn_if_fail(gst_element_link(audio_depay, audio_selector()));
//         fake_audio_queue_ = gst_element_factory_make("queue", "fake_audio_queue");

//         if (USE_AUTO_SINK) {
//             fake_audio_decodec_ = gst_element_factory_make("alawdec", "fake_audio_decodec");
//             fake_audio_sink_ = gst_element_factory_make("autoaudiosink", "fake_audio_sink");
//             fake_audio_convert_ = gst_element_factory_make("audioconvert", "fake_audio_convert");
//             fake_audio_resample_ = gst_element_factory_make("audioresample", "fake_audio_resample");
//             g_object_set(fake_audio_sink_, "sync", FALSE, nullptr);
//             gst_bin_add_many(GST_BIN(Pipeline()),
//                              fake_audio_decodec_,
//                              fake_audio_queue_,
//                              fake_audio_sink_,
//                              fake_audio_convert_,
//                              fake_audio_resample_,
//                              nullptr);
//             gst_element_link_many(fake_audio_queue_,
//                                   fake_audio_decodec_,
//                                   fake_audio_convert_,
//                                   fake_audio_resample_,
//                                   fake_audio_sink_,
//                                   nullptr);
//         } else {
//             fake_audio_sink_ = gst_element_factory_make("fakesink", "fake_audio_sink");
//             g_object_set(fake_audio_sink_, "sync", FALSE, nullptr);
//             gst_bin_add_many(GST_BIN(Pipeline()), fake_audio_queue_, fake_audio_sink_, nullptr);
//             gst_element_link_many(fake_audio_queue_, fake_audio_sink_, nullptr);
//         }


//         g_warn_if_fail(gst_element_link(audio_tee(), fake_audio_queue_));
//         // GstPadTemplate *templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(audio_tee()), "src_%u");
//         // audio_tee_pad_ = gst_element_request_pad(audio_tee(), templ, nullptr, nullptr);
//         // GstPad *sinkpad = gst_element_get_static_pad(fake_audio_queue_, "sink");
//         // g_warn_if_fail(gst_pad_link(audio_tee_pad_, sinkpad) == GST_PAD_LINK_OK);
//         // gst_object_unref(sinkpad);

//         // //monitor data probe
//         // GstPad *pad = gst_element_get_static_pad(audio_depay, "sink");
//         // gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, on_monitor_data, this, nullptr);
//         // gst_object_unref(pad);
//         // g_debug("\n---audio---\n");
//     }
// }