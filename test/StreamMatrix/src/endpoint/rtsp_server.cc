#include <endpoint/rtsp_server.hpp>
#include <stream_matrix.hpp>

using json = nlohmann::json;

GST_DEBUG_CATEGORY_STATIC(my_category);
#define GST_CAT_DEFAULT my_category

std::mutex RtspServer::client_mutex_;

RtspServer::RtspServer(IApp *app, const std::string &id)
    : IEndpoint(app, id)
    , rtsp_session_pool_(nullptr)
    , port_(-1)
    , factory_(nullptr)
{
    GST_DEBUG_CATEGORY_INIT(my_category, "stream_matrix", 2, "stream_matrix");

    gint max_sessions = 100;
    rtsp_session_pool_ = gst_rtsp_session_pool_new();
    gst_rtsp_session_pool_set_max_sessions(rtsp_session_pool_, max_sessions);
    server_ = gst_rtsp_server_new();
    gst_rtsp_server_set_session_pool(server_, rtsp_session_pool_);
}
RtspServer::~RtspServer()
{
    if (server_) {
        g_object_unref(server_);
        server_ = nullptr;
    }
    if (rtsp_session_pool_) {
        g_object_unref(rtsp_session_pool_);
        rtsp_session_pool_ = nullptr;
    }
}
bool RtspServer::Initialize(Promise *promise)
{
    const json &j = promise->data();
    port_ = j["port"];
    type_ = j["protocal"];
    IEndpoint::Protocol() = "rtspserver";

    Prepare(port_);

    return StartLaunch(j["path"],
                       j["launch"],
                       (GCallback)RtspServer::on_rtsp_media_constructed,
                       nullptr);
}
void RtspServer::Prepare(int port)
{
    gchar *service = g_strdup_printf("%d", port);
    gst_rtsp_server_set_service(server_, service);
    g_free(service);

    gst_rtsp_server_attach(server_, StreamMatrix::MainContext());
}
bool RtspServer::StartLaunch(const std::string &path,
                             const std::string &launch,
                             GCallback media_constructed,
                             GCallback media_configure)
{
    path_ = path;
    launch_ = launch;
    GstRTSPMountPoints *mount_points = gst_rtsp_server_get_mount_points(server_);

    factory_ = gst_rtsp_media_factory_new();
    // if you want multiple clients to see the same video,
    // set the shared property to TRUE
    gst_rtsp_media_factory_set_shared(factory_, TRUE);

    gst_rtsp_media_factory_set_launch(factory_, launch.c_str());
    if (media_constructed) {
        g_signal_connect(factory_, "media-constructed", (GCallback)media_constructed, (gpointer)(this));
    }

    if (media_configure) {
        g_signal_connect(factory_, "media-configure", (GCallback)media_configure, (gpointer)(this));
    }

    gst_rtsp_mount_points_add_factory(mount_points, path.c_str(), factory_);
    g_object_unref(mount_points);

    GST_DEBUG("[RtspServer] \"%s\" launched to %s", id().c_str(), path.c_str());
    // g_object_weak_ref(G_OBJECT(factory_), Notify, factory_);

    g_signal_connect(server_, "client-connected", (GCallback)on_client_connected, (gpointer)(this));

    GST_DEBUG("[RtspServer] (path: %s) initialize done.", path.c_str());

    return true;
}

/////////////////////////////////////////////////////////////
GstPadProbeReturn cb_have_data(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    printf("-");
    return GST_PAD_PROBE_OK;
}
void RtspServer::on_rtsp_media_constructed(GstRTSPMediaFactory *factory, GstRTSPMedia *media, gpointer user_data)
{
    RtspServer *instance = static_cast<RtspServer *>(user_data);
    GST_DEBUG("[RtspServer] (path: %s) on_rtsp_media_constructed", instance->path_.c_str());
    GstElement *rtsp_server_media_bin = gst_rtsp_media_get_element(media);

    GstRTSPStream *gstrtspstream = gst_rtsp_media_get_stream(media, 0);

    gboolean sync = TRUE;
    g_object_get(G_OBJECT(gstrtspstream), "sink-false", &sync, nullptr);
    if (!sync) {
        GST_FIXME("[RtspServer] (path: %s) uses the fixed plugin.", instance->path_.c_str());
    } else {
        GST_FIXME("[RtspServer] (path: %s) uses the origin plugin.", instance->path_.c_str());
    }
    g_object_set(G_OBJECT(gstrtspstream), "sink-false", TRUE, nullptr);

    static int session_count = 0;
    if (!instance->app()->VideoEncoding().empty()) {
        GST_DEBUG("[RtspServer] (path: %s) media constructed: video", instance->path_.c_str());

        static std::string media_type = "video";
        std::string pipejoint_name = std::string("rtspserver_video_endpoint_joint_") +
                                     instance->id() +
                                     std::to_string(session_count);
        instance->video_output_joint() = make_pipe_joint(media_type, pipejoint_name);

        instance->app()->AddPipeJoint(instance->video_output_joint().upstream_joint, nullptr);

        g_warn_if_fail(gst_bin_add(GST_BIN(rtsp_server_media_bin), instance->video_output_joint().downstream_joint));

        GstElement *video_pay = gst_bin_get_by_name_recurse_up(GST_BIN(rtsp_server_media_bin), "pay0");
        g_warn_if_fail(gst_element_link(instance->video_output_joint().downstream_joint, video_pay));

        GstPad *pad = gst_element_get_static_pad(video_pay, "src");
        // gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, cb_have_data, user_data, nullptr);
        gst_object_unref(pad);
    }

    if (!instance->app()->AudioEncoding().empty()) {
        GST_DEBUG("[RtspServer] (path: %s) media constructed: audio", instance->path_.c_str());

        static std::string media_type = "audio";
        std::string pipejoint_name = std::string("rtspserver_audio_endpoint_joint_") +
                                     instance->id() +
                                     std::to_string(session_count);
        instance->audio_output_joint() = make_pipe_joint(media_type, pipejoint_name);

        instance->app()->AddPipeJoint(instance->audio_output_joint().upstream_joint, nullptr);

        g_warn_if_fail(gst_bin_add(GST_BIN(rtsp_server_media_bin), instance->audio_output_joint().downstream_joint));

        GstElement *audio_pay = gst_bin_get_by_name_recurse_up(GST_BIN(rtsp_server_media_bin), "pay1");
        g_warn_if_fail(gst_element_link(instance->audio_output_joint().downstream_joint, audio_pay));

        // GstPad *pad = gst_element_get_static_pad(audio_pay, "src");
        // gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, instance->cb_have_data, user_data, nullptr);
        // gst_object_unref(pad);
    }
    session_count++;
}
/////////////////////////////////////////////////////////////

