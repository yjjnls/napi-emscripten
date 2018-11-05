# napi-emscripten
Implementation of webassembly code interfaces based on node.js c++ napi-addon.

## procedure
* webassembly
`c/c++ source code` + `binding.cpp` --->bitcode --->webassembly code(`.wasm`,`.js`)<--- node.js/html   
                                    |<----------      emscripten       ---------->|
* napi
`c/c++ source code` + `binding.cpp` --->plugin.cpp --->node addon(`.node`) <--- node.js/html   
                                    |<- python ->| |<----- node-gyp ---->|