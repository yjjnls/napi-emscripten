mkdir -p build
cd build
emcc ../../napi/core_bindings.cpp -s WASM=1 --bind -std=c++11 -o tesseract.js -I /usr/local/include -L /usr/local/lib -ltesseract -llept
# emcc -s WASM=1 -o tesseract.js -I /usr/local/include -L /usr/local/lib -ltesseract -llept

# emcc /usr/local/lib/libtesseract.so /usr/local/lib/liblept.so -o tesseract.js 
cd -