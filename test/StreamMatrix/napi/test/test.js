'use strict';
const chai = require('chai');
let assert = chai.assert;

var StreamMatrix = require("../index.js").StreamMatrix;
var RtspTestServer = require("../index.js").RtspTestServer;
var RtspAnalyzer = require("../index.js").RtspAnalyzer;
var WebrtcAnalyzer = require("../index.js").WebrtcAnalyzer;
var LiveStream = require("../index.js").LiveStream;
var MultiPoints = require("../index.js").MultiPoints;
var utils = require("../index.js").utils;
var sleep = require("./internal.js").sleep;

let stream_matrix;
let rtsp_test_server;
let rtsp_analyzer;
let webrtc_analyzer;
let livestream;
let multipoints;
let port = 8554;
describe('StreamMatrix', function () {
    beforeEach(async () => {
        stream_matrix = new StreamMatrix();
        await stream_matrix.initialize();
        await stream_matrix.set_notification();
    });

    afterEach(async () => {
        await stream_matrix.terminate();
        console.log("=========================================");
    });

    describe('#interface', function () {
        it(`version`, async () => {
            let version = await stream_matrix.version();
            assert.isString(version);
        });
    });
    describe('#rtsp_test_server', function () {
        it(`test1`, async () => {
            rtsp_test_server = new RtspTestServer(stream_matrix, "app0", port, "/test", {
                video: "h264",
                audio: "pcma"
            });
            await rtsp_test_server.initialize();
            await rtsp_test_server.startup();
            await rtsp_test_server.stop();
            await rtsp_test_server.terminate();
        });
        it(`test2`, async () => {
            rtsp_test_server = new RtspTestServer(stream_matrix, "app0", port, "/test", {
                video: "h264",
                audio: "pcma"
            });
            await rtsp_test_server.initialize();
            await rtsp_test_server.startup();
            await rtsp_test_server.terminate();
        });
        it(`test3`, async () => {
            rtsp_test_server = new RtspTestServer(stream_matrix, "app0", port, "/test", {
                video: "h264",
                audio: "pcma"
            });
            await rtsp_test_server.initialize();
            await rtsp_test_server.startup();
            await rtsp_test_server.startup();
            await rtsp_test_server.terminate();
        });
        it(`test4`, async () => {
            rtsp_test_server = new RtspTestServer(stream_matrix, "app0", port, "/test", {
                video: "h264",
                audio: "pcma"
            });
            await rtsp_test_server.initialize();
            await rtsp_test_server.startup();
            await rtsp_test_server.stop();
            await rtsp_test_server.stop();
            await rtsp_test_server.terminate();
        });
        it(`test5`, async () => {
            rtsp_test_server = new RtspTestServer(stream_matrix, "app0", port, "/test", {
                video: "h264",
                audio: "pcma"
            });
            await rtsp_test_server.initialize();
            await rtsp_test_server.startup();
            await rtsp_test_server.stop();
            await rtsp_test_server.terminate();

            rtsp_test_server = new RtspTestServer(stream_matrix, "app0", port, "/test", {
                video: "h264",
                audio: "pcma"
            });
            await rtsp_test_server.initialize();
            await rtsp_test_server.startup();
            await rtsp_test_server.stop();
            await rtsp_test_server.terminate();
        });
    });
    describe('#rtsp_analyzer', function () {
        beforeEach(async () => {
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
        });
        // it.only(`test1`, async () => {
        //     await sleep(1000);
        // });
        // it.only(`test2`, async () => {
        //     await sleep(1000);
        // });

        it(`analyzer`, async () => {
            rtsp_analyzer = new RtspAnalyzer(stream_matrix,
                "app1",
                {
                    video: "h264",
                    audio: "pcma",
                    url: `rtsp://127.0.0.1:${port}/test`
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
                {
                    video: "h264",
                    audio: "pcma",
                    url: `rtsp://127.0.0.1:${port}/test`
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
                // port++;
                await livestream.add_audience("endpoint0", { type: "rtsp", port: port, path: "/test_server" });

                rtsp_analyzer = new RtspAnalyzer(stream_matrix,
                    "app2",
                    {
                        video: "h264",
                        audio: "pcma",
                        url: `rtsp://127.0.0.1:${port}/test_server`
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
            it(`add webrtc audience`, async () => {
                await livestream.add_audience("endpoint1", { type: "webrtc", signal_bridge: "http://172.16.64.58:9001/", role: "offer", connection_id: "1" });
                await sleep(300000);
                webrtc_analyzer = new WebrtcAnalyzer(stream_matrix, "app2", { video: "h264", audio: "pcma", signal_bridge: "http://172.16.64.58:9001/", role: "answer", connection_id: "1" });
                await webrtc_analyzer.initialize();

                await webrtc_analyzer.startup();
                await utils.poll(() => {
                    return webrtc_analyzer.analyze_done();
                }, 100, 10000);
                await webrtc_analyzer.stop();

                await webrtc_analyzer.startup();
                await utils.poll(() => {
                    return webrtc_analyzer.analyze_done();
                }, 100, 10000);
                // await sleep(200000);
                await webrtc_analyzer.stop();

                await webrtc_analyzer.terminate();

                await livestream.remove_audience("endpoint1");
            });
        });
    });

    describe('#multipoints', function () {
        beforeEach(async () => {
            multipoints = new MultiPoints(stream_matrix, "app1", {
                video: "h264",
                audio: "pcma"
            });
            await multipoints.initialize();
            await multipoints.startup();
        });
        afterEach(async () => {
            await multipoints.stop();
            await multipoints.terminate();
            multipoints = null;
        });
        it(`members`, async () => {
            await multipoints.add_member("endpoint1", "test_room", { type: "webrtc", signal_bridge: "http://172.16.64.58:9001/", role: "offer", connection_id: "1" });


            await multipoints.add_member("endpoint2", "test_room", { type: "webrtc", signal_bridge: "http://172.16.64.58:9001/", role: "offer", connection_id: "2" });

            // webrtc_analyzer = new WebrtcAnalyzer(stream_matrix, "app2", { video: "h264", audio: "pcma", signal_bridge: "http://172.16.64.58:9001/", role: "answer", connection_id: "1", room_id: "test_room" });
            // await webrtc_analyzer.initialize();
            // await webrtc_analyzer.startup();

            await sleep(30000);
            // await webrtc_analyzer.stop();
            // await webrtc_analyzer.terminate();

            await multipoints.remove_member("endpoint1", "test_room");
            await multipoints.remove_member("endpoint2", "test_room");

        });
    });

});
