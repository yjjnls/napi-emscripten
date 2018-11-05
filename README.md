# napi-emscripten
Implementation of webassembly code interfaces based on node.js c++ napi-addon.

## Principle
* webassembly  
`c/c++ source code` + `binding.cpp` ---> bitcode ---> webassembly code(`.wasm`, `.js`) <--- node.js/html   
                              
* napi  
`c/c++ source code` + `binding.cpp` ---> plugin.cpp ---> node addon(`.node`) <--- node.js/html   
                                    
Webassembly code is generated using `emscripten`, which could be invoked by front end or node.js. However, it may not be such efficient as native node.js addon and has some limitations in using. Here we use native node.js addon to implement it with the same webassembly interfaces. It generates a wrapping c++ code declaring napi method and classes, according to the source code, binding.cpp and supplemental materials by python. Then the code will be built into node addon foramt(.node). A glue js code is also required when invoked by node.js. **Hence, the js application code will be the same as webassembly, but owns more efficiency.**

## Details
### Code analysis
Besides the origin c++ source code, a `binding.cpp` is required when generating webassembly code. Types declared in `binding.cpp` will be included in webassembly code and could be invoked by js eventually. For example, `class Mat` is declared in the binding.cpp of opencv:
```c
    emscripten::class_<cv::Mat>("Mat")
        .constructor<>()
        .constructor<const Mat&>()

        ....
```
Then `Mat` could be used in js:
```js
let mat = new cv.Mat();
```

Python module `ply` is used here to parse c++ synax and obtain type declation information in the `binding.cpp`. It collects the information of registered class, object, array, function, constant, vector and so on.

These information will be transfered into napi declartions, so that these types and methods could be invoked in node.js code.

The information will be stored in python as a dict. For example, if it is a declared class or value_object, the `jstype` and `cxxtype` will be stored, representing for the js type name and c++ type name of it respectively. But the most important information is the function.  
For example: 
```cpp
        .function("convertTo", select_overload<void(const Mat&, Mat&, int, double, double)>(&binding_utils::convertTo))
        .function("convertTo", select_overload<void(const Mat&, Mat&, int)>(&binding_utils::convertTo))
        .function("convertTo", select_overload<void(const Mat&, Mat&, int, double)>(&binding_utils::convertTo))
```

This is a declartion of three overload functions of 'convertTo'. And it will be parsed to the stuct below:
```python
{'convertTo':[('void',['Mat&', 'int'],'binding_utils::convertTo','const Mat&, Mat&, int'),
              ('void',['Mat&', 'int', 'double'],'binding_utils::convertTo','const Mat&, Mat&, int, double'),
              ('void',['Mat&', 'int', 'double', 'double'],'binding_utils::convertTo','const Mat&, Mat&, int, double, double')]}
```

Key `convertTo` is the js name of the function and value is the list of three overload function information. 
*   `'void'`: return type
*   `['Mat&', 'int']`: args list, c++ code will parse args js code passed in according to this list
*   `'binding_utils::convertTo'`: function name
*   `'const Mat&, Mat&, int'`: additional information, default is None

Class constructor, member function, properity, static function, global function and other types will all parse to this format. The detail is shown below.

### Type mapping
#### class
##### 

### Memory manager


## Probelms left
- [ ] obtain additional information and generate project file  

- [ ] memory access   
    in 'test mat access' ,
    ```js
        let dataPtr = cv._malloc(8);
        let dataHeap = new Uint8Array(cv.HEAPU8.buffer, dataPtr, 8);
    ```
    `dataPtr` represents the malloced memory address, and `dataHeap` intends to use the memory that `cv._malloc(8)` malloced as `dataPtr` represents the offset in memory area `cv.HEAPU8.buffer`.
    ```js
    uint8Array = new Uint8Array( buffer, byteOffset, length);
    ```
    `byteOffset` represents the offset (in bytes) of the Uint8Array from the start of its ArrayBuffer. 
    While in napi implementation, `cv.HEAPU8.buffer` and `cv._malloc` create different independent memories.

    https://github.com/kripken/emscripten/issues/7437

- [ ] synax in js class instance   
cv.Mat.ones() has synax error in node.js

- [ ] gcc compile   

- [ ] return val   
return type of val( array of objects ) is ambigous. If the element type is basic type, int, float,etc, val should be transfered to napi typedarray(napi_int32_array, etc.).

