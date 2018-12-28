'use strict';
const bindingPath = require.resolve(`./plugin/build/Release/plugin.node`);
// const bindingPath = require.resolve(`./plugin/opencv.js`);
const Module = require(bindingPath);
var Promise = require('bluebird');

async function sleep(timeout) {
    return new Promise((resolve, reject) => {
        setTimeout(function () {
            resolve();
        }, timeout);
    });
}
async function initialize(stream_matrix) {
    return new Promise((resolve, reject) => {
        stream_matrix.Initialize((code, data) => {
            if (code == 0) {
                console.log('Initialize OK!')
                console.log('code: ' + code);
                console.log('data: ' + data);
                resolve();
            }
            else {
                console.log('Initialize FAILED!')
                console.log('code: ' + code);
                console.log('data: ' + data);
                reject();
            }
        });
    });
}
async function set_notification(stream_matrix) {
    return new Promise((resolve, reject) => {
        stream_matrix.SetNotification((meta, data) => {
            console.log('~~~~~notification~~~~~');
            console.log('meta: ' + meta);
            console.log('data: ' + data);
        }, (code, data) => {
            if (code == 0) {
                console.log('set_notification OK!')
                console.log('code: ' + code);
                console.log('data: ' + data);
                resolve();
            }
            else {
                console.log('set_notification FAILED!')
                console.log('code: ' + code);
                console.log('data: ' + data);
                reject();
            }
        });
    });
}
async function version(stream_matrix) {
    return new Promise((resolve, reject) => {
        stream_matrix.version((code, data) => {
            if (code == 0) {
                console.log('version OK!')
                console.log('code: ' + code);
                console.log('data: ' + data);
                resolve();
            }
            else {
                console.log('version FAILED!')
                console.log('code: ' + code);
                console.log('data: ' + data);
                reject();
            }
        });
    });
}
async function terminate(stream_matrix) {
    return new Promise((resolve, reject) => {
        stream_matrix.Terminate((code, data) => {
            if (code == 0) {
                console.log('Terminate OK!')
                console.log('code: ' + code);
                console.log('data: ' + data);
                resolve();
            }
            else {
                console.log('Terminate FAILED!')
                console.log('code: ' + code);
                console.log('data: ' + data);
                reject();
            }
        });
    });
}
async function startup(stream_matrix, id) {
    return new Promise((resolve, reject) => {
        stream_matrix.StartUp(id, (code, data) => {
            if (code == 0) {
                console.log('StartUp OK!')
                console.log('code: ' + code);
                console.log('data: ' + data);
                resolve();
            }
            else {
                console.log('StartUp FAILED!')
                console.log('code: ' + code);
                console.log('data: ' + data);
                reject();
            }
        });
    });
}
async function stop(stream_matrix, id) {
    return new Promise((resolve, reject) => {
        stream_matrix.Stop(id, (code, data) => {
            if (code == 0) {
                console.log('Stop OK!')
                console.log('code: ' + code);
                console.log('data: ' + data);
                resolve();
            }
            else {
                console.log('Stop FAILED!')
                console.log('code: ' + code);
                console.log('data: ' + data);
                reject();
            }
        });
    });
}
async function destroy(stream_matrix, id) {
    return new Promise((resolve, reject) => {
        stream_matrix.Destroy(id, (code, data) => {
            if (code == 0) {
                console.log('DestroyApp OK!')
                console.log('code: ' + code);
                console.log('data: ' + data);
                resolve();
            }
            else {
                console.log('DestroyApp FAILED!')
                console.log('code: ' + code);
                console.log('data: ' + data);
                reject();
            }
        });
    });
}

/////////////////////////////////////////////////////////////////
async function launch(stream_matrix) {
    let id = "app1";
    return new Promise((resolve, reject) => {
        stream_matrix.CreateLauncher(id, "videotestsrc ! xvimagesink", (code, data) => {
            if (code == 0) {
                console.log('CreateLauncher OK!');
                console.log('code: ' + code);
                console.log('data: ' + data);
                resolve(id);
            }
            else {
                console.log('CreateLauncher FAILED!');
                console.log('code: ' + code);
                console.log('data: ' + data);
                reject();
            }
        });
    });
}
async function create_test_server(stream_matrix) {
    let id = "rtsp_test_server1";
    return new Promise((resolve, reject) => {
        stream_matrix.CreateRtspTestServer(id, 554, "/test", "videotestsrc ! video/x-raw,width=320,height=240,framerate=10/1 ! x264enc ! rtph264pay name=pay0 pt=96 ! audiotestsrc ! audio/x-raw,rate=8000 ! alawenc ! rtppcmapay name=pay1 pt=97", (code, data) => {
            if (code == 0) {
                console.log('CreateRtspTestServer OK!');
                console.log('code: ' + code);
                console.log('data: ' + data);
                resolve(id);
            }
            else {
                console.log('CreateRtspTestServer FAILED!');
                console.log('code: ' + code);
                console.log('data: ' + data);
                reject();
            }
        });
    });
}
(async function test() {
    let stream_matrix = new Module.StreamMatrix();

    await initialize(stream_matrix);
    console.log('------------------------------');
    await set_notification(stream_matrix);
    console.log('------------------------------');


    // let launch_id = await launch(stream_matrix);
    // console.log('------------------------------');
    // await startup(stream_matrix, launch_id);
    // console.log('------------------------------');
    // await sleep(5000);
    // await stop(stream_matrix, launch_id);
    // console.log('------------------------------');

    let rtsp_test_server = await create_test_server(stream_matrix);
    console.log('------------------------------');
    await startup(stream_matrix, rtsp_test_server);
    console.log('------------------------------');
    await sleep(10000);
    await stop(stream_matrix, rtsp_test_server);
    console.log('------------------------------');
    await destroy(stream_matrix, rtsp_test_server);

    await version(stream_matrix);
    console.log('------------------------------');
    await terminate(stream_matrix);
    console.log('------------------------------');


    stream_matrix.delete();
    stream_matrix = null;
    global.gc();

    Module.release();
})()

