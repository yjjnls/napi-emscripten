const Module = require('./build/binding.js');
Module.onRuntimeInitialized = function () {
    Module.calculate(0, (num) => {
        console.log('callback: ' + 0 + '+1 = ' + num);
    })
}