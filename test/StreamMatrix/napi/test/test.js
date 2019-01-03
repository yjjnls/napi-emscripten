'use strict'
const chai = require('chai');
let assert = chai.assert;

var StreamMatrix = require("../index.js").StreamMatrix;
var RtspTestServer = require("../index.js").RtspTestServer;
var RtspAnalyzer = require("../index.js").RtspAnalyzer;
var utils = require("../index.js").utils;
var sleep = require("./internal.js").sleep;

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
            assert.isString(version);
        });
    });
    describe('#rtsp_test_server', function () {
        let rtsp_test_server;
        beforeEach(async () => {
            stream_matrix = new StreamMatrix();
            await stream_matrix.initialize();
            await stream_matrix.set_notification();

            rtsp_test_server = new RtspTestServer(stream_matrix, "app0", 8554, "/test", {
                video: "h264",
                audio: "g711a"
            });
            await rtsp_test_server.initialize();
            await rtsp_test_server.startup();
        });

        afterEach(async () => {
            await rtsp_test_server.stop();
            await rtsp_test_server.terminate();
            await stream_matrix.terminate();
        });
        let rtsp_analyzer;
        it(`analyzer`, async () => {
            rtsp_analyzer = new RtspAnalyzer(stream_matrix,
                "app1",
                "rtsp://127.0.0.1:8554/test",
                {
                    video: "h264",
                    audio: "g711a"
                });
            await rtsp_analyzer.initialize();
            await rtsp_analyzer.startup();
            // await sleep(30000);
            await utils.poll(() => {
                return rtsp_analyzer.analyze_done();
            }, 100, 10000);
            await rtsp_analyzer.stop();
            await rtsp_analyzer.terminate();
        });
    });
});
