'use strict';
const bindingPath = require.resolve(`./plugin/build/Release/plugin.node`);
// const bindingPath = require.resolve(`./plugin/opencv.js`);
const Module = require(bindingPath);

const assert = require('assert');

let api = new Module.TessBaseAPI();
api.Init("/opt", "eng");

// let image = Module.pixRead("./cpp/test.bmp");
let image = Module.pixRead("../cosmic.png");
api.SetImage(image);

let result = api.GetUTF8Text();

console.log(result);

Module.pixDestroy(image);

api.End();
api.delete();
api = null;

global.gc();