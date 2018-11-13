'use strict';
// const bindingPath = require.resolve(`./plugin/build/Release/plugin.node`);
const bindingPath = require.resolve(`./plugin/opencv.js`);
const cv = require('./plugin/opencv.js');

const assert = require('assert');
let mat = cv.Mat.ones(5, 5, cv.CV_8UC1);
console.log(mat.step)
let res = cv.meanShift(mat, { x: 1, y: 1, width: 2, height: 2 }, { type: 0, maxCount: 1, epsilon: 0.0001 })
assert.deepEqual(res,[ 0, { y: 1, x: 1, height: 2, width: 2 } ])
console.log(res)


mat.delete();

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