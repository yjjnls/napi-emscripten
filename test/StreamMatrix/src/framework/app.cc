#include <framework/app.hpp>
#include <stream_matrix.hpp>

GST_DEBUG_CATEGORY_STATIC(my_category);
#define GST_CAT_DEFAULT my_category

bool IApp::Initialize(Promise *promise)
{
    GST_DEBUG_CATEGORY_INIT(my_category, "stream_matrix", 2, "stream_matrix");
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


void IApp::Notify(const nlohmann::json &meta, const nlohmann::json &data)
{
    stream_matrix_->Notify(meta, data);
}
void IApp::startup(Promise *promise)
{
    gst_element_set_state(Pipeline(), GST_STATE_PLAYING);
    promise->resolve();
    GST_DEBUG("%s startup!", uname().c_str());
}
void IApp::stop(Promise *promise)
{
    gst_element_set_state(Pipeline(), GST_STATE_NULL);
    promise->resolve();
    GST_DEBUG("%s stop!", uname().c_str());
}