'use strict';
// const bindingPath = require.resolve(`./plugin/build/Release/plugin.node`);
const bindingPath = require.resolve(`./plugin/opencv.js`);
const cv = require('./plugin/opencv.js');

const assert = require('assert');

var ref = require('ref');

let rect = new cv.RotatedRect({x: 100, y: 100}, {height: 100, width: 50}, 30);

assert.equal(rect.center.x, 100);
assert.equal(rect.center.y, 100);
assert.equal(rect.angle, 30);
assert.equal(rect.size.height, 100);
assert.equal(rect.size.width, 50);

global.gc();
