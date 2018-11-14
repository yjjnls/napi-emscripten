'use strict';
// const bindingPath = require.resolve(`./plugin/build/Release/plugin.node`);
const bindingPath = require.resolve(`./plugin/opencv.js`);
const cv = require('./plugin/opencv.js');

const assert = require('assert');

var ref = require('ref');


console.log('---------1----------')
let vec1 = cv.Mat.ones(new cv.Size(20, 20), cv.CV_8UC1); // eslint-disable-line new-cap
console.log('---------2----------')
let source = new cv.MatVector();
console.log('---------3----------')
source.push_back(vec1);
console.log('---------4----------')
let channels = [0];
let histSize = [256];
let ranges =[0, 256];

let hist = new cv.Mat();
console.log('---------5----------')
let mask = new cv.Mat();
let binSize = cv._malloc(4);
let binView = new Int32Array(cv.HEAP8.buffer, binSize);
console.log('---------6----------')
binView[0] = 10;
cv.calcHist(source, channels, mask, hist, histSize, ranges, false);
console.log('---------7----------')

// hist should contains a N X 1 arrary.
let size = hist.size();
assert.equal(size.height, 256);
assert.equal(size.width, 1);

// default parameters
cv.calcHist(source, channels, mask, hist, histSize, ranges);
size = hist.size();
assert.equal(size.height, 256);
assert.equal(size.width, 1);

// Do we need to verify data in histogram?
// let dataView = hist.data;

// Free resource
cv._free(binSize);
mask.delete();
hist.delete();




// let rect = new cv.RotatedRect();

// assert.equal(rect.center.x, 0);
// assert.equal(rect.center.y, 0);
// assert.equal(rect.angle, 0);
// assert.equal(rect.size.height, 0);
// assert.equal(rect.size.width, 0);
// let points = cv.RotatedRect.points(rect);

// assert.equal(points[0].x, 0);
// assert.equal(points[0].y, 0);
// assert.equal(points[1].x, 0);
// assert.equal(points[1].y, 0);
// assert.equal(points[2].x, 0);
// assert.equal(points[2].y, 0);
// assert.equal(points[3].x, 0);
// assert.equal(points[3].y, 0);

global.gc();
