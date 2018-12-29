#ifndef STREAM_MATRIX_APPLICATION_LAUNCHER_HPP
#define STREAM_MATRIX_APPLICATION_LAUNCHER_HPP

#include <framework/app.hpp>

class Launcher : public IApp
{
 public:
    Launcher(const std::string &id, StreamMatrix *instance);

    virtual ~Launcher() {}

    virtual bool Initialize(Promise *promise);
    virtual void On(Promise *promise);
    // virtual bool Destroy();

 protected:
    virtual std::string uname() { return "Launcher@" + id(); }
    virtual void startup(Promise *promise);
    static gboolean message_handler(GstBus *bus, GstMessage *message, gpointer data);
    virtual void on_message(GstBus *bus, GstMessage *message);

 private:
    std::string launch_;
};

#endif