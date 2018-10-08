import ply.lex as lex
from ply.lex import TOKEN
import re
from generate import Gen

    
class Class:
    __type_re = r'(emscripten::)?class_<(?P<cxxtype>.*)>\(\"(?P<jstype>.*)\"\)'
    RE = __type_re + r'(' + r'\s+' + r'\.(constructor|property|function|class_property|class_function)(<(.*)>)?\((.*)\)' + r')*'

    class Constructor:
        RE = r'\.constructor(<(?P<cxxargtypes>.*)>)?\((?P<cxxparams>.*)\)'
        def __init__(self, cxxargtypes='', cxxparams=''):
            self.cxxargtypes = cxxargtypes
            self.cxxparams = cxxparams
    
    class Property:
        RE = r'\.property\(\"(?P<jsfield>.*)\",\s*(?P<cxxfield_or_getter>.*)(,\s*(?P<cxxsetter>.*))?\)'
        def __init__(self, jsfield, cxxfield_or_getter, cxxsetter=None):
            self.jsfield = jsfield
            self.cxxfield_or_getter = cxxfield_or_getter
            self.cxxsetter = cxxsetter
    
    class Function:
        __policies_re = r',\s*(?P<cxxpolicies>[^,]*)'
        RE = r'\.function\(\"(?P<jsmethod>.*)\",\s*(?P<cxxmethod>.*)(?P<cxxpolicies>.*)?\)'
        def __init__(self, jsmethod, cxxmethod, cxxpolicies=None):
            self.jsmethod = jsmethod
            self.cxxmethod = cxxmethod
            self.cxxpolicies = cxxpolicies
    
    class ClassFunction:
        __policies_re = r',\s*(?P<cxxpolicies>[^,]*)'
        RE = r'\.class_function\(\"(?P<jsmethod>.*)\",\s*(?P<cxxmethod>.*)(?P<cxxpolicies>.*)?\)'
        def __init__(self, jsmethod, cxxmethod, cxxpolicies=None):
            self.jsmethod = jsmethod
            self.cxxmethod = cxxmethod
            self.cxxpolicies = cxxpolicies
    
    class ClassProperty:
        RE = r'\.class_property\(\"(?P<jsfield>.*)\",\s*(?P<cxxfield>.*)\)'
        def __init__(self, jsfield, cxxfield):
            self.jsfield = jsfield
            self.cxxfield = cxxfield

    def __init__(self, cxxtype='', jstype=''):
        self.cxxtype = cxxtype
        self.jstype = jstype

        self.constructors = []

        self.functions = {}
        self.properties = {}
        self.class_functions = {}
        self.class_properties = {}
    
    def add_constructor(self, obj):
        self.constructors.append(obj)
    
    def add_function(self, obj):
        arr = self.functions.setdefault(obj.jsmethod,[])
        arr.append((obj.cxxmethod, obj.cxxpolicies))
    
    def add_property(self, obj):
        arr = self.properties.setdefault(obj.jsfield,[])
        arr.append((obj.cxxfield_or_getter, obj.cxxsetter))
    
    def add_class_function(self, obj):
        arr = self.class_functions.setdefault(obj.jsmethod,[])
        arr.append((obj.cxxmethod, obj.cxxpolicies))
    
    def add_class_property(self, obj):
        arr = self.class_properties.setdefault(obj.jsfield,[])
        arr.append(obj.cxxfield)
    
    def make(self,data):
        li = data.split('.')
        result = re.match(Class.__type_re, li[0].strip())
        self.cxxtype = result.group('cxxtype')
        self.jstype = result.group('jstype')

        for s in li[1:]:
            if s.startswith('constructor'):
                result = re.match(Class.Constructor.RE, '.' + s.strip())
                obj = Class.Constructor(result.group('cxxargtypes'),result.group('cxxparams'))
                self.add_constructor(obj)
            elif s.startswith('property'):
                result = re.match(Class.Property.RE, '.' + s.strip())
                obj = Class.Property(result.group('jsfield'),result.group('cxxfield_or_getter'),result.group('cxxsetter'))
                self.add_property(obj)
            elif s.startswith('function'):
                result = re.match(Class.Function.RE, '.' + s.strip())
                obj = Class.Function(result.group('jsmethod'),result.group('cxxmethod'),result.group('cxxpolicies'))
                self.add_function(obj)
            elif s.startswith('class_function'):
                result = re.match(Class.ClassFunction.RE, '.' + s.strip())
                obj = Class.ClassFunction(result.group('jsmethod'),result.group('cxxmethod'),result.group('cxxpolicies'))
                self.add_class_function(obj)
            elif s.startswith('class_property'):
                result = re.match(Class.ClassProperty.RE, '.' + s.strip())
                obj = Class.ClassProperty(result.group('jsfield'),result.group('cxxfield'))
                self.add_class_property(obj)
            else:
                print 'invalid class member'


