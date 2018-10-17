'use strict';
// const bindingPath = require.resolve(`./plugin/build/Release/plugin.node`);
const bindingPath = require.resolve(`./plugin/opencv.js`);
const cv = require('./plugin/opencv.js');

const assert = require('assert');

let data = new Uint8Array([0, 0, 0, 255, 0, 1, 2, 3]);
console.log(data)
console.log(data.buffer)
let dataPtr = cv._malloc(8);
console.log(dataPtr)

console.log(new Uint8Array(2))
console.log(cv.HEAPU8.byteOffset)
console.log(cv.HEAPU8.byteOffset)
let dataHeap = new Uint8Array(cv.HEAPU8.buffer, dataPtr, 8);
console.log(dataHeap.buffer)
console.log(dataHeap)
dataHeap.set(new Uint8Array(data.buffer));
console.log(dataHeap)

let mat = new cv.Mat(8, 1, cv.CV_8UC1, dataPtr, 0);


let unsignedCharView = new Uint8Array(data.buffer);
let charView = new Int8Array(data.buffer);
let shortView = new Int16Array(data.buffer);
let unsignedShortView = new Uint16Array(data.buffer);
let intView = new Int32Array(data.buffer);
let float32View = new Float32Array(data.buffer);
let float64View = new Float64Array(data.buffer);

// console.log(unsignedCharView)
// console.log(mat.data)
assert.deepEqual(unsignedCharView, mat.data);
assert.deepEqual(charView, mat.data8S);
assert.deepEqual(shortView, mat.data16S);
assert.deepEqual(unsignedShortView, mat.data16U);
assert.deepEqual(intView, mat.data32S);
assert.deepEqual(float32View, mat.data32F);
assert.deepEqual(float64View, mat.data64F);

global.gc();