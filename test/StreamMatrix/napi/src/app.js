const assert = require('assert');
const EventEmitter = require('events').EventEmitter;
var Promise = require('bluebird');

class IApp extends EventEmitter {
    constructor(stream_matrix, id, type) {
        super();
        this.stream_matrix_ = stream_matrix;
        this.instance_ = stream_matrix.stream_matrix_;
        this.id_ = id;
        this.uname_ = `${id}@${type}`;

        if (stream_matrix.apps_[this.uname_]) {
            throw {
                name: 'AppCreate',
                message: `create repeated application :${this.uname_}`
            };
        }
        stream_matrix.apps_[this.uname_] = this;
    }

    async startup() {
        let self = this;
        return new Promise((resolve, reject) => {
            self.instance_.StartUp(self.id_, (code, data) => {
                if (code == 0) { resolve(data); }
                else { reject(data); }
            });
        });
    }

    async stop() {
        let self = this;
        return new Promise((resolve, reject) => {
            self.instance_.Stop(self.id_, (code, data) => {
                if (code == 0) { resolve(data); }
                else { reject(data); }
            });
        });
    }

    async destroy() {
        delete this.stream_matrix_.apps_[this.uname_];
        let self = this;
        return new Promise((resolve, reject) => {
            self.instance_.Destroy(self.id_, (code, data) => {
                if (code == 0) { resolve(data); }
                else { reject(data); }
            });
        });
    }
}

module.exports = {
    IApp
}