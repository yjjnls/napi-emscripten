'use strict';

var IApp = require('./app.js').IApp;
var Promise = require('bluebird');
const default_option = require('./analyze_source.js').option;
var WebRTC = require('./webrtc.js').WebRTC;
var utils = require("./utils.js");

class LiveStream extends IApp {
    constructor(stream_matrix, id, url, option) {
        super(stream_matrix, id, "LiveStream");
        this.url_ = url;
        this.option_ = default_option;

        this.option_.video.codec = option.video;
        this.option_.audio.codec = option.audio;

        this.prepared_ = 0;
        let self = this;
        this.on("rtspclient@default_performer", (meta, data) => {
            // console.log(data);
            var obj = JSON.parse(data.toString('utf8'));
            if (obj.msg == "video channel connected" || obj.msg == "audio channel connected")
                self.prepared_++;
        });
        this.on("livestream@default_audience", (meta, data) => {
            // console.log(data);
            var obj = JSON.parse(data.toString('utf8'));
            if (obj.msg == "video data received" || obj.msg == "audio data received")
                self.prepared_++;
        });
        this.on(`webrtc`, (meta, data) => {
            var meta_obj = JSON.parse(meta.toString('utf8'));
            let type = meta_obj.type;
            let id = meta_obj.id;
            for (var pos = 0; pos < self.audiences_.length; ++pos) {
                if (self.audiences_[pos].id == id) {
                    if (type == 'sdp')
                        self.audiences_[pos].webrtc.send_sdp(data);
                    else
                        self.audiences_[pos].webrtc.send_ice(data);
                    break;
                }
            }
        });
        // this.on(`webrtc-peer-closed`, async (id) => {
        //     await self.remove_audience(id);
        // })

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
                    if (code == 0) { resolve(data); } else { reject(data); }
                });
        });
    }
    prepared() {
        return this.prepared_ >= 4;
    }
    async add_audience(id, option) {
        let self = this;
        if (option.type == "rtsp") {

            return new Promise((resolve, reject) => {
                self.stream_matrix().AddRtspAudience(self.id_, id, option.port, option.path,
                    (code, data) => {
                        if (code == 0) {
                            self.audiences_.push({ "type": option.type, "id": id });
                            resolve(data);
                        } else { reject(data); }
                    });
            });
        } else if (option.type == "webrtc") {
            let webrtc = new WebRTC(option.signal_bridge, option.connection_id, id, this, option.role);
            webrtc.connect();
            await utils.poll(() => {
                return webrtc.room_opened_;
            }, 100, 10000, 'webrtc open room timeout!');

            self.audiences_.push({ "type": option.type, "id": id, "webrtc": webrtc });

            return new Promise((resolve, reject) => {
                self.stream_matrix().AddWebrtcAudience(self.id_, id, option.role, (code, data) => {
                    if (code == 0) {
                        resolve(data);
                    } else {
                        webrtc.close();
                        self.audiences_.pop();
                        reject(data);
                    }
                });
            });
        } else if (option.type == "hls") {

        } else {
            return new Promise((resolve, reject) => {
                reject("livestream add_audience: unknown audience type!");
            });
        }

    }
    async remove_audience(id) {
        let self = this;
        let pos = 0;
        for (; pos < this.audiences_.length; ++pos) {
            if (this.audiences_[pos].id == id) {
                break;
            }
        }

        if (pos == this.audiences_.length) {
            return;
            // return new Promise((resolve, reject) => {
            //     reject(`livestream remove_audience: the audience ${id} hasn't been added!`);
            // });
        }

        if (this.audiences_[pos].type == 'webrtc') {
            this.audiences_[pos].webrtc.close();
        }
        this.audiences_.splice(pos, 1);

        return new Promise((resolve, reject) => {
            self.stream_matrix().RemoveAudience(self.id_, id, (code, data) => {
                if (code == 0) { resolve(data); } else { reject(data); }
            });
        });
    }
}

module.exports = {
    LiveStream
};