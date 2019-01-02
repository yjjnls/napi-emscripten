#include <stream_matrix.hpp>
#include <app/launcher.hpp>
#include <app/rtsp_test_client.hpp>
#include <app/rtsp_test_server.hpp>
#include <app/livestream.hpp>

using json = nlohmann::json;

static GThread *main_thread = nullptr;

GMainLoop *StreamMatrix::main_loop_ = nullptr;
GMainContext *StreamMatrix::main_context_ = nullptr;


GST_DEBUG_CATEGORY_STATIC(my_category);
#define GST_CAT_DEFAULT my_category

GMainContext *StreamMatrix::MainContext()
{
    return StreamMatrix::main_context_;
}

static gboolean main_loop_init(GAsyncQueue *queue)
{
    g_async_queue_push(queue, g_strdup("ready"));
    return G_SOURCE_REMOVE;
}

StreamMatrix::~StreamMatrix()
{
    {
        printf("destroy StreamMatrix\n");
        if (terminate_promise_ != nullptr) {
            delete terminate_promise_;
            terminate_promise_ = nullptr;
        }
    }
}
void StreamMatrix::Initialize(callback cb)
{
    GST_DEBUG_CATEGORY_INIT(my_category, "stream_matrix", 2, "stream_matrix");
    gst_init(nullptr, nullptr);
    GAsyncQueue *queue = g_async_queue_new();

    Promise *promise = new Promise(this, cb, queue);

    main_thread = g_thread_new("webstreamer_main_loop",
                               (GThreadFunc)main_loop_entry,
                               promise);

    g_async_queue_pop(queue);
}

gpointer StreamMatrix::main_loop_entry(Promise *promise)
{
    StreamMatrix *instance = promise->stream_matrix();
    instance->main_loop(promise);
    return nullptr;
}

void StreamMatrix::main_loop(Promise *promise)
{
    StreamMatrix::main_context_ = g_main_context_new();
    StreamMatrix::main_loop_ = g_main_loop_new(StreamMatrix::main_context_, FALSE);
    g_main_context_push_thread_default(StreamMatrix::main_context_);

    GAsyncQueue *queue = (GAsyncQueue *)promise->user_data();

    promise->resolve();
    delete promise;
    promise = nullptr;

    GSource *idle_source = g_idle_source_new();
    g_source_set_callback(idle_source,
                          (GSourceFunc)main_loop_init,
                          queue,
                          nullptr);
    g_source_set_priority(idle_source, G_PRIORITY_DEFAULT);
    g_source_attach(idle_source, StreamMatrix::main_context_);

    g_main_loop_run(StreamMatrix::main_loop_);


    cleanup();
    g_main_context_pop_thread_default(StreamMatrix::main_context_);
    
    GST_INFO("[StreamMatrix] initialize done!");
    g_main_loop_unref(StreamMatrix::main_loop_);

    gst_deinit();
    StreamMatrix::main_loop_ = nullptr;
    StreamMatrix::main_context_ = nullptr;
    terminate_promise_->resolve();
    delete terminate_promise_;
    terminate_promise_ = nullptr;
    GST_INFO("[StreamMatrix] termiante done!");
}

void StreamMatrix::cleanup()
{
    for (auto &app : app_container_) {
        GST_DEBUG("[StreamMatrix] clean up app: %s", app.second->uname().c_str());
        app.second->Destroy();
        delete app.second;
    }
}

void StreamMatrix::Terminate(callback cb)
{
    Promise *promise = new Promise(this, cb, nullptr);

    terminate_promise_ = promise;
    g_main_loop_quit(main_loop_);
    g_thread_join(main_thread);
}
void StreamMatrix::SetNotification(notify_fn fun, callback cb)
{
    notification_ = fun;
    cb(0, "");
}
void StreamMatrix::Notify(const nlohmann::json &meta, const nlohmann::json &data)
{
    notification_(meta.dump(), data.dump());
}

