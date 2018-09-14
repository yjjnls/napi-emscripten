import sys
sys.path.append('../source')

import preprocessor
p = preprocessor.Preprocessor()
p.input("./core_bindings.cpp")
p.compile()

#f = open("./class_.cpp")
#data = f.read()
#f.close()
#
#lexer = emscripten.Lexer()
#lexer.lexing(data)