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
            "sources": ["%s"],
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

namespace emscripten {
namespace internal {
template <typename VectorType>
struct VectorAccess
{
    static val get(
        const VectorType &v,
        typename VectorType::size_type index)
    {
        if (index < v.size()) {
            return val(v[index]);
        } else {
            return val::undefined();
        }
    }

    static bool set(
        VectorType &v,
        typename VectorType::size_type index,
        const typename VectorType::value_type &value)
    {
        v[index] = value;
        return true;
    }
};
}  // namespace internal
}  // namespace emscripten
"""
fixed_function = """
namespace emscripten {
namespace internal {
EM_VAL _emval_new_array()
{
    typedef std::vector<napi_value> arr;
    typedef emscripten::memory_view<napi_value> memory_view2arr;
    _EM_VAL *val = new __EM_VAL<arr>(arr());
    val->type = _EMVAL_ARRAY;
    arr &p = static_cast<__EM_VAL<arr> *>(val)->container;

    // handle = std::make_shared<_EM_VAL>(val);
    EM_VAL handle = EM_VAL(val);

    memory_view2arr data(p.size(), (napi_value *)p.data());
    WireTypePack<memory_view2arr> *argv = new WireTypePack<memory_view2arr>(std::forward<memory_view2arr>(data));
    handle->data = argv;

    return handle;
}
static napi_env global_env = nullptr;
template <typename Arg>
napi_value cpp2napi(Arg arg)
{
    printf("cpp2napi type not supported!\\n");
    return nullptr;
}
napi_value cpp2napi()
{
    return nullptr;
}
napi_value cpp2napi(bool arg)
{
    napi_value res;
    napi_get_boolean(global_env, arg, &res);
    return res;
}
napi_value cpp2napi(char arg)
{
    napi_value res;
    napi_create_int32(global_env, (int32_t)arg, &res);
    return res;
}
napi_value cpp2napi(unsigned char arg)
{
    napi_value res;
    napi_create_uint32(global_env, (uint32_t)arg, &res);
    return res;
}
napi_value cpp2napi(short arg)
{
    napi_value res;
    napi_create_int32(global_env, (int32_t)arg, &res);
    return res;
}
napi_value cpp2napi(unsigned short arg)
{
    napi_value res;
    napi_create_uint32(global_env, (uint32_t)arg, &res);
    return res;
}
napi_value cpp2napi(int arg)
{
    napi_value res;
    napi_create_int32(global_env, arg, &res);
    return res;
}
napi_value cpp2napi(unsigned int arg)
{
    napi_value res;
    napi_create_uint32(global_env, arg, &res);
    return res;
}
napi_value cpp2napi(size_t arg)
{
    napi_value res;
    napi_create_int64(global_env, arg, &res);
    return res;
}
napi_value cpp2napi(long arg)
{
    napi_value res;
    napi_create_int64(global_env, arg, &res);
    return res;
}
napi_value cpp2napi(float arg)
{
    napi_value res;
    napi_create_double(global_env, (double)arg, &res);
    return res;
}
napi_value cpp2napi(double arg)
{
    napi_value res;
    napi_create_double(global_env, arg, &res);
    return res;
}
napi_value cpp2napi(const std::string &arg)
{
    napi_value res;
    napi_create_string_utf8(global_env, arg.c_str(), arg.size(), &res);
    return res;
}
%s
template <typename Arg>
void val_array_push(EM_VAL handle, Arg arg)
{
    typedef std::vector<napi_value> arr;
    typedef emscripten::memory_view<napi_value> memory_view2arr;

    arr &array = static_cast<__EM_VAL<arr> *>(handle.get())->container;
    WireTypePack<memory_view2arr> *container = (WireTypePack<memory_view2arr> *)handle->data;
    GenericWireType *container_cursor = (GenericWireType *)(EM_VAR_ARGS)*container;

    napi_value res = cpp2napi(arg);
    array.push_back(res);
    container_cursor->w[1].p = array.data();
    container_cursor->w[0].u = array.size();
}

template <typename T>
void napi2cpp(napi_value arg, T *&res)
{
    printf("napi2cpp type not supported!\\n");
    return;
}
void napi2cpp(napi_value arg, bool &res)
{
    napi_get_value_bool(global_env, arg, &res);
}
void napi2cpp(napi_value arg, char &res)
{
    napi_get_value_int32(global_env, arg, (int32_t *)&res);
}
void napi2cpp(napi_value arg, unsigned char &res)
{
    napi_get_value_uint32(global_env, arg, (uint32_t *)&res);
}
void napi2cpp(napi_value arg, short &res)
{
    napi_get_value_int32(global_env, arg, (int32_t *)&res);
}
void napi2cpp(napi_value arg, unsigned short &res)
{
    napi_get_value_uint32(global_env, arg, (uint32_t *)&res);
}
void napi2cpp(napi_value arg, int &res)
{
    napi_get_value_int32(global_env, arg, &res);
}
void napi2cpp(napi_value arg, unsigned int &res)
{
    napi_get_value_uint32(global_env, arg, &res);
}
void napi2cpp(napi_value arg, size_t &res)
{
    napi_get_value_int64(global_env, arg, (int64_t *)&res);
}
void napi2cpp(napi_value arg, long &res)
{
    napi_get_value_int64(global_env, arg, (int64_t *)&res);
}
void napi2cpp(napi_value arg, float &res)
{
    double data;
    napi_get_value_double(global_env, arg, &data);
    res = data;
}
void napi2cpp(napi_value arg, double &res)
{
    napi_get_value_double(global_env, arg, &res);
}
void napi2cpp(napi_value arg, std::string &res)
{
    size_t strlen;
    napi_get_value_string_utf8(global_env, arg, NULL, 0, &strlen);
    res.resize(strlen + 1, 0);
    size_t len;
    napi_get_value_string_utf8(global_env, arg, (char *)res.c_str(), strlen + 1, &len);
}
#ifdef opencv_String
void napi2cpp(napi_value arg, String &res)
{
    size_t strlen;
    napi_get_value_string_utf8(global_env, arg, NULL, 0, &strlen);
    std::string data = std::string(strlen + 1, 0);
    size_t len;
    napi_get_value_string_utf8(global_env, arg, (char *)res.c_str(), strlen + 1, &len);
    res = data;
}
#endif
%s
}  // namespace internal
}  // namespace emscripten
"""

class_declaration = Template("""
/////////////////////////////////${jstype}///////////////////////////////////////
class ${name}
{
 public:
    ${name}()
        : target_(nullptr)
        , env_(nullptr)
        , wrapper_(nullptr)
        , is_deleted_(false) {}
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
    static napi_value isDeleted(napi_env env, napi_callback_info info);
    static napi_ref cons_ref();
    ${type} *target() { return target_; }
    void setDeleted() { is_deleted_ = true; }
    bool getDeleted() { return is_deleted_; }
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
    static ${type} *${name}_constructor_factory(napi_env env, size_t argc, napi_value *args);

