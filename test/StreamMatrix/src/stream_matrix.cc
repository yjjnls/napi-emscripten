#include <stream_matrix.hpp>

using json = nlohmann::json;

static GThread *main_thread = NULL;

GMainLoop *StreamMatrix::main_loop_ = nullptr;
GMainContext *StreamMatrix::main_context_ = nullptr;


GST_DEBUG_CATEGORY_STATIC(my_category);
#define GST_CAT_DEFAULT my_category

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
    GST_DEBUG_CATEGORY_INIT(my_category, "webstreamer", 2, "libWebStreamer");
    gst_init(NULL, NULL);
    GAsyncQueue *queue = g_async_queue_new();

    Promise *promise = new Promise(this, cb, queue);

    main_thread = g_thread_new("webstreamer_main_loop",
                               (GThreadFunc)main_loop_entry,
                               promise);

    g_async_queue_pop(queue);
}

gpointer StreamMatrix::main_loop_entry(Promise *promise)
{
    StreamMatrix *instance = promise->GetStreamMatrix();
    instance->main_loop(promise);
    return NULL;
}

void StreamMatrix::main_loop(Promise *promise)
{
    StreamMatrix::main_context_ = g_main_context_new();
    StreamMatrix::main_loop_ = g_main_loop_new(StreamMatrix::main_context_, FALSE);
    g_main_context_push_thread_default(StreamMatrix::main_context_);

    GAsyncQueue *queue = (GAsyncQueue *)promise->user_data();

    promise->resolve();
    delete promise;
    promise = NULL;

    GSource *idle_source = g_idle_source_new();
    g_source_set_callback(idle_source,
                          (GSourceFunc)main_loop_init,
                          queue,
                          NULL);
    g_source_set_priority(idle_source, G_PRIORITY_DEFAULT);
    g_source_attach(idle_source, StreamMatrix::main_context_);

    g_main_loop_run(StreamMatrix::main_loop_);


    cleanup();
    g_main_context_pop_thread_default(StreamMatrix::main_context_);

    g_main_loop_unref(StreamMatrix::main_loop_);

    gst_deinit();
    StreamMatrix::main_loop_ = NULL;
    StreamMatrix::main_context_ = NULL;
    terminate_promise_->resolve();
    delete terminate_promise_;
    terminate_promise_ = NULL;
}

void StreamMatrix::cleanup()
{
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
void StreamMatrix::Call(const nlohmann::json &meta, const nlohmann::json &data, callback cb)
{
    Promise *promise = new Promise(this, cb, nullptr, meta, data);
    GSource *source = g_idle_source_new();

    g_source_set_callback(source, StreamMatrix::on_promise_entry, promise, NULL);
    g_source_set_priority(source, G_PRIORITY_DEFAULT);
    g_source_attach(source, StreamMatrix::main_context_);
}

gboolean StreamMatrix::on_promise_entry(gpointer user_data)
{
    Promise *promise = (Promise *)user_data;
    StreamMatrix *instance = promise->GetStreamMatrix();
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
}
void StreamMatrix::destroy_app(Promise *promise)
{
}
void StreamMatrix::operate_app(Promise *promise)
{
}