'use strict';

var IApp = require('./app.js').IApp;
var Promise = require('bluebird');
const default_option = require('./analyze_source.js').option;

class LiveStream extends IApp {
    constructor(stream_matrix, id, url, option) {
        super(stream_matrix, id, "LiveStream");
        this.url_ = url;
        this.option_ = default_option;

        this.option_.video.codec = option.video;
        this.option_.audio.codec = option.audio;

        this.prepared_ = 0;
        this.on("rtspclient@default_performer", (meta, data) => {
            // console.log(data);
            var obj = JSON.parse(data.toString('utf8'));
            if (obj.msg == "video channel connected" || obj.msg == "audio channel connected")
                this.prepared_++;
        });
        this.on("livestream@default_audience", (meta, data) => {
            // console.log(data);
            var obj = JSON.parse(data.toString('utf8'));
            if (obj.msg == "video data received" || obj.msg == "audio data received")
                this.prepared_++;
        });

        this.audiences_ = [];
    }
    async initialize() {
        let self = this;
        return new Promise((resolve, reject) => {
            self.stream_matrix().CreateLiveStream(self.id_,
                self.url_,
                self.option_.video.codec,
                self.option_.audio.codec,
                (code, data) => {
                    if (code == 0) { resolve(data); }
                    else { reject(data); }
                });
        });
    }
    prepared() {
        return this.prepared_ >= 4;
    }
    async add_audience(id, option) {
        let self = this;
        if (option.type = "rtsp") {

            return new Promise((resolve, reject) => {
                self.stream_matrix().AddRtspAudience(self.id_, id, option.port, option.path, (code, data) => {
                    if (code == 0) {
                        self.audiences_.push(id);
                        resolve(data);
                    }
                    else { reject(data); }
                });
            });
        } else if (option.type = "webrtc") {

        } else if (option.type = "hls") {

        } else {
            return new Promise((resolve, reject) => {
                reject("livestream add_audience: unknown audience type!");
            });
        }

    }
    async remove_audience(id) {
        let self = this;
        let pos = this.audiences_.indexOf(id);
        if (pos == -1) {
            return new Promise((resolve, reject) => {
                reject(`livestream remove_audience: the audience ${id} hasn't been added!`);
            });
        }
        this.audiences_.splice(pos, 1);
        return new Promise((resolve, reject) => {
            self.stream_matrix().RemoveAudience(self.id_, id, (code, data) => {
                if (code == 0) { resolve(data); }
                else { reject(data); }
            });
        });
    }
}

module.exports = {
    LiveStream
}