    static napi_ref constructor_;
    ${type} *target_;
    napi_env env_;
    napi_ref wrapper_;
    bool is_deleted_;
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
napi_value ${name}::isDeleted(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value _this;
    napi_get_cb_info(env, info, &argc, nullptr, &_this, nullptr);
    napi_value args[argc];
    napi_get_cb_info(env, info, &argc, args, &_this, nullptr);

    ${name} *obj;
    napi_unwrap(env, _this, reinterpret_cast<void **>(&obj));

    napi_value result;
    napi_get_boolean(env, obj->getDeleted(), &result);
    return result;
}
napi_value ${name}::Release(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value _this;
    napi_get_cb_info(env, info, &argc, nullptr, &_this, nullptr);
    napi_value args[argc];
    napi_get_cb_info(env, info, &argc, args, &_this, nullptr);

    ${name} *obj;
    napi_unwrap(env, _this, reinterpret_cast<void **>(&obj));

    obj->setDeleted();
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
        NAPI_DECLARE_METHOD("delete", Release),
        NAPI_DECLARE_METHOD("isDeleted", isDeleted)
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

napi_init = Template("""
napi_value Init(napi_env env, napi_value exports)
{
    global_env = env;
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
${type} *${name}::${name}_constructor_factory(napi_env env, size_t argc, napi_value *args)
{
 ${type} *p = nullptr;
 switch (argc) {
""")
constructor_func_end = """
 }
 return p;
}
"""

args_bool = """\
    // arg{0}
    bool arg{0};
    napi_get_value_bool(env, args[{0}], &arg{0});
"""
args_int = """\
    // arg{0}
    int32_t arg{0} = 0;
    napi_get_value_int32(env, args[{0}], &arg{0});
"""
args_uint = """\
    // arg{0}
    int32_t arg{0} = 0;
    napi_get_value_uint32(env, args[{0}], &arg{0});
"""
args_long = """\
    // arg{0}
    int64_t arg{0} = 0;
    napi_get_value_int64(env, args[{0}], &arg{0});
"""
args_double = """\
    // arg{0}
    double arg{0} = 0;
    napi_get_value_double(env, args[{0}], &arg{0});
"""
args_string = """\
    // arg{0}
    size_t strlen{0};
    napi_get_value_string_utf8(env, args[{0}], NULL, 0, &strlen{0});
    std::string arg{0}(strlen{0} + 1, 0);
    size_t res{0};
    napi_get_value_string_utf8(env, args[{0}], (char *)arg{0}.c_str(), strlen{0} + 1, &res{0});
"""
# args_cxxtype = """\
#     // arg{0}
#     %s *p{0} = nullptr;
#     napi_unwrap(env, args[{0}], reinterpret_cast<void **>(&p{0}));
#     %s &arg{0} = *(p{0}->target());
# """
args_cxxtype = Template("""
namespace emscripten {
namespace internal {
void napi2cpp(napi_value arg, ${cxx_type} *&res)
{
    ${class_name} *p = nullptr;
    napi_unwrap(global_env, arg, reinterpret_cast<void **>(&p));
    res = p->target();
}
}  // namespace internal
}  // namespace emscripten
""")

args_obj = Template("""\
namespace emscripten {
namespace internal {
void napi2cpp(napi_value arg, ${cxx_type} &res)
{
${obj_detail}    
}
}  // namespace internal
}  // namespace emscripten
""")
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
    napi_value ret{{0}}_{0};
    napi_get_element(env, array{{0}}, {0}, &ret{{0}}_{0});
    double arg{{0}}_{0} = 0;
    napi_get_value_double(env, ret{{0}}_{0}, &arg{{0}}_{0});
"""
arr_args_int = """\
    napi_value ret{{0}}_{0};
    napi_get_element(env, array{{0}}, {0}, &ret{{0}}_{0});
    int32_t arg{{0}}_{0} = 0;
    napi_get_value_int32(env, ret{{0}}_{0}, &arg{{0}}_{0});
"""
arr_args = {
    'float': arr_args_double,
    'double': arr_args_double,
    'int': arr_args_int
}
args_val_array = """\
    // arg{0}
    std::vector<%s> vec{0};
    napi_value array{0} = args[{0}];
    uint32_t length{0};
    napi_get_array_length(env, array{0}, &length{0});
    for (int i = 0; i < length{0}; ++i) {{
%s
        vec{0}.push_back(arg{0}_i);
    }}
    val arg{0} = val::array(vec{0});
"""

return_class = Template("""
namespace emscripten {
namespace internal {
napi_value cpp2napi(const ${cxx_type} &arg)
{
    napi_value res;
    napi_value cons;
    napi_get_reference_value(global_env, ${class_name}::cons_ref(), &cons);
    napi_new_instance(global_env, cons, 0, nullptr, &res);
    ${class_name} *p;
    napi_unwrap(global_env, res, reinterpret_cast<void **>(&p));
    p->update_target(arg);
    return res;
}
}  // namespace internal
}  // namespace emscripten
""")

return_obj = Template("""
namespace emscripten {
namespace internal {
napi_value cpp2napi(const ${cxx_type} &arg)
{
    napi_value res;
    napi_create_object(global_env, &res);
${obj_detail}
    return res;
}
}  // namespace internal
}  // namespace emscripten
""")
return_array = Template("""
namespace emscripten {
namespace internal {
napi_value cpp2napi(const ${cxx_type} &arg)
{
    napi_value res;
    napi_create_array(global_env, &res);
    for (int i = 0; i < ${size}; i++) {
        napi_value value = cpp2napi(arg[i]);
        napi_set_element(global_env, res, i, value);
    }
    return res;
}
}  // namespace internal
}  // namespace emscripten
""")
arr_type = {
    'char': 'napi_int8_array',
    'unsigned char': 'napi_uint8_array',
    'short': 'napi_int16_array',
    'unsigned short': 'napi_uint16_array',
    'int': 'napi_int32_array',
    'unsigned int': 'napi_uint32_array',
    'float': 'napi_float32_array',
    'double': 'napi_float64_array',
    'long': 'napi_bigint64_array',
    'size_t': 'napi_biguint64_array'
}
return_val_typedarray = Template("""    WireTypePack<${val_type}> *argv = (WireTypePack<${val_type}> *)${cxx_val}.get_handle()->data;
    GenericWireType *cursor = (GenericWireType *)(EM_VAR_ARGS)*argv;
    size_t array_length = cursor->w[0].u;
    void *array_data = (void *)cursor->w[1].p;

    napi_value output_buffer;
    napi_create_external_arraybuffer(env,
                                     array_data,
                                     array_length * sizeof(${val_type}),
                                     NULL,  // finalize_callback
                                     NULL,  // finalize_hint
                                     &output_buffer);
    napi_create_typedarray(env,
                           ${array_type},
                           array_length,
                           output_buffer,
                           0,
                           &${napi_val});
    delete argv;
    ${cxx_val}.clear_handle();
""")
return_val_array = Template("""    WireTypePack<napi_value> *argv = (WireTypePack<napi_value> *)${cxx_val}.get_handle()->data;
    GenericWireType *cursor = (GenericWireType *)(EM_VAR_ARGS)*argv;
    size_t array_length = cursor->w[0].u;
    napi_value *array_data = (napi_value *)cursor->w[1].p;

    napi_create_array(env, &${napi_val});
    for (int i = 0; i < array_length; ++i) {
        napi_set_element(env, ${napi_val}, i, array_data[i]);
    }
    delete argv;
    ${cxx_val}.clear_handle();
""")
return_val_object = Template("""    if (res.isUndefined()) {
        return nullptr;
    }
    WireTypePack<${val_type}> *argv = (WireTypePack<${val_type}> *)${cxx_val}.get_handle()->data;
    GenericWireType *cursor = (GenericWireType *)(EM_VAR_ARGS)*argv;

${get_data}
${create_return_val}
    delete argv;
    ${cxx_val}.clear_handle();
""")

# ###############################################################################################
function_detail_start = """
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
    ${type} *target = obj ? obj->target_ : nullptr;
    ${return_res}fun_${fun_name}_factory(target, env, argc, args);
    ////////////////////////////////////////////////////////////////////////
${return_val}
}
""")

static_func_template = Template("""
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
    ${return_res}fun_${fun_name}_factory(nullptr, env, argc, args);
    ////////////////////////////////////////////////////////////////////////
${return_val}
}
""")

prop_getter = Template("""
napi_value ${name}::${fun_name}(napi_env env, napi_callback_info info)
{
    napi_value _this;
    napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr);

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

