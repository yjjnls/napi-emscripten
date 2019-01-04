'use strict'
const chai = require('chai');
let assert = chai.assert;

var StreamMatrix = require("../index.js").StreamMatrix;
var RtspTestServer = require("../index.js").RtspTestServer;
var RtspAnalyzer = require("../index.js").RtspAnalyzer;
var LiveStream = require("../index.js").LiveStream;
var utils = require("../index.js").utils;
var sleep = require("./internal.js").sleep;

let stream_matrix;
let rtsp_test_server;
let rtsp_analyzer;
let livestream;
let port = 8554;
describe('StreamMatrix', function () {
    describe('#interface', function () {
        beforeEach(async () => {
            stream_matrix = new StreamMatrix();
            await stream_matrix.initialize();
        });

        afterEach(async () => {
            await stream_matrix.terminate();
            console.log("=========================================")
        });

        it(`version`, async () => {
            let version = await stream_matrix.version();
            assert.isString(version);
        });
    });
    describe('#rtsp_test_server', function () {
        beforeEach(async () => {
            stream_matrix = new StreamMatrix();
            await stream_matrix.initialize();
            await stream_matrix.set_notification();

            rtsp_test_server = new RtspTestServer(stream_matrix, "app0", port, "/test", {
                video: "h264",
                audio: "pcma"
            });
            await rtsp_test_server.initialize();
            await rtsp_test_server.startup();
        });

        afterEach(async () => {
            await rtsp_test_server.stop();
            await rtsp_test_server.terminate();
            await stream_matrix.terminate();
            port++;
            console.log("=========================================")
        });
        it(`analyzer`, async () => {
            rtsp_analyzer = new RtspAnalyzer(stream_matrix,
                "app1",
                `rtsp://127.0.0.1:${port}/test`,
                {
                    video: "h264",
                    audio: "pcma"
                });
            await rtsp_analyzer.initialize();
            await rtsp_analyzer.startup();
            await utils.poll(() => {
                return rtsp_analyzer.analyze_done();
            }, 100, 10000);
            await rtsp_analyzer.stop();
            await rtsp_analyzer.terminate();
        });
        it(`analyzer2`, async () => {
            rtsp_analyzer = new RtspAnalyzer(stream_matrix,
                "app1",
                `rtsp://127.0.0.1:${port}/test`,
                {
                    video: "h264",
                    audio: "pcma"
                });
            await rtsp_analyzer.initialize();
            await rtsp_analyzer.startup();
            await utils.poll(() => {
                return rtsp_analyzer.analyze_done();
            }, 100, 10000);
            await rtsp_analyzer.stop();
            await rtsp_analyzer.terminate();
        });
    });
    describe('#livestream', function () {
        beforeEach(async () => {
            stream_matrix = new StreamMatrix();
            await stream_matrix.initialize();
            await stream_matrix.set_notification();
            // rtsp test server
            rtsp_test_server = new RtspTestServer(stream_matrix, "app0", port, "/test", {
                video: "h264",
                audio: "pcma"
            });
            await rtsp_test_server.initialize();
            await rtsp_test_server.startup();
        });
        afterEach(async () => {
            await rtsp_test_server.stop();
            await rtsp_test_server.terminate();

            await stream_matrix.terminate();
            port++;
            console.log("=========================================")
        });

        it(`create livestream`, async () => {
            livestream = new LiveStream(stream_matrix, "app1", `rtsp://127.0.0.1:${port}/test`, {
                video: "h264",
                audio: "pcma"
            });
            await livestream.initialize();
            await livestream.startup();
            await utils.poll(() => {
                return livestream.prepared();
            }, 100, 10000);
            await livestream.stop();
            await livestream.terminate();
            livestream = null;
        });
        describe('#audience', function () {
            beforeEach(async () => {
                livestream = new LiveStream(stream_matrix, "app1", `rtsp://127.0.0.1:${port}/test`, {
                    video: "h264",
                    audio: "pcma"
                });
                await livestream.initialize();
                await livestream.startup();
                await utils.poll(() => {
                    return livestream.prepared();
                }, 100, 10000);
            });
            afterEach(async () => {
                await livestream.stop();
                await livestream.terminate();
            });
            it(`add rtsp audience`, async () => {
                port++;
                await livestream.add_audience("endpoint0", { type: "rtsp", port: port, path: "/test_server" });
                // await sleep(30000);

                rtsp_analyzer = new RtspAnalyzer(stream_matrix,
                    "app2",
                    `rtsp://127.0.0.1:${port}/test_server`,
                    {
                        video: "h264",
                        audio: "pcma"
                    });
                await rtsp_analyzer.initialize();
                await rtsp_analyzer.startup();
                await utils.poll(() => {
                    return rtsp_analyzer.analyze_done();
                }, 100, 10000);
                await rtsp_analyzer.stop();
                await rtsp_analyzer.terminate();

                await livestream.remove_audience("endpoint0");
            });
            it.only(`add webrtc audience`, async () => {
                await livestream.add_audience("endpoint0", { type: "rtsp", port: 8553, path: "/test_server" });

                rtsp_analyzer = new RtspAnalyzer(stream_matrix,
                    "app2",
                    "rtsp://127.0.0.1:8553/test_server",
                    {
                        video: "h264",
                        audio: "pcma"
                    });
                await rtsp_analyzer.initialize();
                await rtsp_analyzer.startup();
                await utils.poll(() => {
                    return rtsp_analyzer.analyze_done();
                }, 100, 10000);
                await rtsp_analyzer.stop();
                await rtsp_analyzer.terminate();

                await livestream.remove_audience("endpoint0");
            });
        })
    });
});
