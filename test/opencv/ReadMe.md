## Start
```sh
$ npm install node-qunit

$ python2 test.py
$ ./build.sh
$ LD_LIBRARY_PATH=/usr/local/lib node tests.js
```
## supplement file
1. functions in namespace binding_utils can't be static

2. declared value_objects should be noted in supplements wheather it has constructors with declared fields as args. 

3. template function and class/struct declared in namespace should be noted in json

4. if jsmethod and cxxmethod are different in declartion, it should be noted
```cpp
#define EMSCRIPTEN_CV_RECT(type, name) \
    emscripten::value_object<cv::Rect_<type>> (name) \
        .field("x", &cv::Rect_<type>::x) \
        .field("y", &cv::Rect_<type>::y) \
        .field("width", &cv::Rect_<type>::width) \
        .field("height", &cv::Rect_<type>::height);

    EMSCRIPTEN_CV_RECT(int, "Rect")
    EMSCRIPTEN_CV_RECT(float, "Rect2f")
``` 

5. `val` is commonly JsArray format, it can be napi typedarray, object, number, string or others. It shoud noted according to their function name. In function args, val usually is array type. While in function return type, it can not only be array but also be single number, string or object.




## notes
Args passing to value_object constructor(js code) should be the same order with the declartion(cpp code).
Declared valued_object can be created by these synax:
1. js object format
```js
let rect = {x: 1, y: 2, width: 3, height: 4};
```
2. class instance with no args
```js
let rect = new cv.Rect();
```
3. class instance with all default args
```js
let rect = new cv.Rect(9, 10, 11, 12);
```
4. class instance with copy constructor
```js
let rect = new cv.Rect({x: 1, y: 2, width: 3, height: 4});
```
5. class instance with args format noted in supplement file
```js
```

If the value_object only has default constructor in cpp code. Its information will be noted in supplement file and can only be created in `synax 2`.


---
## problems left
1. 
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

2. 
pointer data in val returned from register_vector `get` method is different from the one when the val is being created.

3. 
array in val