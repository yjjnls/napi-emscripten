'use strict';
// const bindingPath = require.resolve(`./plugin/build/Release/plugin.node`);
const bindingPath = require.resolve(`./plugin/opencv.js`);
const cv = require('./plugin/opencv.js');

const assert = require('assert');

let data = new Uint8Array([0, 0, 0, 255, 0, 1, 2, 3]);
let dataPtr = cv._malloc(8);
console.log(dataPtr)
console.log(typeof(dataPtr))

let dataHeap = new Uint8Array(cv.HEAPU8.buffer, dataPtr, 8);
dataHeap.set(new Uint8Array(data.buffer));

let mat = new cv.Mat(8, 1, cv.CV_8UC1, dataPtr, 0);


let unsignedCharView = new Uint8Array(data.buffer);
let charView = new Int8Array(data.buffer);
let shortView = new Int16Array(data.buffer);
let unsignedShortView = new Uint16Array(data.buffer);
let intView = new Int32Array(data.buffer);
let float32View = new Float32Array(data.buffer);
let float64View = new Float64Array(data.buffer);


assert.deepEqual(unsignedCharView, mat.data);
assert.deepEqual(charView, mat.data8S);
assert.deepEqual(shortView, mat.data16S);
assert.deepEqual(unsignedShortView, mat.data16U);
assert.deepEqual(intView, mat.data32S);
assert.deepEqual(float32View, mat.data32F);
assert.deepEqual(float64View, mat.data64F);

global.gc();


// const R = 7;
// const G = 13;
// const B = 29;

// console.log("--------  let mat = cv.ones(5, 5, cv.CV_8UC3);   --------")
// let mat = cv.ones(5, 5, cv.CV_8UC3);
// console.log("--------  let view = mat.data;   --------")
// let view = mat.data;
// view[0] = R;
// view[1] = G;
// view[2] = B;

// console.log("--------  let bgrPlanes = new cv.MatVector();   --------")
// let bgrPlanes = new cv.MatVector();
// console.log("--------  cv.split(mat, bgrPlanes);   --------")
// cv.split(mat, bgrPlanes);
// console.log("--------  assert.equal(bgrPlanes.size(), 3);   --------")
// assert.equal(bgrPlanes.size(), 3);
// console.log("--------  let rMat = bgrPlanes.get(0);   --------")
// let rMat = bgrPlanes.get(0);
// console.log("--------  view = rMat.data;   --------")
// view = rMat.data;
// assert.equal(view[0], R);