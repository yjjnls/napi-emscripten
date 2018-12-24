#ifndef STREAM_MATRIX_HPP
#define STREAM_MATRIX_HPP

#include <framework/common.hpp>
#include <framework/promise.hpp>

class StreamMatrix
{
 public:
    explicit StreamMatrix()
        : terminate_promise_(nullptr){};

    ~StreamMatrix();

    void Initialize(callback cb);
    void Terminate(callback cb);

    void SetNotification(notify_fn fun, callback cb);

    void Call(const nlohmann::json &meta, const nlohmann::json &data, callback cb);


 private:
    void get_version(Promise *promise);
    static GMainLoop *main_loop_;
    static GMainContext *main_context_;
    static gpointer main_loop_entry(Promise *promise);
    void main_loop(Promise *promise);
    void cleanup();
    static gboolean on_promise_entry(gpointer user_data);
    void on_promise(Promise *promise);

    void create_app(Promise *promise);
    void destroy_app(Promise *promise);
    void operate_app(Promise *promise);

    notify_fn notification_;
    Promise *terminate_promise_;
};

#endif