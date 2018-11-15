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
Basic types in c++, such as char, int, float, double, string and so on could be transfered between js and c++. As these types have already been declared [napi_valuetype](https://nodejs.org/dist/latest-v11.x/docs/api/n-api.html#n_api_napi_valuetype). But other types defined in `binding.cpp` should also be transfered to napi type. 

    c++ type <-----> napi_value

#### class
Registered classes are defined as c++ classes in the generated file, and they are declared with `napi_define_class` as wrapped js classes.   

For example, a class named `class_Mat` is corresponding to `cv::Mat` in the source code. 
*   Prefix 'class_' means this c++ class is a class type in js.
*   `class_Mat` owns a pointer to instance `cv::Mat` to invoke its methods.
*   Napi properities defined in `class_Mat` will invoked corresponding constructors, member properties, functions, static functions of `cv::Mat` when they're invoked from js code.
    *   mat.rows(js) ----> class_Mat::getrows/class_Mat::setrows ----> cv::Mat::rows
    *   mat.elemSize(js) ----> class_Mat::elemSize ----> cv::Mat::elemSize
*   As function argument: unwrap from the napi_value to get `cv::Mat`. 
*   As return type: create a napi instance containing `cv::Mat` the c++ returned.

#### value_object
Registered value_objects are also defined as napi class type in the generated file, but they represent js objects.

For example, a class named `object_Range` is corresponding to `cv::Range`. And it's almostly the same as register classes.
*   Prefix 'object_' means this c++ class is a object type in js.
*   `object_Range` owns a pointer to instance `cv::Range` to invoke its methods.
*   Fields defined in value_object are declared napi_properities in `object_Range`.
*   As return type: create a napi object and set all the properities as fields defined.
*   As function argument: crate a napi instance of `cv::Range` and set all the properities extracted from napi_value.

#### value_array
Registered value_arrays are defined as napi_array type in the generated file, and declared as a global napi method for invoking.
*   It has fixed length as defined in declaration.
*   In source code, its type is a class but not really an array.
*   As return type: create a napi_array and set all the elements.
*   As function argument: get all elements from the napi_array and create the class instance.

#### constant
Contants are defined as global napi properities in the generated file and the properity functions return the constant value in source code.

#### global function
Similar to constant, global function are also global napi properities.

#### vector
Register vectors are defined as napi classes in the generated file.

For example, a class named `vecMatVector` is corresponding to `MatVector`.
*   Prefix 'vec' means this c++ class is a object type in js.
*   `vecMatVector` owns a pointer to instance `std::vector<cv::Mat>` to invoke its methods.
*   `vecMatVector` only has 5 methods: push_back, resize, size, get, set
*   As function argument: unwrap from the napi_value to get `std::vector<cv::Mat>`. 
*   As return type: create a napi instance containing `cv::Mat` the c++ returned.

#### val
`val` is the common type implementation in emscripten, which can represents various js types, such as undefined, number, string, array, etc. And it's not expilictly registered in `binding.cpp`. Hence, you should expilictly note the real type it represents.

`val` now is mostly used as array, unlike vector, it represents the raw array. 
*   As function argument: get the element value from napi_value and store in `std::vector<>`, then create `val` using `val::array`
*   As return type: create `napi_typedarray_type` and set data buffer from `val`

If `val` represents a single type, get the value/buffer and operate as types memtioned above.

### Memory manager
1. value created in js code  
The jvm will manage the memory, when there's no reference to the variable, it will be released.

2. return value created in c++  
All the returned values are `napi_value`, that means jvm will also release them.

3. variable created in c++   
Napi class will own a pointer pointing to c++ instance. This instance is dynamicly allocated, but it will be released when the desrtuctor function invoked.



## Probelms left
~~obtain additional information and generate project file~~

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

- [x] synax in js class instance   
cv.Mat.ones() has synax error in node.js

- [x] gcc compile   

- [x] return val   
return type of val( array of objects ) is ambigous. If the element type is basic type, int, float,etc, val should be transfered to napi typedarray(napi_int32_array, etc.).


- [ ] new cv.Mat.ones

- [x] cv.RotatedRect.points(rect);