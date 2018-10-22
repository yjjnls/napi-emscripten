'use strict';
// const bindingPath = require.resolve(`./plugin/build/Release/plugin.node`);
const bindingPath = require.resolve(`./plugin/opencv.js`);
const cv = require('./plugin/opencv.js');

const assert = require('assert');

let rectVector = new cv.RectVector();
let rect = {x: 1, y: 2, width: 3, height: 4};
rectVector.push_back(rect);
rectVector.push_back(new cv.Rect());
rectVector.push_back(new cv.Rect(rect));
// rectVector.push_back(new cv.Rect({x: 5, y: 6}, {width: 7, height: 8}));
rectVector.push_back(new cv.Rect(9, 10, 11, 12));

assert.equal(rectVector.size(), 4);

let item = rectVector.get(0);
console.log(item)
assert.equal(item.x, 1);
assert.equal(item.y, 2);
assert.equal(item.width, 3);
assert.equal(item.height, 4);

item = rectVector.get(1);
assert.equal(item.x, 0);
assert.equal(item.y, 0);
assert.equal(item.width, 0);
assert.equal(item.height, 0);

item = rectVector.get(2);
assert.equal(item.x, 1);
assert.equal(item.y, 2);
assert.equal(item.width, 3);
assert.equal(item.height, 4);

// item = rectVector.get(3);
// assert.equal(item.x, 5);
// assert.equal(item.y, 6);
// assert.equal(item.width, 7);
// assert.equal(item.height, 8);

item = rectVector.get(3);
assert.equal(item.x, 9);
assert.equal(item.y, 10);
assert.equal(item.width, 11);
assert.equal(item.height, 12);

rectVector.delete();

global.gc();


// const R =7;
// const G =13;
// const B =29;

// let mat = cv.ones(5, 5, cv.CV_8UC3);
// let view = mat.data;
// view[0] = R;
// view[1] = G;
// view[2] = B;

// let bgrPlanes = new cv.MatVector();
// cv.split(mat, bgrPlanes);
// assert.equal(bgrPlanes.size(), 3);

// let rMat = bgrPlanes.get(0);
// view = rMat.data;
// assert.equal(view[0], R);