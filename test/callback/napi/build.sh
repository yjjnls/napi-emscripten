export CC=clang 
export CXX=clang++ 
# export CXX=g++-5
export LINKXX=clang++ 
export LINK=clang

node-gyp clean 

mkdir -p plugin/build 
cd  plugin/build 

node-gyp configure --arch=x86_64 -C ..
node-gyp build  -C ..

cd -