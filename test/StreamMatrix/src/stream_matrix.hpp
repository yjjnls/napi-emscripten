#ifndef STREAM_MATRIX_HPP
#define STREAM_MATRIX_HPP

#include "framework/common.hpp"

class StreamMatrix
{
 public:
    explicit StreamMatrix();
    ~StreamMatrix();

    void Initialize(callback cb);
    void Terminate(callback cb);

    void SetNotification(notify_fn fun, callback cb);

    void CreateApp(MediaType type, const std::string &id, const std::string &param, callback cb);
    void DestroyApp(const std::string &id, callback cb);

    void OperateApp(const std::string &id, const std::string &param, callback cb);

 private:
    static GMainLoop *main_loop;
    static GMainContext *main_context;
    static gpointer MainloopEntry(Promise *promise);


    notify_fn notification_;
};

#endif