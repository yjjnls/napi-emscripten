'use strict';
// const bindingPath = require.resolve(`./plugin/build/Release/plugin.node`);
const bindingPath = require.resolve(`./plugin/opencv.js`);
const cv = require('./plugin/opencv.js');

const assert = require('assert');


// let data = cv._malloc(10 * 10 * 1);
// let mat = new cv.Mat(10, 10, cv.CV_8UC1, data, 0);

// assert.equal(mat.type(), cv.CV_8UC1);
// assert.equal(mat.depth(), cv.CV_8U);
// assert.equal(mat.channels(), 1);
// assert.ok(mat.empty() === false);

// let size = mat.size();
// assert.ok(size.height === 10);
// assert.ok(size.width === 10);
// console.log(mat.ucharAt(0))
// mat.delete();

//     let data = new Uint8Array([0, 0, 0, 255, 0, 1, 2, 3]);
//     let dataPtr = cv._malloc(8);
// console.log("-----1------")
//     let dataHeap = new Uint8Array(cv.HEAPU8.buffer, dataPtr, 8);
// console.log("-----2------")
// dataHeap.set(new Uint8Array(data.buffer));
// console.log("-----3------")

//     let mat = new cv.Mat(8, 1, cv.CV_8UC1, dataPtr, 0);
//     console.log("-----4------")

//     assert.equal(mat.ucharAt(0), 0);
//     assert.equal(mat.ucharAt(1), 0);
//     assert.equal(mat.ucharAt(2), 0);
//     assert.equal(mat.ucharAt(3), 255);
//     assert.equal(mat.ucharAt(4), 0);
//     assert.equal(mat.ucharAt(5), 1);
//     assert.equal(mat.ucharAt(6), 2);
//     assert.equal(mat.ucharAt(7), 3);


let mat = new cv.Mat(10, 20, cv.CV_8UC3);

assert.equal(mat.type(), cv.CV_8UC3);
assert.equal(mat.depth(), cv.CV_8U);
assert.equal(mat.channels(), 3);
assert.ok(mat.empty() === false);

let size = mat.size();
assert.equal(size.height, 10);
assert.equal(size.width, 20);

mat.delete();


mat.delete();
mat2.delete();

global.gc();
