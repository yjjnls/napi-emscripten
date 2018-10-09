from string import Template
bind_gyp = """
{
    "targets": [
        {
            "target_name": "plugin",
            "include_dirs": [
                "include"
            ],
            "libraries": [],
            "sources": ["%s",
                        "%s"],
            "conditions": [
                ["OS==\\"linux\\"", {"cflags": ["-frtti", "-w"],
                                   "cflags_cc": ["-frtti", "-w"]
                                   }]
            ]
        }
    ]
}
"""

bind_cxx_fixed = """\
#include <node_api.h>
%s
using namespace emscripten::internal;

#define NAPI_DECLARE_METHOD(name, func)         \\
    {                                           \\
        name, 0, func, 0, 0, 0, napi_default, 0 \\
    }
#define NAPI_DECLARE_METHOD(name, func)         \\
    {                                           \\
        name, 0, func, 0, 0, 0, napi_default, 0 \\
    }
#define NAPI_ASSERT_BASE(env, assertion, message, ret_val)               \\
  do {                                                                   \\
    if (!(assertion)) {                                                  \\
      napi_throw_error(                                                  \\
          (env),                                                         \\
        NULL,                                                            \\
          "assertion (" #assertion ") failed: " message);                \\
      break;                                                             \\
    }                                                                    \\
  } while (0)

// Returns NULL on failed assertion.
// This is meant to be used inside napi_callback methods.
#define NAPI_ASSERT(env, assertion, message)                             \\
  NAPI_ASSERT_BASE(env, assertion, message, NULL)
extern "C" {
%s
}
"""
register_class = """
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
"""
register_constant = """
    void _embind_register_constant(
        const char *name,
        TYPEID constantType,
        double value){}
"""
register_object = """
    void _embind_register_value_object(
        TYPEID structType,
        const char *fieldName,
        const char *constructorSignature,
        GenericFunction constructor,
        const char *destructorSignature,
        GenericFunction destructor){}

    void _embind_register_value_object_field(
        TYPEID structType,
        const char *fieldName,
        TYPEID getterReturnType,
        const char *getterSignature,
        GenericFunction getter,
        void *getterContext,
        TYPEID setterArgumentType,
        const char *setterSignature,
        GenericFunction setter,
        void *setterContext){}
    
    void _embind_finalize_value_object(TYPEID structType){}
"""
register_array = """
    void _embind_register_value_array(
        TYPEID tupleType,
        const char *name,
        const char *constructorSignature,
        GenericFunction constructor,
        const char *destructorSignature,
        GenericFunction destructor){}
    
    void _embind_register_value_array_element(
        TYPEID tupleType,
        TYPEID getterReturnType,
        const char *getterSignature,
        GenericFunction getter,
        void *getterContext,
        TYPEID setterArgumentType,
        const char *setterSignature,
        GenericFunction setter,
        void *setterContext){}

    void _embind_finalize_value_array(TYPEID tupleType){}
"""
register_val = """
    EM_VAL _emval_take_value(TYPEID type, EM_VAR_ARGS argv){
        return (_EM_VAL *)argv;
    }
    void _emval_decref(EM_VAL value){}
"""
class_declaration = Template("""
/////////////////////////////////${jstype}///////////////////////////////////////
class ${name}
{
 public:
    ${name}()
        : target_(nullptr)
        , env_(nullptr)
        , wrapper_(nullptr) {}
    ~${name}()
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
    static napi_ref cons_ref();
    ${type} *target() { return target_; }
    void update_target(const ${type} &t) {
        if (target_ != nullptr) {
            delete target_;
            target_ = nullptr;
        }
        target_ = new ${type}(t);
    }

    ////////////////////////////////////////////////////////////////////////
${function}
${property}
${class_function}
    ////////////////////////////////////////////////////////////////////////

 private:
    static napi_value Release(napi_env env, napi_callback_info info);
    static napi_value New(napi_env env, napi_callback_info info);

    static napi_ref constructor_;
    ${type} *target_;
    napi_env env_;
    napi_ref wrapper_;
};
""")

