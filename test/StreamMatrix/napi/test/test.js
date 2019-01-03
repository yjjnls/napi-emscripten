'use strict'
const chai = require('chai');
let assert = chai.assert;

var StreamMatrix = require("../index.js").StreamMatrix;
var RtspTestServer = require("../index.js").RtspTestServer;
var sleep = require("./internal.js").sleep;

let stream_matrix;
describe('WebStreamer', function () {
    describe.skip('#interface', function () {
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
    describe('#rtsp_test_server', function () {
        let rtsp_test_server;
        beforeEach(async () => {
            stream_matrix = new StreamMatrix();
            await stream_matrix.initialize();
            rtsp_test_server = new RtspTestServer(stream_matrix, "app0");
            await rtsp_test_server.initialize();
            await rtsp_test_server.startup();
        });

        afterEach(async () => {
            await rtsp_test_server.stop();
            await rtsp_test_server.destroy();
            await stream_matrix.terminate();
        });

        it(`version`, async () => {
            let version = await stream_matrix.version();
            console.log(version)
            assert.isString(version);
            await sleep(30000);
        });
    });
});
