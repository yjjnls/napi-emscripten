'use strict'
const chai = require('chai');
let assert = chai.assert;

var StreamMatrix = require("../index.js").StreamMatrix;

let stream_matrix;
describe('WebStreamer', function () {
    describe('#interface', function () {
        beforeEach(async () => {
            stream_matrix = new StreamMatrix();
            await stream_matrix.initialize();
        });

        afterEach(async () => {
            await stream_matrix.terminate();
        });

        it(`version`, async () => {
            let version = await stream_matrix.version();
            console.log(version)
            assert.isString(version);
        });
        it(`version2`, async () => {
            let version = await stream_matrix.version();
            console.log(version)
            assert.isString(version);
        });
    });
    // describe('#interface2', function () {
    //     before(async () => {
    //         console.log("----1")
    //         stream_matrix = new StreamMatrix();
    //         console.log("----2")
    //         await stream_matrix.initialize();
    //         console.log("----3")
    //     });

    //     after(async () => {
    //         await stream_matrix.terminate();
    //     });

    //     it(`version`, async () => {
    //         console.log("----4")
    //         let version = await stream_matrix.version();
    //         assert.isString(version);
    //     });
    // });
});
