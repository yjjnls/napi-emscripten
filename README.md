# napi-emscripten
Implementation of webassembly code interfaces based on [node.js c++ napi-addon](https://nodejs.org/dist/latest-v11.x/docs/api/n-api.html).

Webassembly code is generated using [`emscripten`](https://kripken.github.io/emscripten-site/docs/introducing_emscripten/about_emscripten.html)  , which could be invoked by front end or node.js. However, it may not be such efficient as native node.js addon and has some limitations in using. Here we use native node.js addon to implement it **with the same webassembly interfaces**. **Hence, the js application code will be the same as webassembly, but owns more efficiency.**

## Principle
### Emscripten
Emscripten is an Open Source LLVM to JavaScript compiler. With emscripten, we mostly compile C and C++ code into JavaScript, avoiding the high cost of porting code manually.

[Emscripten Compiler Frontend (emcc)](https://kripken.github.io/emscripten-site/docs/tools_reference/emcc.html#emccdoc) acts as a drop-in replacement for a standard compiler like *gcc*.

![](https://kripken.github.io/emscripten-site/_images/EmscriptenToolchain.png)

There are several ways to [connect C/C++ code and JavaScript](https://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html#interacting-with-code). [WebIDL Binder](https://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/WebIDL-Binder.html) and [Embind](https://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/embind.html) are the main two tools for porting C++ code, allowing C++ code entities to be used in a natural manner from JavaScript. They could also dela with `name-mangled C++ functions`.

Both tools create bindings between C++ and JavaScript, but they operate at different levels, and use very different approaches for defining the binding:

*   Embind declares bindings within the C/C++ file.
*   WebIDL-Binder declares the binding in a separate file. This is run through the binder tool to create "glue" code that is then compiled with the project.

Here, we choose `Embind` for porting C++ projects. There's usually a `binding.cpp` additional to the project, using `EMSCRIPTEN_BINDINGS()` blocks to create bindings for functions, classes, value types, pointers (including both raw and smart pointers), enums, and constants.

### N-API
The `binding.cpp` is also used here to define the interfaces for calling C++ from JavaScript. It will be parsed to structed information to generate the C++ wrapping code. The the C++ wrapping code will be compiled to a node plugin module using `node-gyp`. Then JavaScript code could use the functions and classes in the module. A glue js code may also be required. The main principle is shown as below:

![](https://www.draw.io/?lightbox=1&highlight=0000FF&edit=_blank&layers=1&nav=1&title=napi-emscripten1.html#R1Vpdk5owFP01PuoABsTHXd1tp%2B12OrOd2e5jhCykRcKGuGp%2FfW9MEAlaP0YFfYHcJJdwz7kn4Y6d%2Fmi6%2BMRxFj%2BxkCQdxwoXnf644ziOZbtwkZalsniWrwwRp6Ey2aXhmf4l2mhp64yGJK8MFIwlgmZVY8DSlASiYsOcs3l12BtLqk%2FNcERqhucAJ3XrCw1FrKy%2BMyjtnwmN4uLJtjdUPVNcDNZvksc4ZPMNU%2F%2Bh0x9xxoS6my5GJJHBK%2BKi5j3u6F0vjJNUHDLh7c%2FTu%2F%2BSP95Poq9iOPpI3XHS1V4%2BcDLTL9xxvAT83b8xcAurFksdCu99xoqObr4C6g4G2H62KDtliHFQnfBtFtAQQ8%2BIpTkDbxujvUheewEsuxdkmbzgYgUTXvTnbMbBqUQ5JPXuwgKz1aoLs1N5AYezWRoSGQ4buucxFeQ5U6udA3vBFotport1YAgXZLEz4vYaR0gAwqZE8CUMKSb4GnrN%2Fb7vqfa8ZJKN9Jh4g0WetmFN3mjtusQXbjTER8DttALuCU1DmkYK8HZC1x0Y0FlWw9D1WwGdDHYG2Bm52Db4apnXOHzuFviMIJE0vJM7FrSCBOc5DapxgXfny1%2FQsIrGq2z0XNlcUPGrGAf3ZY96DAlr25wRS1iKUtm96iEwj4jYR9U6Nhuxd7eEvrBxkmBBP6rL3YaHfsIPRlf835G5LjIgVa%2BpZ23ul6Yjp%2BoIDQ1HKg41Ryt6rF%2F7dMZ4Z094lZzZUsQsPTRzIQdFlYacwBPwZDVAMjGTAViFxL3vuGOw4JlgahWrCTihUQr3CXmTrmRiUzhg3WmzYDL%2Fc5AD0JWfsjHuovPIwNAxVADVVGCwhYnOpURg0AoNn7JwlpBe2mL9di1Dv72m9du%2FrH4frtJ71XfQpPoiEzhTNA9VX2RXHUHyXlV9h0fhnbKUGGBfY0NudKPtGxvt2u%2FRUO%2FjzIWhLpjWbqx3COaVsDbP06ceqhrH%2BvwFD7Vlyt20Gy0P%2FqC96XMVsgbG9lw%2FWA2vebCy21HYINM84DQTZH28LotUMcEh4dIvTUh%2Bu0Us5Dd9FLO3lULap9dOo4Jtfry6wxsVbNSKzP4CGWtFcgntLnyZH07IbTxbz1%2FHOAXA7wBa73feVtw8v7qjosYLlva2YsWpX7zrRr1iadVEOMR5vA79ORVZM7HdX9ADZAiuifChyj0YGo7sKyv3OUsmt0UgtWk1xSDP93pe%2BTOExfNOo5Nn8BLZVs%2FefAy6LruOK9DsKcgpQhX82jwjllx71Vy7GLvQLcjT%2F8llfuCdTC4XXYZc0Cz%2FcaGGl%2F9b6T%2F8Aw%3D%3D)

```
graph TB
id0(binding.cpp)
id1("napi-emscripten") 
id2(wrapping code)
id3(".c/.cpp/.a (source code)")
id4("emscripten header files")
id5("node-gyp")
id6("module.node")
id7("Js glue code")
id8("Node.js")
id0-->id1
id1-->id2
id2-->id5
id3-->id5
id4-->id5
id5-->id6
id8-.->id6
id8-.->id7
id7-.->id6
```

## Details
### Type conversion
The generated wrapped code declares the napi-addon properties and methods that defined in the `binding.cpp`. Hence the 


| C++ type            | JavaScript type                                                  |
| ------------------- | ---------------------------------------------------------------- |
| void                | undefined                                                        |
| bool                | true or false                                                    |
| char                | Number                                                           |
| signed char         | Number                                                           |
| unsigned char       | Number                                                           |
| short               | Number                                                           |
| unsigned short      | Number                                                           |
| int                 | Number                                                           |
| unsigned int        | Number                                                           |
| long                | Number                                                           |
| unsigned long       | Number                                                           |
| float               | Number                                                           |
| double              | Number                                                           |
| std::string         | ArrayBuffer, Uint8Array, Uint8ClampedArray, Int8Array, or String |
| **emscripten::val** | **anything**                                                     |
| class               | class, object, array                                             |
| pointer             | Number                                                           |
| char \*             | String                                                           |
|||


### Val implementation

### Code generation

1.大致流程
2.流程图

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


- [x] new cv.Mat.ones

- [x] cv.RotatedRect.points(rect);