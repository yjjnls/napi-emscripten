import sys
sys.path.append('../../source')

import preprocessor
p = preprocessor.Preprocessor()
p.input("./core_bindings.cpp")
# p.input("./binding.cpp")
p.compile()