global_malloc = """
napi_value global_malloc(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value _this;
    napi_get_cb_info(env, info, &argc, nullptr, &_this, nullptr);
    napi_value args[argc];
    napi_get_cb_info(env, info, &argc, args, &_this, nullptr);

    unsigned int kBufferSize = 0;
    napi_get_value_uint32(env, args[0], &kBufferSize);
    void *theCopy = nullptr;
    napi_value buffer;
    napi_create_buffer(env, kBufferSize, &theCopy, &buffer);
    int64_t p = reinterpret_cast<int64_t>(theCopy);
    napi_value result;
    napi_create_int64(env, p, &result);
    return result;
}
"""
constant_func = """
napi_value %s(napi_env env, napi_callback_info info)
{
    napi_value result;
    napi_create_int32(env, %s, &result);
    return result;
}
"""
array_func = """
napi_value generate_%s(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value _this;
    napi_get_cb_info(env, info, &argc, nullptr, &_this, nullptr);
    napi_value args[argc];
    napi_get_cb_info(env, info, &argc, args, &_this, nullptr);

    NAPI_ASSERT(env, argc <= %s, "Arg size is greater than the desired array size!");
    napi_value result;
    napi_create_array(env, &result);
    for (int i = 0; i < argc; i++) {
        napi_set_element(env, result, i, args[i]);
    }
    return result;
}
"""

global_func_start = Template("""
${return_type} fun_${fun_name}_factory(napi_env env, size_t argc, napi_value *args)
{
 switch (argc) {
""")


global_func_end = Template("""
 }
}
napi_value ${fun_name}(napi_env env, napi_callback_info info)
{
    size_t argc = 0;
    napi_value _this;
    napi_get_cb_info(env, info, &argc, nullptr, &_this, nullptr);
    napi_value args[argc];
    napi_get_cb_info(env, info, &argc, args, &_this, nullptr);

    ${return_res}fun_${fun_name}_factory(env, argc, args);

${return_val}
}
""")
