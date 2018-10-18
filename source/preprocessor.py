import os
import re
import emscripten


class Preprocessor:
    RE = r'EMSCRIPTEN_BINDINGS\(.*\)\s*\{(?P<bindings_body>.*)\}'

    MACRO_DEFINE_RE = r'#define[ \t]+((?P<macro>\w+)\((?P<macro_arglist>[^\\]*)\))(?P<macro_def>[ \t]+((\\[ \t]*\n[ \t]*)([^\n\\]*))*)'

    def __init__(self):
        self.output = ''
        self.target = ''

    def input(self, file):
        self.target = file
        with open(file, 'r') as f:
            file_obj = f
            self.data = file_obj.read()
            file_obj.close()

    def compile(self):
        searchObj = re.findall('#include(.*)\n', self.data)
        self.include = []
        if searchObj:
            self.include = ['#include%s' % file for file in searchObj]
        searchObj = re.findall('using namespace (.*)\n', self.data)
        self.namespace_declaration = []
        if searchObj:
            self.namespace_declaration = ['using namespace %s' % namespace for namespace in searchObj]
        searchObj = re.findall('\nnamespace (.*)\n', self.data)
        self.namespace = []
        if searchObj:
            self.namespace = [namespace.rstrip(' {') for namespace in searchObj]
        pattern = re.compile(Preprocessor.RE, flags=re.DOTALL)
        start = 0
        pos = []
        while True:
            start = self.data.find('EMSCRIPTEN_BINDINGS', start + 1)
            if start == -1:
                break
            pos.append(start)
            start = self.data.find('}', start + 1)
            if start == -1:
                break
            pos.append(start + 1)
        output = ''
        for i in range(len(pos) / 2):
            result = pattern.search(self.data, pos[2 * i], pos[2 * i + 1])
            if result == None:
                break
            output += result.group('bindings_body')
        self.napi_compile(output, {'supplement': self.include +
                                   self.namespace_declaration, 'namespace': self.namespace})

    def napi_compile(self, data, include):
        cxx_bind = self.cxx_preprocess(data)
        lexer = emscripten.Lexer()
        lexer.lexing(cxx_bind)
        return lexer.napi_compile(self.target, include)

    def cxx_preprocess(self, data):
        pattern = re.compile(Preprocessor.MACRO_DEFINE_RE, flags=re.DOTALL)
        start = 0
        output = ''
        macros = []
        while True:
            result = pattern.search(data, start)
            if result == None:
                output += data[start:]
                break
            output += data[start:result.start()]
            start = result.end()
            macros.append((result.group('macro'), result.group('macro_arglist'), re.sub(
                r'[ \t]+\\[ \t]*', ' ', result.group('macro_def')).strip()))
        return self.cxx_macro_replace(output, macros)

    #print arglist_re, params
    def cxx_macro_replace(self, data, macros):
        for macro in macros:
            func, arglist, body = macro
            arglist_re, params = self.get_cxx_macro_arglist_re(arglist)
            pattern = re.compile(func + r'\(' + arglist_re + r'\)')
            start = 0
            output = ''
            while True:
                result = pattern.search(data, start)
                if result == None:
                    output += data[start:]
                    break
                output += data[start:result.start()]
                output += self.cxx_macro_replace_execute(params, body, result.groups())
                start = result.end()
            data = output
        return data

    def cxx_macro_replace_execute(self, params, body, real_params):
        for param, rparam in zip(params, real_params):
            body = re.sub(r'#%s' % (param), rparam, body)
            body = re.sub(r'%s' % (param), rparam, body)
        return body

    def get_cxx_macro_arglist_re(self, data):
        _re = r''
        params = []
        pattern = re.compile(r'(?P<param>\w+)')
        start = 0
        while True:
            result = pattern.search(data, start)
            if result == None:
                _re += data[start:]
                break
            params.append(result.group('param'))
            _re += data[start:result.start()].strip()
            _re += r'\s*([\"\w]+)\s*'
            start = result.end()
        return _re, params