fixed_class_function = Template("""
napi_ref ${name}::constructor_;
napi_ref ${name}::cons_ref()
{
    return constructor_;
}
void ${name}::Destructor(napi_env env, void *nativeObject, void *finalize_hint)
{
    printf("~~~~~~~  ${jstype} Destructor  ~~~~~~~~\\n");
    ${name} *obj = static_cast<${name} *>(nativeObject);
    delete obj;
}
napi_value ${name}::Release(napi_env env, napi_callback_info info)
{
    return nullptr;
}
napi_value ${name}::CreateObject(napi_env env, napi_callback_info info)
{
    napi_value instance;
    ${name}::NewInstance(env, info, &instance);

    return instance;
}
napi_value ${name}::New(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value _this;
    napi_get_cb_info(env, info, &argc, nullptr, &_this, nullptr);
    napi_value args[argc];
    napi_get_cb_info(env, info, &argc, args, &_this, nullptr);

    ${name} *obj = new ${name}();
    ${type} *target = ${name}_constructor_factory(env, argc, args);

    obj->env_ = env;
    obj->target_ = target;
    napi_wrap(env, _this, obj, ${name}::Destructor,
              nullptr,  // finalize_hint
              &obj->wrapper_);

    return _this;
}
napi_status ${name}::NewInstance(napi_env env, napi_callback_info info, napi_value *instance)
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
void ${name}::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor properties[] = {
        //property
${declare_property}
        //function(return type, arguments, function name)
${declare_function}
        NAPI_DECLARE_METHOD("delete", Release)
    };

    napi_value cons;
    napi_define_class(env,
                      "${jstype}",
                      -1,
                      New,
                      nullptr,
                      sizeof(properties) / sizeof(properties[0]),
                      properties,
                      &cons);

    napi_create_reference(env, cons, 1, &constructor_);

    napi_set_named_property(env, exports, "${jstype}", cons);
}
/////////////////////////////////${jstype}///////////////////////////////////////
""")
constant_func = """
napi_value %s(napi_env env, napi_callback_info info)
{
    napi_value result;
    napi_create_int32(env, %s, &result);
    return result;
}
"""
napi_init = Template("""
napi_value Init(napi_env env, napi_value exports)
{
${init}
    napi_property_descriptor desc[] = {
${declaration}
${create_object}
    };

    napi_define_properties(env,
                           exports,
                           sizeof(desc) / sizeof(*desc),
                           desc);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
""")

constructor_func_start = Template("""
${type} *${name}_constructor_factory(napi_env env, size_t argc, napi_value *args)
{
 ${type} *p = nullptr;
 switch (argc) {
""")
constructor_func_end = """
 }
 return p;
}
"""


args_int = """\
    // arg{0}
    int32_t arg{0} = 0;
    napi_get_value_int32(env, args[{0}], &arg{0});
"""
args_double = """\
    // arg{0}
    double arg{0} = 0;
    napi_get_value_double(env, args[{0}], &arg{0});
"""
args_string = """\
    // arg{0}
    size_t strlen;
    napi_get_value_string_utf8(env, args[{0}], NULL, 0, &strlen);
    std::string arg{0}(strlen + 1, 0);
    size_t res;
    napi_get_value_string_utf8(env, args[{0}], (char *)arg{0}.c_str(), strlen + 1, &res);
"""
args_cxxtype = """\
    // arg{0}
    %s *p{0} = nullptr;
    napi_unwrap(env, args[{0}], reinterpret_cast<void **>(&p{0}));
    %s &arg{0} = *(p{0}->target());
"""
args_obj = """\
    // arg{0}
    napi_valuetype valuetype{0};
    napi_typeof(env, args[{0}], &valuetype{0});
    NAPI_ASSERT(env, valuetype{0} == napi_object, "Passing arg is not object!");
    napi_value cons{0};
    napi_get_reference_value(env, %s::cons_ref(), &cons{0});
    napi_value value{0};
    napi_new_instance(env, cons{0}, 0, nullptr, &value{0});
    %s *p{0} = nullptr;
    napi_unwrap(env, value{0}, reinterpret_cast<void **>(&p{0}));
%s    %s &arg{0} = *(p{0}->target());
"""
args_array = """\
    // arg{0}
    napi_value array{0} = args[{0}];
    bool isarray{0};
    napi_is_array(env, array{0}, &isarray{0});
    NAPI_ASSERT(env, isarray{0}, "Passing arg is not array!");
    uint32_t length{0};
    napi_get_array_length(env, array{0}, &length{0});
%s    %s &arg{0} = *new %s(%s);
"""

