import os
import re
import template


class Gen:
    def __init__(self, target):
        self.target = target
        self.base_path = os.path.dirname(os.path.realpath(target))
        # {"js_method":[("cxx_return_type","cxx_args_list(invoke)","cxx_func_name","cxx_args_list(real)")]}
        self.constructors = {}
        self.functions = {}
        self.properties = {}
        self.class_functions = {}
        self.pattern = 'select_overload<(?P<return_type>.*)[(](?P<args_list>.*)[)].*>[(][&](?P<fun_name>((?!,).)*)[)(.*)]'

    def genfile_start(self):
        if not os.path.exists(os.path.join(self.base_path, 'plugin')):
            os.mkdir(os.path.join(self.base_path, 'plugin'))

        self.output_cxx = os.path.join(self.base_path, 'plugin/binding.cc')
        self.output_cxx_fp = open(self.output_cxx, 'w+')

        (shotname, extension) = os.path.splitext(os.path.basename(os.path.realpath(self.target)))
        self.output_js = os.path.join(self.base_path, 'plugin/%s.js' % shotname)
        self.output_js_fp = open(self.output_js, 'w+')

        self.output_gyp = os.path.join(self.base_path, 'plugin/binding.gyp')
        self.output_gyp_fp = open(self.output_gyp, 'w+')

        #########################################################################
        # node-gyp
        # self.generate_gyp()
        # fixed function and macro
        self.output_cxx_fp.write(template.bind_cxx_fixed)
        # Addon class declaration
        self.output_cxx_fp.write(template.class_start % self.cxxtype)
        self.output_cxx_fp.write('    // function\n')
        napi_fun = ''
        for fun_name in self.functions.keys():
            napi_fun += '        NAPI_DECLARE_METHOD("{0}", {0}),\n'.format(fun_name)
            self.output_cxx_fp.write('    static napi_value %s(napi_env env, napi_callback_info info);\n' % fun_name)

        self.output_cxx_fp.write('    // property\n')
        napi_property = ''
        for prop in self.properties.items():
            prop_name = prop[0]
            getter = 'get%s' % prop_name
            self.output_cxx_fp.write('    static napi_value %s(napi_env env, napi_callback_info info);\n' % getter)
            if not prop[1][1] == None:
                setter = 'set%s' % prop_name
                self.output_cxx_fp.write('    static napi_value %s(napi_env env, napi_callback_info info);\n' % setter)
            else:
                setter = 'nullptr'
            napi_property += '        {"%s", nullptr, nullptr, %s, %s, 0, napi_default, 0},\n' % (prop_name,
                                                                                                  getter,
                                                                                                  setter)

        self.output_cxx_fp.write('    // class function\n')
        napi_class_function = ''
        for fun_name in self.class_functions.keys():
            self.output_cxx_fp.write('    static napi_value %s(napi_env env, napi_callback_info info);\n' % fun_name)
            napi_class_function += '        NAPI_DECLARE_METHOD("{0}", Addon::{0}),\n'.format(fun_name)
        self.output_cxx_fp.write(template.class_end % self.cxxtype)
        # namespace
        self.output_cxx_fp.write('namespace %s {\n\n' % self.namespace)
        for item in [self.constructors, self.functions, self.properties, self.class_functions]:
            for overload_fun in item.values():
                for spec_fun in overload_fun:
                    if not spec_fun == None and not spec_fun[3] == None:
                        self.output_cxx_fp.write('extern %s %s(%s);\n' % (
                            spec_fun[0],
                            spec_fun[2].split('::')[1],
                            spec_fun[3]))
        self.output_cxx_fp.write('\n}  // namespace binding_utils\nusing namespace binding_utils;\n')
        # constructor implementation
        self.generate_constructor()
        # function implementation
        self.generate_function()
        # property implementation
        self.generate_prop()
        # class function implementation
        self.generate_class_function()
        # napi declaration
        self.output_cxx_fp.write(template.fixed_class_function % self.cxxtype)
        self.output_cxx_fp.write(template.napi_init %
                                 (napi_property,
                                  napi_fun,
                                  self.cxxtype,
                                  self.cxxtype,
                                  napi_class_function))

    def genfile_end(self):
        self.output_cxx_fp.close()
        self.output_js_fp.close()
        self.output_gyp_fp.close()

    def generate_gyp(self):
        self.output_gyp_fp.write(template.bind_gyp %
                                 (os.path.relpath(self.output_cxx, start=os.path.dirname(self.output_gyp)),
                                  os.path.relpath(self.target, start=os.path.dirname(self.output_gyp))))

    def parse_func_line(self, line, bool_static=False):
        if line == None:
            return None
        searchObj = re.search(self.pattern, line)
        if searchObj:
            return_type = searchObj.group('return_type')
            args_list = searchObj.group('args_list').split(', ')
            fun_name = searchObj.group('fun_name')
            args_real = None
            if not self.cxxtype in fun_name:
                if not bool_static:
                    del args_list[0]
                self.namespace = fun_name.split('::')[0]
                args_real = searchObj.group('args_list')
            if args_list == ['']:
                args_list = []
            return (return_type, args_list, fun_name, args_real)
        return None

    def parse_class(self, obj):
        self.cxxtype = obj.cxxtype
        self.jstype = obj.jstype

        self.parse_constructor(obj.constructors)
        self.parse_function(obj.functions)
        self.parse_property(obj.properties)
        self.parse_class_function(obj.class_functions)

    def parse_constructor(self, constructors):
        self.constructors['constructor'] = []
        for constructor in constructors:
            # print 'constructor arg types: %s\n' % constructor.cxxargtypes
            # print 'constructor cxxparams: %s\n' % constructor.cxxparams
            if constructor.cxxargtypes == None:
                searchObj = re.search(self.pattern, constructor.cxxparams)
                if searchObj:
                    args_list = searchObj.group('args_list').split(', ')
                    if args_list == ['']:
                        args_list = []
                    self.constructors['constructor'].append(('%s *' % self.cxxtype,
                                                             searchObj.group('args_list').split(', '),
                                                             searchObj.group('fun_name'),
                                                             searchObj.group('args_list')))
            else:
                args_list = constructor.cxxargtypes.split(', ')
                if args_list == ['']:
                    args_list = []
                self.constructors['constructor'].append(('%s *' % self.cxxtype,
                                                         args_list,
                                                         'new %s' % self.cxxtype,
                                                         None))

        print '===========constructors=========='
        print self.constructors
        print ''

    def parse_function(self, functions):
        for function in functions.items():
            js_method = function[0]
            self.functions[js_method] = []
            for spec_fun in function[1]:
                detail = self.parse_func_line(spec_fun[0])
                self.functions[js_method].append(detail)

        print '===========functions=========='
        print self.functions
        print ''

    def parse_property(self, properties):
        for prop in properties.items():
            js_method = prop[0]
            self.properties[js_method] = []
            prop_function = prop[1][0][0]
            if ',' in prop_function:
                getter = prop_function.split(',')[0]
                setter = prop_function.split(',')[1]
            else:
                getter = prop_function
                setter = None

            self.properties[js_method].append(self.parse_func_line(getter))
            self.properties[js_method].append(self.parse_func_line(setter))

        print '===========properties=========='
        print self.properties
        print ''

    def parse_class_function(self, class_functions):
        for static_func in class_functions.items():
            print static_func
            js_method = static_func[0]
            self.class_functions[js_method] = []
            for spec_fun in static_func[1]:
                detail = self.parse_func_line(spec_fun[0], True)
                self.class_functions[js_method].append(detail)

        print '===========class functions=========='
        print self.class_functions
        print ''

    def parse_arg_type(self, arg):
        if arg == 'int':
            return template.args_int
        if 'string' in arg:
            return template.args_string
        if self.cxxtype in arg:
            return template.args_cxxtype % self.cxxtype
        return None

    def parse_return_type(self, arg):
        if arg == 'void':
            return template.return_void
        if arg == 'int':
            return template.return_int
        if arg == 'std::string':
            return template.return_string
        if self.cxxtype in arg:
            return template.return_external
        return None

    def generate_constructor(self):
        self.output_cxx_fp.write(template.constructor_func_start % (self.cxxtype, self.cxxtype))

        for cons_fun in self.constructors.values()[0]:
            # print cons_fun
            self.output_cxx_fp.write('        case %d: {\n' % len(cons_fun[1]))

            argc = 0
            args = ''
            for i in range(len(cons_fun[1])):
                arg_type = cons_fun[1][i]
                self.output_cxx_fp.write(
                    self.parse_arg_type(arg_type).format(i))
                argc += 1
                args += 'arg{0}'.format(i)
                if not i == len(cons_fun[1]) - 1:
                    args += ', '
            self.output_cxx_fp.write(
                '\n            p = {0}({1});\n'.format(cons_fun[2], args))

            self.output_cxx_fp.write('        } break;\n')

        self.output_cxx_fp.write(template.constructor_func_end)

    def generate_function(self):
        def detail(fun_name, args):
            if 'operator()' in fun_name:
                self.output_cxx_fp.write('\n            return (*obj)({0});\n'.format(args))
            elif not self.cxxtype in fun_name:
                self.output_cxx_fp.write('\n            return {0}(*obj, {1});\n'.format(fun_name, args))
            else:
                self.output_cxx_fp.write('\n            return obj->{0}({1});\n'.format(fun_name, args))

        self.generate_function_detail(self.functions, detail)

    def generate_function_detail(self, functions, func_detail):
        for overload_fun in functions.items():
            fun_name = overload_fun[0]
            return_type = overload_fun[1][0][0]
            self.output_cxx_fp.write(template.function_datail_start % (return_type,
                                                                       fun_name,
                                                                       self.cxxtype))
            for spec_fun in overload_fun[1]:
                self.output_cxx_fp.write('        case %d: {\n' % len(spec_fun[1]))

                argc = 0
                args = ''
                arg_list = spec_fun[1]
                for i in range(len(arg_list)):
                    arg_type = arg_list[i]
                    self.output_cxx_fp.write(self.parse_arg_type(arg_type).format(i))
                    argc += 1
                    args += 'arg{0}'.format(i)
                    if not i == len(arg_list) - 1:
                        args += ', '

                fun_name = spec_fun[2]

                func_detail(fun_name, args)

                self.output_cxx_fp.write('        } break;\n')

            self.output_cxx_fp.write(template.function_datail_end)

            if return_type == 'void':
                return_res = ''
            else:
                return_res = '%s res = ' % return_type
            return_val = self.parse_return_type(return_type)
            self.output_cxx_fp.write(template.func_template % (overload_fun[0],
                                                               self.cxxtype,
                                                               return_res,
                                                               overload_fun[0],
                                                               return_val))

    def generate_prop(self):
        for prop in self.properties.items():
            # getter
            if self.cxxtype in prop[1][0][2]:
                res = self.parse_getter_type(prop[1][0][0], 'target->%s()' % prop[1][0][2])
            else:
                res = self.parse_getter_type(prop[1][0][0], '%s(*target)' % prop[1][0][2])

            self.output_cxx_fp.write(template.prop_getter % ('get%s' % prop[0], self.cxxtype, res))
            # setter
            if not prop[1][1] == None:
                res = self.parse_setter_type(prop[1][0][0])

                if self.cxxtype in prop[1][1][2]:
                    fun = '    target->%s(value);' % prop[1][1][2]
                else:
                    fun = '    %s(*target, value);' % prop[1][1][2]

                self.output_cxx_fp.write(template.prop_setter % ('set%s' % prop[0], self.cxxtype, res, fun))

    def parse_getter_type(self, arg, fun):
        if arg == 'int':
            return template.getter_int % fun
        return None

    def parse_setter_type(self, arg):
        if arg == 'int':
            return template.setter_int
        return None

    def generate_class_function(self):
        def detail(fun_name, args):
            if not self.cxxtype in fun_name:
                self.output_cxx_fp.write('\n            return {0}({1});\n'.format(fun_name, args))
            else:
                self.output_cxx_fp.write('\n            return {0}({1});\n'.format(fun_name, args))

        self.generate_function_detail(self.class_functions, detail)
