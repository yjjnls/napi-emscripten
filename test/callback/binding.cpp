
#include <functional>
#include <emscripten/bind.h>

using namespace emscripten;


namespace binding_utils {
#ifdef NAPI_VERSION
template <typename T>
T specialization(const emscripten::val &callback)
{
    return callback.functor<T>();
}
#else
template <typename T>
emscripten::val specialization(const emscripten::val &callback)
{
    return callback;
}
#endif
void increase(int num, std::function<void(int)> cb)
{
    cb(num + 1);
}
void calculate(int param1, const emscripten::val &callback)
{
    increase(param1, specialization<std::function<void(int)>>(callback));
}
}  // namespace binding_utils

EMSCRIPTEN_BINDINGS(binding_utils)
{
    function("calculate", select_overload<void(int, const emscripten::val &)>(&binding_utils::calculate));
}