arr_args_double = """\
    napi_value ret{0}_%s;
    napi_get_element(env, array{0}, %s, &ret{0}_%s);
    double arg{0}_%s = 0;
    napi_get_value_double(env, ret{0}_%s, &arg{0}_%s);
"""


return_void = """
    return nullptr;
"""
return_bool = """
    napi_value result;
    napi_get_boolean(env, res, &result);
    return result;
"""
return_int = """
    napi_value result;
    napi_create_int32(env, res, &result);
    return result;
"""
return_uint = """
    napi_value result;
    napi_create_uint32(env, res, &result);
    return result;
"""
return_double = """
    napi_value result;
    napi_create_double(env, res, &result);
    return result;
"""
return_string = """
    napi_value result;
    napi_create_string_utf8(env, res.c_str(), res.size(), &result);
    return result;
"""
return_obj = """
    napi_value cons;
    napi_get_reference_value(env, %scons_ref(), &cons);
    napi_value result;
    napi_new_instance(env, cons, 0, nullptr, &result);
    %s *p;
    napi_unwrap(env, result, reinterpret_cast<void **>(&p));
    p->update_target(res);
    return result;
"""
return_val = """
    EM_VAR_ARGS *handle = (EM_VAR_ARGS *)res.get_handle();
    typedef std::array<GenericWireType, PackSize<%s>::value> arr;
    arr &elements = *(arr *)handle;
    GenericWireType *cursor = &elements[0];
    size_t array_length = cursor->w[0].u;
    %s *array_data = (%s *)cursor->w[1].p;

    napi_value result;
    napi_create_array(env, &result);
    for (int i = 0; i < array_length; i++) {
        napi_value value;
        %s(env, array_data[i], &value);
        napi_set_element(env, result, i, value);
    }
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
func_template = Template("""
napi_value ${name}::${fun_name}(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value _this;
    napi_get_cb_info(env, info, &argc, nullptr, &_this, nullptr);
    napi_value args[argc];
    napi_get_cb_info(env, info, &argc, args, &_this, nullptr);

    ${name} *obj;
    napi_unwrap(env, _this, reinterpret_cast<void **>(&obj));

    ////////////////////////////////////////////////////////////////////////
    ${type} *target = obj->target_;
    ${return_res}fun_${fun_name}_factory(target, env, argc, args);
    ////////////////////////////////////////////////////////////////////////
${return_val}
}
""")

prop_getter = Template("""
napi_value ${name}::${fun_name}(napi_env env, napi_callback_info info)
{
    napi_value _this;
    env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr);

    ${name} *obj;
    napi_unwrap(env, _this, reinterpret_cast<void **>(&obj));

    ////////////////////////////////////////////////////////////////////////
    ${type} *target = obj->target_;
    ${return_fun}
    ////////////////////////////////////////////////////////////////////////
${return_val}
}
""")

prop_setter = Template("""
napi_value ${name}::${fun_name}(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1];
    napi_value _this;
    napi_get_cb_info(env, info, &argc, args, &_this, nullptr);

    ${name} *obj;
    napi_unwrap(env, _this, reinterpret_cast<void **>(&obj));

    ////////////////////////////////////////////////////////////////////////
    ${type} *target = obj->target_;
${res}
${fun}
    ////////////////////////////////////////////////////////////////////////

    return nullptr;
}
""")
