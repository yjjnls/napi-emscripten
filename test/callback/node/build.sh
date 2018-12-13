mkdir -p build
cd build
emcc --bind -s WASM=1 -o binding.js ../../binding.cpp
cd -