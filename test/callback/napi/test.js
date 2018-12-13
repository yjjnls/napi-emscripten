'use strict';
const bindingPath = require.resolve(`./plugin/build/Release/plugin.node`);
// const bindingPath = require.resolve(`./plugin/opencv.js`);
const Module = require(bindingPath);

Module.calculate(0, (num) => {
    console.log('callback: ' + 0 + '+1 = ' + num);
})