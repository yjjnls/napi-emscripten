#include <app/rtsp_test_server.hpp>

using json = nlohmann::json;

GST_DEBUG_CATEGORY_STATIC(my_category);
#define GST_CAT_DEFAULT my_category


RtspTestServer::RtspTestServer(const std::string &id, StreamMatrix *instance)
    : IApp(id, instance)
    , server_(nullptr)
    , port_(-1)
{
    GST_DEBUG_CATEGORY_INIT(my_category, "stream_matrix", 2, "stream_matrix");
}

bool RtspTestServer::Initialize(Promise *promise)
{
    if (server_ == nullptr) {
        server_ = new RtspServer(this, "default_server");
    }
    const json &j = promise->data();

    port_ = j["port"];
    path_ = j["path"];
    launch_ = j["launch"];
    server_->Prepare(port_);

    return true;
}
void RtspTestServer::Destroy()
{
    if (server_ != nullptr) {
        delete server_;
        server_ = nullptr;
    }
}
void RtspTestServer::On(Promise *promise)
{
    const json &j = promise->meta();
    std::string action = j["action"];
    if (action == "startup") {
        startup(promise);
    } else if (action == "stop") {
        stop(promise);
    }
    promise->reject("unkown action: " + action);
}

void RtspTestServer::startup(Promise *promise)
{
    server_->StartLaunch(path_, launch_, nullptr, nullptr);
    promise->resolve();
    GST_DEBUG("[RtspTestServer] %s startup!", uname().c_str());
}
void RtspTestServer::stop(Promise *promise)
{
    server_->StopLaunch();
    promise->resolve();
    GST_DEBUG("[RtspTestServer] %s stop!", uname().c_str());
}