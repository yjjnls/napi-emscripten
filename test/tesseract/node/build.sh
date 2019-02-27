mkdir -p build
cd build
# emcc ../../core_bindings.cpp -s WASM=1 --bind -std=c++11 -o tesseract.js -I /usr/local/include -L /usr/local/lib -ltesseract -llept
# emcc ../../core_bindings.cpp -s WASM=0 -s DEMANGLE_SUPPORT=1 --bind -std=c++11 -o tesseract.js -I /usr/local/include -L /usr/local/lib -ltesseract -llept

emcc ../../core_bindings.cpp -s WASM=1 --bind -std=c++11 -o tesseract.js -I /root/Desktop/test/package/include -L /root/Desktop/test/package/lib /root/Desktop/test/package/lib/libtesseract.so /root/Desktop/test/package/lib/libleptonica.so /root/Desktop/test/package/lib/libpng.so /root/Desktop/test/package/lib/libjpeg.so /root/Desktop/test/package/lib/libtiff.so /root/Desktop/test/package/lib/libz.so -DUSE_NODEFS=1 -s ALLOW_MEMORY_GROWTH=1 -s BINARYEN_TRAP_MODE='clamp'

# emcc -s WASM=1 -o tesseract.js -I /usr/local/include -L /usr/local/lib -ltesseract -llept

# emcc /usr/local/lib/libtesseract.so /usr/local/lib/liblept.so -o tesseract.js 
cd -