void StreamMatrix::Call(const nlohmann::json &meta, const nlohmann::json &data, callback cb)
{
    Promise *promise = new Promise(this, cb, nullptr, meta, data);
    GSource *source = g_idle_source_new();

    g_source_set_callback(source, StreamMatrix::on_promise_entry, promise, nullptr);
    g_source_set_priority(source, G_PRIORITY_DEFAULT);
    g_source_attach(source, StreamMatrix::main_context_);
}

gboolean StreamMatrix::on_promise_entry(gpointer user_data)
{
    Promise *promise = (Promise *)user_data;
    StreamMatrix *instance = promise->stream_matrix();
    instance->on_promise(promise);
    return G_SOURCE_REMOVE;
}

void StreamMatrix::on_promise(Promise *promise)
{
    const json &j = promise->meta();

    std::string action = j["action"];
    if (action == "version") {
        get_version(promise);
    } else if (action == "create") {
        create_app(promise);
    } else if (action == "destroy") {
        destroy_app(promise);
    } else {
        operate_app(promise);
    }
    // promise->reject(promise->data());
    delete promise;
}
void StreamMatrix::get_version(Promise *promise)
{
    json data;
    data["version"] = "0.0.1";
    promise->resolve(data);
}
void StreamMatrix::create_app(Promise *promise)
{
    const json &data = promise->data();
    std::string id = data["id"];

    if (app_container_.count(id) > 0) {
        GST_ERROR("[StreamMatrix] %s has been created!", id.c_str());
        promise->reject("[StreamMatrix] " + id + " has been created!");
        return;
    }

    IApp *app = app_factory(data);

    if (app == nullptr) {
        GST_ERROR("[StreamMatrix] %s fail to create!", id.c_str());
        promise->reject("[StreamMatrix] " + id + " fail to create!");
        return;
    }
    if (!app->Initialize(promise)) {
        delete app;
        GST_ERROR("[StreamMatrix] %s initialize failed!", id.c_str());
        promise->reject("[StreamMatrix] " + id + " initialize failed!");
        return;
    }
    app_container_[id] = app;
    GST_INFO("[StreamMatrix] create app: \"%s\" successfully!", app->uname().c_str());
    promise->resolve();
}
IApp *StreamMatrix::app_factory(const nlohmann::json &data)
{
    IApp *app = nullptr;
    std::string id = data["id"];
    int type = data["type"];
    // todo
    if (type == MediaType::kTestServer) {
        app = new RtspTestServer(id, this);
    } else if (type == MediaType::kAnalyzer) {
        int protocal = data["protocal"];
        if (protocal == AnalyzerType::kRtsp) {
            app = new RtspTestClient(id, this);
        }
    } else if (type == MediaType::kLiveStream) {
        app = new LiveStream(id, this);
    } else if (type == MediaType::kMultiPoints) {
    } else if (type == MediaType::kPlayBack) {
    } else if (type == MediaType::kLauncher) {
        app = new Launcher(id, this);
    }

    return app;
}

void StreamMatrix::destroy_app(Promise *promise)
{
    const json &data = promise->data();
    std::string id = data["id"];

    if (app_container_.count(id) <= 0) {
        GST_ERROR("[StreamMatrix] %s hasn't been created!", id.c_str());
        promise->reject("[StreamMatrix] " + id + " hasn't been created!");
        return;
    }

    IApp *app = app_container_[id];
    std::string uname = app->uname();
    app->Destroy();
    delete app;
    app_container_.erase(id);

    GST_INFO("[StreamMatrix] destroy app: \"%s\" successfully!", uname.c_str());
    promise->resolve();
}
void StreamMatrix::operate_app(Promise *promise)
{
    const json &data = promise->data();
    std::string id = data["id"];

    if (app_container_.count(id) <= 0) {
        GST_ERROR("[StreamMatrix] %s hasn't been created!", id.c_str());
        promise->reject("[StreamMatrix] " + id + " hasn't been created!");
        return;
    }

    IApp *app = app_container_[id];

    app->On(promise);
}