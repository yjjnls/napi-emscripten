#include <app/launcher.hpp>

using json = nlohmann::json;

GST_DEBUG_CATEGORY_STATIC(my_category);
#define GST_CAT_DEFAULT my_category

Launcher::Launcher(const std::string &id,
                   StreamMatrix *instance)
    : IApp(id, instance)
{
    GST_DEBUG_CATEGORY_INIT(my_category, "stream_matrix", 2, "stream_matrix");
}

bool Launcher::Initialize(Promise *promise)
{
    const json &data = promise->data();
    launch_ = data["launch"];
    GError *error = nullptr;
    GstElement *bin = gst_parse_launch(launch_.c_str(), &error);
    if (error) {
        g_printerr("Failed to parse launch: %s\n", error->message);
        g_error_free(error);
        return false;
    }
    IApp::Initialize(promise);
    gst_bin_add(GST_BIN(Pipeline()), bin);
    return true;
}

void Launcher::On(Promise *promise)
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

void Launcher::startup(Promise *promise)
{
    GstBus *bus = gst_element_get_bus(Pipeline());
    gst_bus_add_watch(bus, Launcher::message_handler, this);
    gst_object_unref(bus);

    IApp::startup(promise);
}

// void Launcher::stop(Promise *promise)
// {
//     gst_element_set_state(Pipeline(), GST_STATE_NULL);
//     promise->resolve();
// }

gboolean Launcher::message_handler(GstBus *bus, GstMessage *message, gpointer data)
{
    Launcher *instance = static_cast<Launcher *>(data);
    instance->on_message(bus, message);

    return TRUE;
}

void Launcher::on_message(GstBus *bus, GstMessage *message)
{
}