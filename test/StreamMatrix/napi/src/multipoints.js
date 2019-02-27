'use strict';

var IApp = require('./app.js').IApp;
var Promise = require('bluebird');
const default_option = require('./analyze_source.js').option;
var WebRTC = require('./webrtc.js').WebRTC;
var utils = require("./utils.js");

class MultiPoints extends IApp {
    constructor(stream_matrix, id, option) {
        super(stream_matrix, id, "MultiPoints");
        this.option_ = default_option;

        this.option_.video.codec = option.video;
        this.option_.audio.codec = option.audio;

        let self = this;
        this.on(`webrtc`, (meta, data) => {
            var meta_obj = JSON.parse(meta.toString('utf8'));
            let type = meta_obj.type;
            let id = meta_obj.id;

            Object.keys(self.members_).forEach(function (key) {
                let members = self.members_[key];
                for (var index = 0; index < members.length; ++index) {
                    if (members[index].id == id) {
                        if (type == 'sdp')
                            members[index].webrtc.send_sdp(data);
                        else
                            members[index].webrtc.send_ice(data);
                        break;
                    }
                }
            });
        });

        this.members_ = {};
    }
    async initialize() {
        let self = this;
        return new Promise((resolve, reject) => {
            self.stream_matrix().CreateMultiPoints(self.id_,
                self.option_.video.codec,
                self.option_.audio.codec,
                (code, data) => {
                    if (code == 0) { resolve(data); } else { reject(data); }
                });
        });
    }

    async add_member(id, room_id, option) {
        let self = this;
        if (option.type == "rtsp") {

        } else if (option.type == "webrtc") {
            let webrtc = new WebRTC(option.signal_bridge, option.connection_id, id, this, option.role);
            webrtc.connect();
            if (!self.members_[room_id])
                self.members_[room_id] = [];
            self.members_[room_id].push({ "type": option.type, "id": id, "webrtc": webrtc });

            return new Promise((resolve, reject) => {
                self.stream_matrix().AddMember(self.id_, id, option.role, (code, data) => {
                    if (code == 0) {
                        resolve(data);
                    } else {
                        webrtc.close();
                        self.members_[room_id].pop();
                        reject(data);
                    }
                });
            });
        } else if (option.type == "hls") {

        } else {
            return new Promise((resolve, reject) => {
                reject("multipoints add_member: unknown member type!");
            });
        }

    }
    async remove_member(id, room_id) {
        let self = this;
        let pos = 0;
        if (!this.members_[room_id]) {
            return;
        }
        for (; pos < this.members_[room_id].length; ++pos) {
            if (this.members_[room_id][pos].id == id) {
                break;
            }
        }

        if (pos == this.members_[room_id].length) {
            return;
            // return new Promise((resolve, reject) => {
            //     reject(`multipoints remove_member: the member ${id} hasn't been added!`);
            // });
        }

        if (this.members_[room_id][pos].type == 'webrtc') {
            this.members_[room_id][pos].webrtc.close();
        }
        this.members_[room_id].splice(pos, 1);

        return new Promise((resolve, reject) => {
            self.stream_matrix().RemoveMember(self.id_, id, (code, data) => {
                if (code == 0) { resolve(data); } else { reject(data); }
            });
        });
    }
    async set_speaker(id) {
        let self = this;
        let pos = 0;
        for (; pos < this.members_.length; ++pos) {
            if (this.members_[pos].id == id) {
                break;
            }
        }

        if (pos == this.members_.length) {
            return new Promise((resolve, reject) => {
                reject(`multipoints set_speaker: the member ${id} hasn't been added!`);
            });
        }

        return new Promise((resolve, reject) => {
            self.stream_matrix().SetSpeaker(self.id_, id, (code, data) => {
                if (code == 0) { resolve(data); } else { reject(data); }
            });
        });
    }
}

module.exports = {
    MultiPoints
};