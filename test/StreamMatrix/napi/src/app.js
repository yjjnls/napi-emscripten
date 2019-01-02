const assert = require('assert');
const EventEmitter = require('events').EventEmitter;
var Promise = require('bluebird');

class IApp extends EventEmitter {
    constructor(stream_matrix, id, type) {
        this.stream_matrix_ = stream_matrix;
        this.id = id;
        var uname = `${id}@${type}`;

        if (stream_matrix.apps_[uname]) {
            throw {
                name: 'AppCreate',
                message: `create repeated application :${uname}`
            };
        }
        stream_matrix.apps_[uname] = this;
        return uname;
    }

    async startup() {
        let self = this;
        return new Promise((resolve, reject) => {
            self.stream_matrix_.StartUp(self.id, (code, data) => {
                if (code == 0) { resolve(data); }
                else { reject(data); }
            });
        });
    }

    async stop() {
        let self = this;
        return new Promise((resolve, reject) => {
            self.stream_matrix_.Stop(self.id, (code, data) => {
                if (code == 0) { resolve(data); }
                else { reject(data); }
            });
        });
    }

    async destroy() {
        let self = this;
        return new Promise((resolve, reject) => {
            self.stream_matrix_.Destroy(self.id, (code, data) => {
                if (code == 0) { resolve(data); }
                else { reject(data); }
            });
        });
    }
}

module.exports = {
    IApp
}