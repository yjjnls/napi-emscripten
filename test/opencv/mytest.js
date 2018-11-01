'use strict';
// const bindingPath = require.resolve(`./plugin/build/Release/plugin.node`);
const bindingPath = require.resolve(`./plugin/opencv.js`);
const cv = require('./plugin/opencv.js');

const assert = require('assert');

let mat1 = cv.ones(9, 9, cv.CV_8UC3);
let mat2 = new cv.Mat();

cv.medianBlur(mat1, mat2, 3);

// Verify result.
let size = mat2.size();
assert.equal(mat2.channels(), 3);
assert.equal(size.height, 9);
assert.equal(size.width, 9);

mat1 = null
mat2 = null

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