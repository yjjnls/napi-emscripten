const assert = require('assert');
const EventEmitter = require('events').EventEmitter;
var Promise = require('bluebird');

class IApp extends EventEmitter {
    constructor(stream_matrix, id, type) {
        super();
        this.owner_ = stream_matrix;
        // this.owner_.stream_matrix_ = stream_matrix.stream_matrix_;
        this.id_ = id;
        this.uname_ = `${type}@${id}`;

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
            self.owner_.stream_matrix_.StartUp(self.id_, (code, data) => {
                if (code == 0) { resolve(data); }
                else { reject(data); }
            });
        });
    }

    async stop() {
        let self = this;
        return new Promise((resolve, reject) => {
            self.owner_.stream_matrix_.Stop(self.id_, (code, data) => {
                if (code == 0) { resolve(data); }
                else { reject(data); }
            });
        });
    }

    async terminate() {
        delete this.owner_.apps_[this.uname_];
        let self = this;
        return new Promise((resolve, reject) => {
            self.owner_.stream_matrix_.Destroy(self.id_, (code, data) => {
                if (code == 0) { resolve(data); }
                else { reject(data); }
            });
        });
    }
    stream_matrix() {
        return this.owner_.stream_matrix_;
    }
}

module.exports = {
    IApp
}