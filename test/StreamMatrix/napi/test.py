import sys
sys.path.append('../../../source')

import preprocessor
p = preprocessor.Preprocessor()
p.input("../binding.cpp")
# p.input("./binding.cpp")
p.compile()