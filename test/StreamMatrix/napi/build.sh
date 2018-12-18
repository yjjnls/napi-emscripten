# export CC=clang 
# export CXX=clang++ 
# export CC=gcc-7
# export CXX=g++-7
# export LINKXX=clang++ 
# export LINK=clang

node-gyp clean 

mkdir -p plugin/build 
cd  plugin/build 

node-gyp configure --arch=x86_64 -C ..
node-gyp build  -C ..

cd -