class Vector:
    RE = r'(emscripten::)?register_vector<(?P<cxxelemtype>.*)>\(\"(?P<jstype>.*)\"\)'

    def __init__(self, cxxelemtype, jstype):
        self.cxxelemtype = cxxelemtype
        self.jstype = jstype
    
    def make_class(self):
        cxxtype = 'std::vector<%s>'%(self.cxxelemtype)
        self.class_ = Class(cxxtype, self.jstype)

        self.class_.add_constructor(Class.Constructor())
        self.class_.add_function( Class.Function('push_back', '&%s::push_back'%(cxxtype)) )
        self.class_.add_function( Class.Function('resize', '&%s::resize'%(cxxtype)) )
        self.class_.add_function( Class.Function('size', '&%s::size'%(cxxtype)) )
        self.class_.add_function( Class.Function('get', '&emscripten::internal::VectorAccess<%s>::get'%(cxxtype)) )
        self.class_.add_function( Class.Function('set', '&emscripten::internal::VectorAccess<%s>::set'%(cxxtype)) )

class Map:
    RE = r'(emscripten::)?register_map<(?P<cxxkeytype>[^,]*),\s*(?P<cxxvaltype>.*)>\(\"(?P<jstype>.*)\"\)'

    def __init__(self, cxxkeytype, cxxvaltype, jstype):
        self.cxxkeytype = cxxkeytype
        self.cxxvaltype = cxxvaltype
        self.jstype = jstype

    def make_class(self):
        cxxtype = 'std::map<%s,%s>'%(self.cxxkeytype, self.cxxvaltype)
        self.class_ = Class(cxxtype, self.jstype)

        self.class_.add_constructor(Class.Constructor())
        self.class_.add_function( Class.Function('size', '&%s::size'%(cxxtype)) )
        self.class_.add_function( Class.Function('get', '&emscripten::internal::MapAccess<%s>::get'%(cxxtype)) )
        self.class_.add_function( Class.Function('set', '&emscripten::internal::MapAccess<%s>::set'%(cxxtype)) )

class Enum:
    __type_re = r'(emscripten::)?enum_<(?P<cxxtype>.*)>\(\"(?P<jstype>.*)\"\)'
    __val_re = r'\.value\(\"(?P<jsval>[^,]*)\",\s*(?P<cxxval>.*)\)'
    RE = __type_re + r'(' + r'\s*' + __val_re + r')' + r'+'

    '''
    self.cxxtype
    self.jstype
    self.val_dict
    '''

    def __init__(self):
        self.val_dict={}
    
    def make(self,data):
        li = data.split('.')
        result = re.match(Enum.__type_re, li[0].strip())
        self.cxxtype = result.group('cxxtype')
        self.jstype = result.group('jstype')
        for s in li[1:]:
            result = re.match(Enum.__val_re, '.'+s.strip())
            self.val_dict[result.group('jsval')] = result.group('cxxval')

class Constant:
    RE = r'(emscripten::)?constant\(\"(?P<jsval>.*)\",\s+(?P<cxxval>.*)\)'
    def __init__(self, jsval, cxxval):
        self.jsval = jsval
        self.cxxval = cxxval

class ValueArray:
    __type_re = r'(emscripten::)?value_array<(?P<cxxtype>.*)>\s*\(\"(?P<jstype>.*)\"\)'
    __element_re = r'\.element\((?P<cxx_field_getter_index>[^,]*)(\s+,(?P<cxx_setter>.*))?\)'
    RE = __type_re + r'(' + r'\s+' + r'.element\((.*)\)' + r')*'
    def __init__(self):
        self.elem_arr = []
    
    def make(self,data):
        li = data.split('.')
        result = re.match(ValueArray.__type_re, li[0].strip())
        self.cxxtype = result.group('cxxtype')
        self.jstype = result.group('jstype')
        for s in li[1:]:
            result = re.match(ValueArray.__element_re, '.'+s.strip())
            self.elem_arr.append((result.group('cxx_field_getter_index'),result.group('cxx_setter')))

class ValueObject:
    __type_re = r'(emscripten::)?value_object<\s*(?P<cxxtype>.*)>\s*\(\"(?P<jstype>.*)\"\)'
    __field_re = r'\.field\(\"(?P<js_field>.*)\",\s*(?P<cxx_field_getter_index>((?!,).)*)([,]\s*(?P<cxx_setter>.*))?\)'
    RE = __type_re + r'(' + r'\s+' + r'\.field\((.*)\)' + r')*'
    
    def __init__(self):
        self.field_arr = []
    
    def make(self,data):
        li = data.split('.')
        result = re.match(ValueObject.__type_re, li[0].strip())
        self.cxxtype = result.group('cxxtype')
        self.jstype = result.group('jstype')
        for s in li[1:]:
            result = re.match(ValueObject.__field_re, '.'+s.strip())
            self.field_arr.append((result.group('js_field'),result.group('cxx_field_getter_index'),result.group('cxx_setter')))

