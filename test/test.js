'use strict';
// const bindingPath = require.resolve(`./plugin/build/Release/plugin.node`);
const bindingPath = require.resolve(`./plugin/opencv.js`);
console.log(bindingPath)
const cv = require('./plugin/opencv.js');

const assert = require('assert');

let src = cv.ones(4, 4, cv.CV_8UC1);

src.data[2] = 0;
src.data[5] = 2;

let result = cv.minMaxLoc(src);

assert.equal(result.minVal, 0);
assert.equal(result.maxVal, 2);
assert.deepEqual(result.minLoc, {x: 2, y: 0});
assert.deepEqual(result.maxLoc, {x: 1, y: 1});

src.delete();

global.gc();