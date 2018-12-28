#include <framework/app.hpp>
#include <stream_matrix.hpp>

bool IApp::Initialize(Promise *promise)
{
    if (!pipeline_) {
        pipeline_ = gst_pipeline_new((const gchar *)uname().c_str());
    }

    if (pipeline_) {
        return true;
    } else {
        return false;
    }
}


void IApp::Destroy()
{
    if (pipeline_) {
        gst_element_set_state(pipeline_, GST_STATE_NULL);
        g_object_unref(pipeline_);
        pipeline_ = nullptr;
    }
    stream_matrix_ = nullptr;
}


void IApp::Notify(const nlohmann::json &data, const nlohmann::json &meta)
{
    stream_matrix_->Notify(meta, data);
}