class Function:
    RE = r'function\(\"(?P<js_func>.*)\",\s*(?P<cxx_funcs_policies>.*)\)'
    
    def make(self, data):
        result = re.match(Function.RE, data)
        self.js_func = result.group('js_func')
        self.cxx_funcs_policies = result.group('cxx_funcs_policies')

class Lexer:
    tokens = (
        'NEWLINE',
        'CLASS',
        'VECTOR',
        'MAP',
        'ENUM',
        'CONSTANT',
        'VALUE_ARRAY',
        'VALUE_OBJECT',
        'FUNCTION',
        'MACRO_DEFINE',
        'INCLUDE',
        'COMMENT'
    )

    t_ignore = ' \t;'
    t_ignore_NEWLINE = r'\n+'
    t_ignore_COMMENT = r'(/\*(.|\n)*?\*/)|(//.*)'

    def __init__(self):
        self.inlcudes = []
        self.classes = []
        self.vectors = []
        self.maps = []
        self.enums = []
        self.constants = []
        self.value_arrays = []
        self.value_objects = []
        self.functions = []
    
    #def t_NEWLINE(self, t):
    #    r'\n+'
    #    t.lexer.lineno += len(t.value)

    # def t_COMMENT(self,t):
    #     r'(/\*(.|\n)*?\*/)|(//.*)'
    #     print '~~~~~~~~~~~~~~~~~~~~~~~~'
    #     print t.value
    #     print '~~~~~~~~~~~~~~~~~~~~~~~~'
    #     # t.lexer.skip(len(t.value))
    #     return t


    @TOKEN(Class.RE)
    def t_CLASS(self,t):
        # print t.value
        obj = Class()
        obj.make(t.value)
        self.classes.append(obj)
        return t

    @TOKEN(Vector.RE)
    def t_VECTOR(self, t):
        #print t.value
        result = re.match(Vector.RE, t.value)
        obj = Vector(result.group('cxxelemtype'),result.group('jstype'))
        obj.make_class()
        self.vectors.append(obj)
        return t

    @TOKEN(Map.RE)
    def t_MAP(self, t):
        #print t.value
        result = re.match(Map.RE, t.value)
        obj = Map(result.group('cxxkeytype'), result.group('cxxvaltype'), result.group('jstype'))
        obj.make_class()
        self.maps.append(obj)
        return t

    @TOKEN(Enum.RE)
    def t_ENUM(self, t):
        #print t.value
        obj = Enum()
        obj.make(t.value)
        self.enums.append(obj)
        return t
    
    @TOKEN(Constant.RE)
    def t_CONSTANT(self, t):
        # print t.value
        result = re.match(Constant.RE, t.value)
        obj = Constant(result.group('jsval'), result.group('cxxval'))
        self.constants.append(obj)
        return t
    
    @TOKEN(ValueArray.RE)
    def t_VALUE_ARRAY(self, t):
        # print t.value
        obj = ValueArray()
        obj.make(t.value)
        self.value_arrays.append(obj)
        return t
    
    @TOKEN(ValueObject.RE)
    def t_VALUE_OBJECT(self, t):
        # print t.value
        obj = ValueObject()
        obj.make(t.value)
        self.value_objects.append(obj)
        return t
    
    @TOKEN(Function.RE)
    def t_FUNCTION(self, t):
        #print t.value
        obj = Function()
        obj.make(t.value)
        self.functions.append(obj)
        return t

    def t_error(self,t):
        # print("Illegal character '%s'(%d)" %(t.value[0], t.lineno))
        t.lexer.skip(1)

    def lexing(self, data, **kwargs):
        self.lexer = lex.lex(module=self, **kwargs)
        self.lexer.input(data)
        while True:
            tok = self.lexer.token()
            if not tok:
                break
            # print tok
    
    def napi_compile(self, path, include):

        print '\n-----------------napi compile-------------'
        generator = Gen(path, include)
        generator.parse_class(self.classes)
        generator.parse_constant(self.constants)
        generator.parse_objects(self.value_objects)
        generator.parse_arrays(self.value_arrays)
        generator.genfile_start()
        generator.genfile_end()
        # generator.generate_gyp()

        # print include
        # for obj in self.constants:
        #     print obj
            # print obj.jsval
            # print obj.cxxval


        return ""
