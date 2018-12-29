#ifndef STREAM_MATRIX_ENDPOINT_RTSP_SERVER_HPP
#define STREAM_MATRIX_ENDPOINT_RTSP_SERVER_HPP

#include <framework/endpoint.hpp>
#include <gst/rtsp-server/rtsp-server.h>
#include <gst/rtsp-server/rtsp-session-pool.h>


class RtspServer : public IEndpoint
{
 public:
    RtspServer(IApp *app, const std::string &id);
    ~RtspServer();

    bool Initialize(Promise *promise);
    void Terminate();

    ////////////////////////////////////////////////////////
    void Prepare(int port = 554);
    bool StartLaunch(const std::string &path,
                     const std::string &launch,
                     GCallback media_constructed,
                     GCallback media_configure);
    void StopLaunch();

 private:
    static void on_rtsp_media_constructed(GstRTSPMediaFactory *factory,
                                          GstRTSPMedia *media,
                                          gpointer user_data);
    static void on_client_connected(GstRTSPServer *gstrtspserver,
                                    GstRTSPClient *client,
                                    gpointer user_data);
    static void on_new_session(GstRTSPClient *gstrtspclient,
                               GstRTSPSession *session,
                               gpointer user_data);
    static void on_closed(GstRTSPClient *client,
                          gpointer user_data);


    GstRTSPSessionPool *rtsp_session_pool_;
    int port_;
    std::string path_;
    ServerType type_;
    GstRTSPServer *server_;
    std::string launch_;
    std::map<GstRTSPSession *, GstRTSPClient *> clients_;
    static std::mutex client_mutex_;
    GstRTSPMediaFactory *factory_;
};

#endif