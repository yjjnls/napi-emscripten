import os
import re
import template
import json


class Gen:
    def __init__(self, target):
        self.target = target
        self.base_path = os.path.dirname(os.path.realpath(target))
        self.constructors = {}
        self.functions = {}
        self.properties = {}
        self.class_functions = {}

    def genfile_start(self):
        if not os.path.exists(os.path.join(self.base_path, 'plugin')):
            os.mkdir(os.path.join(self.base_path, 'plugin'))

        self.output_cxx = os.path.join(self.base_path, 'plugin/binding.cc')
        self.output_cxx_fp = open(self.output_cxx, 'w+')

        (shotname, extension) = os.path.splitext(
            os.path.basename(os.path.realpath(self.target)))
        self.output_js = os.path.join(self.base_path, 'plugin/%s.js' % shotname)
        self.output_js_fp = open(self.output_js, 'w+')

        self.output_gyp = os.path.join(self.base_path, 'plugin/binding.gyp')
        self.output_gyp_fp = open(self.output_gyp, 'w+')

        #########################################################################
        # node-gyp
        self.generate_gyp()
        # fixed function and macro
        self.output_cxx_fp.write(template.bind_cxx_fixed)
        # Addon class declaration
        self.output_cxx_fp.write(template.class_start % self.cxxtype)
        napi_fun = ''
        for fun_name in self.functions.keys():
            napi_fun += '        NAPI_DECLARE_METHOD("{0}", {0}),\n'.format(
                fun_name)
            self.output_cxx_fp.write(
                '    static napi_value %s(napi_env env, napi_callback_info info);\n' % fun_name)
        self.output_cxx_fp.write(template.class_end % self.cxxtype)
        # namespace
        self.output_cxx_fp.write('namespace %s {\n' % self.namespace)
        for overload_fun in self.functions.values():
            for spec_fun in overload_fun:
                fun_detail = spec_fun.values()[0]
                if not self.cxxtype in fun_detail[2]:
                    self.output_cxx_fp.write('extern %s %s(%s);\n' % (
                        fun_detail[0], fun_detail[2].split('::')[1], fun_detail[3]))
        self.output_cxx_fp.write(
            '}  // namespace binding_utils\nusing namespace binding_utils;\n')
        # constructor implementation
        self.generate_constructor()
        # function implementation
        self.generate_function()

        # napi declaration
        self.output_cxx_fp.write(template.fixed_class_function % self.cxxtype)
        self.output_cxx_fp.write(template.napi_init %
                                 ('', napi_fun, self.cxxtype, self.cxxtype, ''))

    def genfile_end(self):
        self.output_cxx_fp.close()
        self.output_js_fp.close()
        self.output_gyp_fp.close()

    def generate_gyp(self):
        self.output_gyp_fp.write(template.bind_gyp %
                                 (os.path.relpath(self.output_cxx, start=os.path.dirname(self.output_gyp)),
                                  os.path.relpath(self.target, start=os.path.dirname(self.output_gyp))))

    def parse_class(self, obj):
        self.cxxtype = obj.cxxtype
        self.jstype = obj.jstype

        self.parse_constructor(obj.constructors)
        self.parse_function(obj.functions)

    def parse_constructor(self, constructors):
        for constructor in constructors:
            # print 'constructor arg types: %s\n' % constructor.cxxargtypes
            # print 'constructor cxxparams: %s\n'%constructor.cxxparams
            if constructor.cxxargtypes == None:
                # todo something
                continue
            if ', ' in constructor.cxxargtypes:
                argtypes = constructor.cxxargtypes.split(', ')
                self.constructors[len(argtypes)] = argtypes
            else:
                self.constructors[len(
                    constructor.cxxargtypes)] = constructor.cxxargtypes

        print self.constructors
        print ''

    def parse_function(self, functions):
        for function in functions.items():
            js_method = function[0]
            self.functions[js_method] = []
            for f in function[1]:
                if not 'select_overload' in f[0]:
                    raise Exception("Invalid function format!")
                searchObj = re.search(
                    'select_overload<(.*)>[(][&](.*)[)]', f[0])
                # print f[0]
                if searchObj:
                    print searchObj.group(0)
                    res = re.search('(.*)[(](.*)[)]', searchObj.group(1))
                    if res:
                        return_type = res.group(1)
                        args = res.group(2).split(', ')
                        fun = searchObj.group(2)
                        if not self.cxxtype in searchObj.group(2):
                            del args[0]
                            self.namespace = searchObj.group(2).split('::')[0]
                        if args == ['']:
                            args = []
                        # print return_type
                        # print args
                        # print fun
                        # print ""

                    self.functions[js_method].append(
                        {len(args): (return_type, args, fun, res.group(2))})
        print json.dumps(self.functions, sort_keys=True,
                         indent=4, separators=(',', ': '))

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
        if self.cxxtype in arg:
            return template.return_external
        return None

    def generate_constructor(self):
        self.output_cxx_fp.write(
            template.constructor_func_start % (self.cxxtype, self.cxxtype))

        for cons_fun in self.constructors.items():
            self.output_cxx_fp.write('        case %d: {\n' % cons_fun[0])

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
                '\n            p = new {0}({1});\n'.format(self.cxxtype, args))

            self.output_cxx_fp.write('        } break;\n')

        self.output_cxx_fp.write(template.constructor_func_end)

    def generate_function(self):
        for overload_fun in self.functions.items():
            fun_name = overload_fun[0]

            return_type = overload_fun[1][0].values()[0][0]
            self.output_cxx_fp.write(template.function_datail_start % (
                return_type, fun_name, self.cxxtype))
            for spec_fun in overload_fun[1]:
                self.output_cxx_fp.write(
                    '        case %d: {\n' % spec_fun.keys()[0])

                argc = 0
                args = ''
                arg_list = spec_fun.values()[0][1]
                for i in range(len(arg_list)):
                    arg_type = arg_list[i]
                    self.output_cxx_fp.write(
                        self.parse_arg_type(arg_type).format(i))
                    argc += 1
                    args += 'arg{0}'.format(i)
                    if not i == len(arg_list) - 1:
                        args += ', '

                fun_name = spec_fun.values()[0][2]
                if 'operator()' in fun_name:
                    self.output_cxx_fp.write(
                        '\n            return (*obj)({0});\n'.format(args))
                if not self.cxxtype in fun_name:
                    self.output_cxx_fp.write(
                        '\n            return {0}(*obj, {1});\n'.format(fun_name, args))
                else:
                    self.output_cxx_fp.write(
                        '\n            return obj->{0}({1});\n'.format(fun_name, args))

                self.output_cxx_fp.write('        } break;\n')

            self.output_cxx_fp.write(template.function_datail_end)

            if return_type == 'void':
                return_res = ''
            else:
                return_res = '%s res = ' % return_type
            return_val = self.parse_return_type(return_type)
            self.output_cxx_fp.write(template.func_template % (
                overload_fun[0], self.cxxtype, return_res, overload_fun[0], return_val))
