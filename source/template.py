bind_gyp = """
{
    "targets": [
        {
            "target_name": "plugin",
            'include_dirs': [
                'include'
            ],
            "sources": ["%s",
                        "%s",
                        "../example.cc"],
            "conditions": [
                ["OS==\\"linux\\"", {"cflags": ["-frtti", "-w"],
                                   "cflags_cc": ["-frtti", "-w"]
                                   }]
            ]
        }
    ]
}
"""

bind_cxx_fixed = """
#include <emscripten/bind.h>
#include <node_api.h>
#include "../example.h"

using namespace emscripten::internal;

#define NAPI_DECLARE_METHOD(name, func)         \\
    {                                           \\
        name, 0, func, 0, 0, 0, napi_default, 0 \\
    }

extern "C" {
void _embind_register_class(
    TYPEID classType,
    TYPEID pointerType,
    TYPEID constPointerType,
    TYPEID baseClassType,
    const char *getActualTypeSignature,
    GenericFunction getActualType,
    const char *upcastSignature,
    GenericFunction upcast,
    const char *downcastSignature,
    GenericFunction downcast,
    const char *className,
    const char *destructorSignature,
    GenericFunction destructor) {}

void _embind_register_class_constructor(
    TYPEID classType,
    unsigned argCount,
    const TYPEID argTypes[],
    const char *invokerSignature,
    GenericFunction invoker,
    GenericFunction constructor) {}

void _embind_register_class_function(
    TYPEID classType,
    const char *methodName,
    unsigned argCount,
    const TYPEID argTypes[],
    const char *invokerSignature,
    GenericFunction invoker,
    void *context,
    unsigned isPureVirtual) {}

void _embind_register_class_property(
    TYPEID classType,
    const char *fieldName,
    TYPEID getterReturnType,
    const char *getterSignature,
    GenericFunction getter,
    void *getterContext,
    TYPEID setterArgumentType,
    const char *setterSignature,
    GenericFunction setter,
    void *setterContext) {}

void _embind_register_class_class_function(
    TYPEID classType,
    const char *methodName,
    unsigned argCount,
    const TYPEID argTypes[],
    const char *invokerSignature,
    GenericFunction invoker,
    GenericFunction method) {}

void _embind_register_class_class_property(
    TYPEID classType,
    const char *fieldName,
    TYPEID fieldType,
    const void *fieldContext,
    const char *getterSignature,
    GenericFunction getter,
    const char *setterSignature,
    GenericFunction setter) {}
}
"""

class_start = """
class Addon
{
 public:
    Addon()
        : target_(nullptr)
        , env_(nullptr)
        , wrapper_(nullptr) {}
    ~Addon()
    {
        napi_delete_reference(env_, wrapper_);
        if (target_ != nullptr) {
            delete target_;
            target_ = nullptr;
        }
    }
    static void Init(napi_env env, napi_value exports);
    static void Destructor(napi_env env, void *nativeObject, void *finalize_hint);
    static napi_status NewInstance(napi_env env, napi_callback_info info, napi_value *instance);
    static napi_value CreateObject(napi_env env, napi_callback_info info);
    %s *target() { return target_; }

 private:
    static napi_value Release(napi_env env, napi_callback_info info);
    static napi_value New(napi_env env, napi_callback_info info);

    ////////////////////////////////////////////////////////////////////////
"""


class_end = """
    ////////////////////////////////////////////////////////////////////////

    static napi_ref constructor_;
    %s *target_;
    napi_env env_;
    napi_ref wrapper_;
};

"""

fixed_class_function = """
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
napi_ref Addon::constructor_;
void Addon::Destructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    printf("~~~~~~~  Destructor  ~~~~~~~~");
    Addon *obj = static_cast<Addon *>(nativeObject);
    delete obj;
}
napi_value Addon::Release(napi_env env, napi_callback_info info)
{
    return nullptr;
}
napi_value Addon::CreateObject(napi_env env, napi_callback_info info)
{
    napi_value instance;
    Addon::NewInstance(env, info, &instance);

    return instance;
}
napi_value Addon::New(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value _this;
    napi_get_cb_info(env, info, &argc, nullptr, &_this, nullptr);
    napi_value args[argc];
    napi_get_cb_info(env, info, &argc, args, &_this, nullptr);

    Addon *obj = new Addon();
    %s *target = constructor_factory(env, argc, args);

    obj->env_ = env;
    obj->target_ = target;
    env, napi_wrap(env, _this, obj, Addon::Destructor,
                   nullptr,  // finalize_hint
                   &obj->wrapper_);

    return _this;
}
napi_status Addon::NewInstance(napi_env env, napi_callback_info info, napi_value *instance)
{
    napi_status status;

    size_t argc = 0;
    napi_value _this;
    napi_get_cb_info(env, info, &argc, nullptr, &_this, nullptr);
    napi_value args[argc];
    assert(napi_get_cb_info(env, info, &argc, args, &_this, nullptr) == napi_ok);

    napi_value cons;
    status = napi_get_reference_value(env, constructor_, &cons);
    if (status != napi_ok)
        return status;

    status = napi_new_instance(env, cons, argc, args, instance);
    if (status != napi_ok)
        return status;

    return napi_ok;
}
"""

