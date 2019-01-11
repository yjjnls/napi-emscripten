'use strict';
var Promise = require('bluebird');
const bindingPath = require.resolve(`../plugin/build/Release/plugin.node`);
const Module = require(bindingPath);

class StreamMatrix {
    constructor() {
        Module.initialize();
        this.stream_matrix_ = new Module.StreamMatrix();
        this.apps_ = {};
    }
    async initialize() {
        let self = this;
        return new Promise((resolve, reject) => {
            self.stream_matrix_.Initialize((code, data) => {
                if (code == 0) { resolve(data); }
                else { reject(data); }
            });
        });
    }

    async terminate() {
        let self = this;
        return new Promise((resolve, reject) => {
            self.stream_matrix_.Terminate((code, data) => {
                self.stream_matrix_.delete();
                self.stream_matrix_ = null;
                global.gc();
                Module.release();

                if (code == 0) { resolve(data); }
                else { reject(data); }
            });
        });
    }

    async version() {
        let self = this;
        return new Promise((resolve, reject) => {
            self.stream_matrix_.version((code, data) => {
                if (code == 0) { resolve(JSON.parse(data).version); }
                else { reject(data); }
            });
        });
    }
    async set_notification() {
        let self = this;
        return new Promise((resolve, reject) => {
            self.stream_matrix_.SetNotification((meta, data) => {
                // console.log('~~~~~notification~~~~~');
                // console.log('meta: ' + meta);
                // console.log('data: ' + data);

                var m = JSON.parse(meta.toString('utf8'));
                var app = self.apps_[m.origin];
                if (app) {
                    app.emit(m.topic, meta, data);
                } else {
                    console.log("=================error")
                }

            }, (code, data) => {
                if (code == 0) { resolve(data); }
                else { reject(data); }
            });
        });
    }


}

module.exports = {
    StreamMatrix
};