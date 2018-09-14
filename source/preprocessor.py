import re
import emscripten

class Preprocessor:
    RE = r'EMSCRIPTEN_BINDINGS\(.*\)\s*\{(?P<bindings_body>.*)\}'
    
    MACRO_DEFINE_RE = r'#define[ \t]+((?P<macro>\w+)\((?P<macro_arglist>[^\\]*)\))(?P<macro_def>[ \t]+((\\[ \t]*\n[ \t]*)([^\n\\]*))*)'
    def __init__(self):
        self.output = ''

    def input(self, file):
        with open(file, 'r') as f:
            self.data = f.read()
    
    def compile(self):
        pattern = re.compile(Preprocessor.RE, flags=re.DOTALL)
        start = 0
        while True:
            result = pattern.search(self.data, start)
            if result == None:
                break
            self.output += self.data[start:result.start()]
            self.output += self.napi_compile(result.group('bindings_body'))
            start = result.end()

    def napi_compile(self, data):
        cxx_bind = self.cxx_preprocess(data)
        lexer = emscripten.Lexer()
        lexer.lexing(cxx_bind)
        return lexer.napi_compile()
    
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
            macros.append(( result.group('macro'), result.group('macro_arglist'), re.sub(r'[ \t]+\\[ \t]*',' ',result.group('macro_def')).strip() ))
        return self.cxx_macro_replace(output, macros)
    
    #print arglist_re, params
    def cxx_macro_replace(self, data, macros):
        for macro in macros:
            func, arglist, body = macro
            arglist_re, params = self.get_cxx_macro_arglist_re(arglist)
            pattern = re.compile(func+r'\('+arglist_re+r'\)')
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
            body=re.sub(r'#%s'%(param), rparam ,body)
            body=re.sub(r'%s'%(param), rparam ,body)
        return body

    def get_cxx_macro_arglist_re(self,data):
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