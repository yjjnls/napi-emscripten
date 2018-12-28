#ifndef STREAM_MATRIX_APPLICATION_RTSP_TEST_SERVER_HPP
#define STREAM_MATRIX_APPLICATION_RTSP_TEST_SERVER_HPP

#include <framework/app.hpp>
#include <endpoint/rtsp_server.hpp>

class RtspTestServer : public IApp
{
 public:
    RtspTestServer(const std::string &id, StreamMatrix *instance);
    virtual ~RtspTestServer() {}

    virtual bool Initialize(Promise *promise);
    virtual void Destroy();
    virtual void On(Promise *promise);

    std::string uname() { return "RtspTestServer@" + id(); }

 protected:
    void startup(Promise *promise);
    void stop(Promise *promise);

 private:
    RtspServer *server_;
    int port_;
    std::string path_;
    std::string launch_;
};

#endif