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

## build from docker
use docker yjjnls/emcc-docker:0.1

```sh
# load env
cd ~/emsdk; source emsdk_env.sh
export EMSCRIPTEN_VERSIONS=1.38
export CONAN_BUILD_TYPES=Release
export CONAN_ARCHS=x86_64 
export CONAN_USERNAME=conanos
export CONAN_CHANNEL=stable

# emcc build to llvm bitcode
cd ~; git clone https://github.com/yjjnls/zlib.git
cd zlib && git checkout emcc/1.2.11
python build.py

cd ~; git clone https://github.com/yjjnls/libpng.git
cd libpng && git checkout emcc/1.6.34
python build.py

cd ~; git clone https://github.com/yjjnls/libtiff.git
cd libtiff && git checkout emcc/4.0.9
python build.py

cd ~; git https://github.com/yjjnls/libjpeg-turbo.git
cd libjpeg-turbo && git checkout emcc/1.5.2
python build.py

cd ~; git clone https://github.com/yjjnls/leptonica.git
cd leptonica && git checkout emcc/1.76.0
python build.py

cd ~; git clone https://github.com/yjjnls/tesseract.git
cd tesseract && git checkout emcc/4.0.0
python build.py

# copy conan package to dir

# build tesseract
./build.sh
```