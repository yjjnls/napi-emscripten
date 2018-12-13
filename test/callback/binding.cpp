
#include <algorithm>
#include <emscripten/bind.h>

using namespace emscripten;


namespace binding_utils {
void increase(int num, std::function<void(int)> cb)
{
    cb(num + 1);
}
void calculate(int param1, const emscripten::val &callback)
{
    // std::function<void(int)> cb = callback;
    increase(param1, callback);
    // callback(param1);
}
}  // namespace binding_utils

EMSCRIPTEN_BINDINGS(binding_utils)
{
    function("calculate", select_overload<void(int, const emscripten::val &)>(&binding_utils::calculate));
}