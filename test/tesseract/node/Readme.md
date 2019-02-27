`tesseract/leptonica` header files shouldn't placed in `/usr/include`, as `stdio.h` should be coming from emscripten.

https://github.com/apiaryio/emscripten-docker/issues/13

## build leptonica to bit code
git clone https://github.com/DanBloomberg/leptonica.git
cd leptonica; git checkout v1.73
mkdir build; cd build
<!-- emconfigure cmake ..
emmake make -j4
sudo cp src/liblept173.so /usr/local/lib/liblept.so -->

file /usr/local/lib/liblept.so  ==> LLVM IR bitcode

emconfigure ./configure
emmake make -j4
sudo make install
=> file /usr/local/lib/liblept.so.5.0.0  ==> LLVM IR bitcode

## build tesseract to bitcode
git clone https://github.com/tesseract-ocr/tesseract.git
cd tesseract; git checkout 3.04.01
./autogen.sh
LIBLEPT_HEADERSDIR=/usr/local/include emconfigure ./configure --prefix=/usr/local/ --with-extra-libraries=/usr/local/lib --disable-graphics
emmake make -j4
sudo make install

file /usr/local/lib/libtesseract.so.3.0.4  ==> LLVM IR bitcode

## download tessdata
cd /opt && sudo git clone https://github.com/yjjnls/tessdata.git

export TESSDATA_PREFIX=/opt