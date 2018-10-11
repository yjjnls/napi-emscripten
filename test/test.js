'use strict';
const bindingPath = require.resolve(`./plugin/build/Release/plugin.node`);
console.log(bindingPath)
const cv = require(bindingPath);

const assert = require('assert');

// var p= cv.Mat
// console.log(p)

// let Size = new cv.Size()
// // console.log(Size)
// console.log(Size.width)
// console.log(Size.height)
// Size.width = 11
// Size.height = 22
// console.log(Size.width)
// console.log(Size.height)
// // let mat = new cv.Mat(Size, cv.CV_8UC3);
let mat = new cv.Mat({ height: 10, width: 5 }, cv.CV_8UC3);

// // let mat = new cv.Mat(2, 2, cv.CV_8UC4, new cv.Scalar(1));
// // let mat = new cv.Mat(2, 2, cv.CV_8UC4, new cv.Scalar(1, 0.3, 1, 0));
// let mat = new cv.Mat(2, 2, cv.CV_8UC4, [5, 4, 3, 2]);
// // let mat = new cv.Mat(2, 2, cv.CV_8UC4);

// console.log(mat.type())
// console.log(mat.depth())
// console.log(mat.channels())
// // let element = mat.ptr(0, 0);
// // console.log('------element------')
// // console.log(element[0])
// // console.log(element[1])
// // console.log(element[2])
// // console.log(element[3])
// // console.log('------element------')
// // assert.equal(element[0], 0);
// // assert.equal(element[1], 1);
// // assert.equal(element[2], 2);
// // assert.equal(element[3], 3);

// console.log('mat empty: ' + mat.empty())
// let size = mat.size()
// console.log(size.height)
// console.log(size.width)
// // console.log('-------------------------')
// // let mat2 = new cv.Mat(mat);
// // console.log(mat2.type())
// // console.log(mat2.depth())
// // console.log(mat2.channels())
// // console.log(mat2.empty())
// // let size2 = mat2.size()
// // console.log(size2)
// // console.log(size2.height)
// // console.log(size2.width)

// // assert.equal(mat.type(), cv.CV_8UC3);
// // assert.equal(mat.depth(), cv.CV_8U);
// // assert.equal(mat.channels(), 3);
// // assert.ok(mat.empty() === false);

// // let size = mat.size();
// // assert.equal(size.height, 10);
// // assert.equal(size.width, 20);

// mat.delete();
// let mat = cv.ones(5, 5, cv.CV_8UC1);
// let mat = cv.eye(4, 4, cv.CV_8UC1);
// let mat = cv.zeros(10, 10, cv.CV_8UC1);


assert.equal(mat.type(), cv.CV_8UC1);
assert.equal(mat.depth(), cv.CV_8U);
assert.equal(mat.channels(), 1);
console.log(mat.empty());

let size = mat.size();
console.log(size.height);
console.log(size.width);

mat.delete();
// mat2 = null
// size2 = null
size = null
mat = null;


// console.log(new cv.Scalar(1))
// console.log(new cv.Scalar(1, 0.3, 1, 0))

global.gc();