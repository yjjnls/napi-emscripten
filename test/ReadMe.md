functions in namespace binding_utils can't be static

declared value_objects should be noted in supplements wheather it has constructors with declared fields as args. 

template function and class/struct declared in namespace should be noted in json

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