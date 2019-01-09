#include <app/webrtc_analyzer.hpp>

using json = nlohmann::json;

GST_DEBUG_CATEGORY_STATIC(my_category);
#define GST_CAT_DEFAULT my_category


WebrtcAnalyzer::WebrtcAnalyzer(const std::string &id,
                               StreamMatrix *instance)
    : Analyzer(id, instance)
    , webrtc_ep_(nullptr)
{
    GST_DEBUG_CATEGORY_INIT(my_category, "stream_matrix", 2, "stream_matrix");
}

WebrtcAnalyzer::~WebrtcAnalyzer()
{
}

bool WebrtcAnalyzer::Initialize(Promise *promise)
{
    if (webrtc_ep_ == nullptr) {
        webrtc_ep_ = new Webrtc(this, "analyzer");
    }
    return webrtc_ep_->Initialize(promise);
}
void WebrtcAnalyzer::Destroy()
{
    if (webrtc_ep_ != nullptr) {
        delete webrtc_ep_;
        webrtc_ep_ = NULL;
    }
}
void WebrtcAnalyzer::On(Promise *promise)
{
    const json &j = promise->meta();
    std::string action = j["action"];
    if (action == "startup") {
        startup(promise);
    } else if (action == "stop") {
        stop(promise);
    } else if (action == "remote_sdp") {
        set_remote_description(promise);
    } else if (action == "remote_candidate") {
        set_remote_candidate(promise);
    }
}

void WebrtcAnalyzer::startup(Promise *promise)
{
    Analyzer::startup(promise);
}

void WebrtcAnalyzer::stop(Promise *promise)
{
    if (webrtc_ep_) {
        webrtc_ep_->Terminate();
        promise->resolve();
        return;
    }
}
void WebrtcAnalyzer::set_remote_description(Promise *promise)
{
    if (webrtc_ep_) {
        webrtc_ep_->SetRemoteDescription(promise);
        promise->resolve();
        return;
    }
    promise->reject("WebrtcAnalyzer set_remote_description failed!");
}
void WebrtcAnalyzer::set_remote_candidate(Promise *promise)
{
    if (webrtc_ep_) {
        webrtc_ep_->SetRemoteCandidate(promise);
        promise->resolve();
        return;
    }
    promise->reject("WebrtcAnalyzer set_remote_candidate failed!");
}