void RtspServer::on_new_session(GstRTSPClient *client,
                                GstRTSPSession *session,
                                gpointer user_data)
{
    RtspServer *instance = static_cast<RtspServer *>(user_data);
    char *str;
    g_object_get(G_OBJECT(client), "path", &str, nullptr);
    std::string path(str);
    if (path != instance->path_)
        return;
    // std::string sessid(gst_rtsp_session_get_sessionid(session));
    // printf("RtspServer sessionid:%s\n", sessid.c_str());

    client_mutex_.lock();
    instance->clients_[session] = client;
    client_mutex_.unlock();
    g_signal_connect(client, "closed", (GCallback)(instance->onclosed), user_data);
    GST_DEBUG("[RtspServer] (path: %s) client: %p connected.", instance->path_.c_str(), client);
}
void RtspServer::onclosed(GstRTSPClient *client, gpointer user_data)
{
    RtspServer *instance = static_cast<RtspServer *>(user_data);
    client_mutex_.lock();
    auto it = std::find_if(instance->clients_.begin(),
                           instance->clients_.end(),
                           [client](std::pair<GstRTSPSession *, GstRTSPClient *> curr_client) {
                               return (curr_client.second == client);
                           });
    if (it != instance->clients_.end()) {
        instance->clients_.erase(it);
        GST_DEBUG("[RtspServer] (path: %s) client: %p closed and removed.", instance->path_.c_str(), client);
    }
    client_mutex_.unlock();
}

void RtspServer::on_client_connected(GstRTSPServer *gstrtspserver,
                                     GstRTSPClient *client,
                                     gpointer user_data)
{
    RtspServer *instance = static_cast<RtspServer *>(user_data);
    g_signal_connect(client, "new-session", (GCallback)(instance->on_new_session), user_data);
}

/////////////////////////////////////////////////////////////

void RtspServer::StopLaunch()
{
    client_mutex_.lock();
    std::map<GstRTSPSession *, GstRTSPClient *> client_info;
    client_info.swap(clients_);
    client_mutex_.unlock();

    if (!client_info.empty()) {
        for (auto client : client_info) {
            // There's no bug here now, it will invoke 'gst_rtsp_client_finalize' firstly
            // and do 'g_main_context_unref (priv->watch_context)'; while 'gst_rtsp_client_close'
            // will unref the context again. The correct invoke order should be that invoke
            // 'gst_rtsp_client_finalize'(automaticlly) after we invoke `gst_rtsp_client_close`
            // seems like the bug below
            // https://bugzilla.gnome.org/show_bug.cgi?id=790909
            GST_DEBUG("[RtspServer] (path: %s) start close client: %p actively.",
                      path_.c_str(),
                      client.second);
            g_object_set(G_OBJECT(client.second), "path", (gchar *)path_.c_str(), "close", client.first, nullptr);

            GST_INFO("[RtspServer] (path: %s) close client: %p actively.",
                     path_.c_str(),
                     client.second);
        }
    }
    if (factory_) {
        GstRTSPMountPoints *mount_points =
            gst_rtsp_server_get_mount_points(server_);
        gst_rtsp_mount_points_remove_factory(mount_points, path_.c_str());
        g_object_unref(mount_points);

        factory_ = nullptr;
        GST_DEBUG("[RtspServer] (path: %s) terminate done.", path_.c_str());
    }
}

void RtspServer::Terminate()
{
    // dynamicly unlink
    if (!app()->VideoEncoding().empty() &&
        video_output_joint().upstream_joint != nullptr) {
        app()->RemovePipeJoint(video_output_joint().upstream_joint, nullptr);
    }
    if (!app()->AudioEncoding().empty() &&
        audio_output_joint().upstream_joint != nullptr) {
        app()->RemovePipeJoint(audio_output_joint().upstream_joint, nullptr);
    }
    // stop itself
    StopLaunch();
}