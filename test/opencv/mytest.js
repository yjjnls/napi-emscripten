'use strict';
// const bindingPath = require.resolve(`./plugin/build/Release/plugin.node`);
const bindingPath = require.resolve(`./plugin/opencv.js`);
const cv = require('./plugin/opencv.js');

const assert = require('assert');

const R =7;
const G =13;
const B =29;

let mat = cv.Mat.ones(5, 5, cv.CV_8UC3);
console.log("----------1-----------")
let view = mat.data;
console.log("----------2-----------")
view[0] = R;
view[1] = G;
view[2] = B;

console.log("----------3-----------")
let bgrPlanes = new cv.MatVector();
console.log("----------4-----------")
cv.split(mat, bgrPlanes);
console.log("----------5-----------")
assert.equal(bgrPlanes.size(), 3);

let rMat = bgrPlanes.get(0);
view = rMat.data;
assert.equal(view[0], R);

let gMat = bgrPlanes.get(1);
view = gMat.data;
assert.equal(view[0], G);

let bMat = bgrPlanes.get(2);
view = bMat.data;
assert.equal(view[0], B);

mat.delete();
rMat.delete();
gMat.delete();
bgrPlanes.delete();
bMat.delete();

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