napi_init = """
void Addon::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        //property
%s
        //function(return type, arguments, function name)
%s
        NAPI_DECLARE_METHOD("delete", Release)
    };

    napi_value cons;
    napi_define_class(env,
                      "%s",
                      -1,
                      New,
                      nullptr,
                      sizeof(properties) / sizeof(properties[0]),
                      properties,
                      &cons);

    napi_create_reference(env, cons, 1, &constructor_);

    napi_set_named_property(env, exports, "%s", cons);
}
napi_value Init(napi_env env, napi_value exports)
{
    Addon::Init(env, exports);

    napi_property_descriptor desc[] = {
%s
        NAPI_DECLARE_METHOD("createObject", Addon::CreateObject)
    };

    napi_define_properties(env,
                           exports,
                           sizeof(desc) / sizeof(*desc),
                           desc);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
"""

constructor_func_start = """
%s *constructor_factory(napi_env env, size_t argc, napi_value *args)
{
    %s *p = nullptr;
    switch (argc) {
"""
constructor_func_end = """
    }
    return p;
}
"""
args_int = """
            int32_t arg{0} = 0;
            napi_get_value_int32(env, args[{0}], &arg{0});
"""

args_string = """
            size_t strlen;
            napi_get_value_string_utf8(env, args[{0}], NULL, 0, &strlen);
            std::string arg{0}(strlen + 1, 0);
            size_t res;
            napi_get_value_string_utf8(env, args[{0}], (char *)arg{0}.c_str(), strlen + 1, &res);
"""
args_cxxtype = """
            Addon *p;
            napi_unwrap(env, args[{0}], reinterpret_cast<void **>(&p));
            %s &arg{0} = *(p->target());
"""
return_void = """
    return nullptr;
"""
return_external = """
    napi_value result;
    napi_create_external(env, &res, nullptr, nullptr, &result);
    return result;
"""
return_int = """
    napi_value result;
    napi_create_int32(env, res, &result);
    return result;
"""
return_string="""
    napi_value result;
    napi_create_string_utf8(env, res.c_str(), res.size(), &result);
    return result;
"""
function_datail_start = """
%s fun_%s_factory(%s *obj, napi_env env, size_t argc, napi_value *args)
{
    switch (argc) {
"""
function_datail_end = """
    }
}
"""
func_template = """
napi_value Addon::%s(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value _this;
    napi_get_cb_info(env, info, &argc, nullptr, &_this, nullptr);
    napi_value args[argc];
    napi_get_cb_info(env, info, &argc, args, &_this, nullptr);

    Addon *obj;
    napi_unwrap(env, _this, reinterpret_cast<void **>(&obj));

    ////////////////////////////////////////////////////////////////////////
    %s *target = obj->target_;
    %sfun_%s_factory(target, env, argc, args);
    ////////////////////////////////////////////////////////////////////////
    %s
}
"""

getter_int = """
    napi_create_int32(env, %s, &res);
"""

prop_getter = """
napi_value Addon::%s(napi_env env, napi_callback_info info)
{
    napi_value _this;
    env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr);

    Addon *obj;
    napi_unwrap(env, _this, reinterpret_cast<void **>(&obj));

    ////////////////////////////////////////////////////////////////////////
    %s *target = obj->target_;
    napi_value res;
    %s
    ////////////////////////////////////////////////////////////////////////

    return res;
}
"""

setter_int = """
    int32_t value;
    napi_get_value_int32(env, args[0], &value);
"""
prop_setter = """
napi_value Addon::%s(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    napi_value _this;
    napi_get_cb_info(env, info, &argc, args, &_this, nullptr);

    Addon *obj;
    napi_unwrap(env, _this, reinterpret_cast<void **>(&obj));

    ////////////////////////////////////////////////////////////////////////
    %s *target = obj->target_;
%s
%s
    ////////////////////////////////////////////////////////////////////////

    return nullptr;
}
"""

class_function_template_start = """
napi_value %s(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
    napi_value args[argc];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    switch (argc) {
"""
class_function_template_end = """
%s
}
"""
