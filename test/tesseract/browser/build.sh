mkdir -p build
cd build
# emcc ../../core_bindings.cpp -s WASM=1 --bind -std=c++11 -o tesseract.js -I /usr/local/include -L /usr/local/lib -ltesseract -llept
# emcc ../../core_bindings.cpp -s WASM=0 -s DEMANGLE_SUPPORT=1 --bind -std=c++11 -o tesseract.js -I /usr/local/include -L /usr/local/lib -ltesseract -llept
mkdir -p data
if [ ! -d data/tessdata ]; then
    cd data
    git clone https://github.com/yjjnls/tessdata.git
    cd ..
fi
cp -f ../../cosmic.png data

emcc ../../core_bindings.cpp -s WASM=1 --bind -std=c++11 -o tesseract.js \
-I /root/Desktop/test/package/include \
/root/Desktop/test/package/lib/libtesseract.so \
/root/Desktop/test/package/lib/libleptonica.so \
/root/Desktop/test/package/lib/libpng.so \
/root/Desktop/test/package/lib/libjpeg.so \
/root/Desktop/test/package/lib/libtiff.so \
/root/Desktop/test/package/lib/libz.so \
--no-heap-copy -s ALLOW_MEMORY_GROWTH=1 -s BINARYEN_TRAP_MODE='clamp' -s DEMANGLE_SUPPORT=1 \
--preload-file data

cd -
cp -f build/tesseract.data .