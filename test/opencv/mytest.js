'use strict';
// const bindingPath = require.resolve(`./plugin/build/Release/plugin.node`);
const bindingPath = require.resolve(`./plugin/opencv.js`);
const cv = require('./plugin/opencv.js');

const assert = require('assert');

let data = new Uint8Array([0, 0, 0, 255, 0, 1, 2, 3]);
console.log('"let data = new Uint8Array([0, 0, 0, 255, 0, 1, 2, 3]);"')
console.log(data)
console.log(typeof data)
console.log(data.buffer)
console.log(typeof data.buffer)
let dataPtr = cv._malloc(8);
console.log('\n"let dataPtr = cv._malloc(8);"')
console.log(dataPtr)
console.log(typeof dataPtr)
console.log('\n"cv.HEAPU8.buffer"')
console.log(cv.HEAPU8.buffer)
console.log(typeof cv.HEAPU8.buffer)
// console.log('"cv.HEAPU8.buffer"')
// console.log(cv.HEAPU8)
// console.log(typeof cv.HEAPU8)
let dataHeap = new Uint8Array(cv.HEAPU8.buffer, dataPtr, 8);
console.log('\n"let dataHeap = new Uint8Array(cv.HEAPU8.buffer, dataPtr, 8);"')
console.log(dataHeap)
console.log(typeof dataHeap)
dataHeap.set(new Uint8Array(data.buffer));
console.log(dataHeap)
console.log(typeof dataHeap)

console.log(dataPtr)
console.log(typeof dataPtr)
let mat = new cv.Mat(8, 1, cv.CV_8UC1, dataPtr, 0);

assert.equal(mat.ucharAt(0), 0);
assert.equal(mat.ucharAt(1), 0);
assert.equal(mat.ucharAt(2), 0);
assert.equal(mat.ucharAt(3), 255);
assert.equal(mat.ucharAt(4), 0);
assert.equal(mat.ucharAt(5), 1);
assert.equal(mat.ucharAt(6), 2);
assert.equal(mat.ucharAt(7), 3);

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