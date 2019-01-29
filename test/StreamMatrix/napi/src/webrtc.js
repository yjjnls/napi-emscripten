'use strict';
const socket = require('socket.io-client');
const uuid = require('uuid');
var Promise = require('bluebird');

class WebRTC {
    constructor(signal_bridge, connection_id, endpoint_name, owner, role = 'answer', id = uuid.v4()) {

        this.signal_bridge_ = signal_bridge;
        this.connection_id_ = connection_id;
        this.endpoint_name_ = endpoint_name;
        this.owner_ = owner;
        this.role_ = role;
        this.id_ = id;
        this.connection = null;
        this.closed = false;
        this.io = null;
        this.socketMessageEvent = 'video-conference-demo';
        this.remote_user_id_ = null;
        this.remote_user_set = false;
        this.send_data = [];
        this.negoated = false;
        this.room_opened_ = (role == 'answer') ? true : false;

        this.join_msg = {
            "sessionid": String(this.connection_id_),
            "session": {
                "audio": true,
                "video": true
            },
            "mediaConstraints": {
                "audio": {
                    "mandatory": {},
                    "optional": []
                },
                "video": {
                    "mandatory": {},
                    "optional": [
                        {
                            "facingMode": "user"
                        }
                    ]
                }
            },
            "sdpConstraints": {
                "mandatory": {
                    "OfferToReceiveAudio": true,
                    "OfferToReceiveVideo": true
                },
                "optional": [
                    {
                        "VoiceActivityDetection": false
                    }
                ]
            },
            "streams": [
                {
                    "streamid": "XnIKvGX2d1TVi3I3K5dQiluoHWxkVxI0ECva",
                    "tracks": 2
                }
            ],
            "extra": {},
            "password": ""
        };
        this.join_msg1 = {
            "remoteUserId": String(this.connection_id_),
            "message": {
                "newParticipationRequest": true,
                "isOneWay": false,
                "isDataOnly": false,
                "localPeerSdpConstraints": {
                    "OfferToReceiveAudio": true,
                    "OfferToReceiveVideo": true
                },
                "remotePeerSdpConstraints": {
                    "OfferToReceiveAudio": true,
                    "OfferToReceiveVideo": true
                }
            },
            "sender": this.id_
        };
        this.join_msg2 = {
            "remoteUserId": "",
            "message": {
                "readyForOffer": true,
                "userPreferences": {
                    "extra": {},
                    "localPeerSdpConstraints": {
                        "OfferToReceiveAudio": true,
                        "OfferToReceiveVideo": true
                    },
                    "remotePeerSdpConstraints": {
                        "OfferToReceiveAudio": true,
                        "OfferToReceiveVideo": true
                    },
                    "isOneWay": true,
                    "isDataOnly": false,
                    "dontGetRemoteStream": false,
                    "dontAttachLocalStream": false,
                    "connectionDescription": {
                        "remoteUserId": "",
                        "message": {
                            "newParticipationRequest": true,
                            "isOneWay": false,
                            "isDataOnly": false,
                            "localPeerSdpConstraints": {
                                "OfferToReceiveAudio": true,
                                "OfferToReceiveVideo": true
                            },
                            "remotePeerSdpConstraints": {
                                "OfferToReceiveAudio": true,
                                "OfferToReceiveVideo": true
                            }
                        },
                        "sender": this.id_
                    },
                    "streamsToShare": {
                        "XnIKvGX2d1TVi3I3K5dQiluoHWxkVxI0ECva": {
                            "isAudio": true,
                            "isVideo": true,
                            "isScreen": false
                        }
                    }
                }
            },
            "sender": this.id_
        };
        this.open_msg2 = {
            "remoteUserId": "",
            "message": {
                "enableMedia": true,
                "userPreferences": {
                    "extra": {},
                    "localPeerSdpConstraints": {
                        "OfferToReceiveAudio": true,
                        "OfferToReceiveVideo": true
                    },
                    "remotePeerSdpConstraints": {
                        "OfferToReceiveAudio": true,
                        "OfferToReceiveVideo": true
                    },
                    "isOneWay": true,
                    "isDataOnly": false,
                    "dontGetRemoteStream": false,
                    "dontAttachLocalStream": false,
                    "connectionDescription": {
                        "remoteUserId": this.id_,
                        "message": {
                            "newParticipationRequest": true,
                            "isOneWay": false,
                            "isDataOnly": false,
                            "localPeerSdpConstraints": {
                                "OfferToReceiveAudio": true,
                                "OfferToReceiveVideo": true
                            },
                            "remotePeerSdpConstraints": {
                                "OfferToReceiveAudio": true,
                                "OfferToReceiveVideo": true
                            }
                        },
                        "sender": ""
                    }
                }
            },
            "sender": this.id_
        };

        this.sdp_msgs_ = null;
        this.ice_msgs_ = [];
    }
    send_sdp(data) {
        let json_data = {
            "remoteUserId": this.remote_user_id_,
            "message": {
                "type": this.role_,
                "sdp": JSON.parse(data.toString('utf8')).sdp,
                "remotePeerSdpConstraints": {
                    "OfferToReceiveAudio": true,
                    "OfferToReceiveVideo": true
                },
                "renegotiatingPeer": false,
                "connectionDescription": {
                    "remoteUserId": this.id_,
                    "message": {
                        "newParticipationRequest": true,
                        "isOneWay": false,
                        "isDataOnly": false,
                        "localPeerSdpConstraints": {
                            "OfferToReceiveAudio": true,
                            "OfferToReceiveVideo": true
                        },
                        "remotePeerSdpConstraints": {
                            "OfferToReceiveAudio": true,
                            "OfferToReceiveVideo": true
                        }
                    },
                    "sender": this.remote_user_id_,
                    "password": false
                },
                "dontGetRemoteStream": false,
                "extra": {},
                "streamsToShare": {
                    "XnIKvGX2d1TVi3I3K5dQiluoHWxkVxI0ECva": {
                        "isAudio": true,
                        "isVideo": true,
                        "isScreen": false
                    }
                }
            },
            "sender": this.id_
        };
        if (this.negoated)
            this.io.emit(this.socketMessageEvent, json_data);
        else
            this.sdp_msgs_ = json_data;
    }
    send_ice(data) {
        // console.log(data)
        let json_data = {
            "remoteUserId": this.remote_user_id_,
            "message": JSON.parse(data.toString('utf8')),
            "sender": this.id_
        };
        if (this.negoated)
            this.io.emit(this.socketMessageEvent, json_data);
        else
            this.ice_msgs_.push(json_data);
    }

    handle_sdp(data) {
        let self = this;
        return new Promise((resolve, reject) => {
            self.owner_.stream_matrix().SetRemoteDescription(self.owner_.id_, self.endpoint_name_, data.type, data.sdp, (code, data) => {
                if (code == 0) { resolve(); } else { reject(); }
            });
        });
    }

    handle_ice(data) {
        let self = this;
        return new Promise((resolve, reject) => {
            self.owner_.stream_matrix().SetRemoteCandidate(self.owner_.id_, self.endpoint_name_, data.candidate, data.sdpMLineIndex, (code, data) => {
                if (code == 0) { resolve(); } else { reject(); }
            });
        });

    }
    connect() {
        this.io = socket(`${this.signal_bridge_}?userid=${this.id_}&sessionid=${this.connection_id_}&msgEvent=${this.socketMessageEvent}&socketCustomEvent=RTCMultiConnection-Custom-Message&autoCloseEntireSession=false&maxParticipantsAllowed=1000`);

        let io = this.io;
        let self = this;
        io.on('connect', (socket) => {
            if (self.role_ == "answer") {
                io.emit('join-room', self.join_msg);
                io.emit(self.socketMessageEvent, self.join_msg1);
            } else {
                io.emit('extra-data-updated', {});
                io.emit('open-room', self.join_msg);
                self.room_opened_ = true;
            }

            io.on(self.socketMessageEvent, (data) => {
                let msg = data.message;
                if (!self.remote_user_id_ && data.hasOwnProperty('sender')) {
                    self.remote_user_set = true;
                    self.remote_user_id_ = data.sender;

                    if (self.role_ == "answer") {
                        self.join_msg2.remoteUserId = data.sender;
                        self.join_msg2.message.userPreferences.connectionDescription.remoteUserId = data.sender;
                        io.emit(self.socketMessageEvent, self.join_msg2);
                        self.negoated = true;
                    } else {
                        self.open_msg2.remoteUserId = data.sender;
                        self.open_msg2.message.userPreferences.connectionDescription.sender = data.sender;
                        io.emit(self.socketMessageEvent, self.open_msg2);
                    }
                }
                // offer send sdp&ice
                if (!self.negoated && self.role_ == "offer" && data.message.hasOwnProperty('readyForOffer')) {
                    self.negoated = true;

                    if (self.sdp_msgs_) {
                        self.sdp_msgs_.remoteUserId = self.remote_user_id_;
                        self.sdp_msgs_.message.connectionDescription.sender = self.remote_user_id_;
                        self.io.emit(self.socketMessageEvent, self.sdp_msgs_);
                    }

                    self.ice_msgs_.forEach((msg) => {
                        msg.remoteUserId = self.remote_user_id_;
                        self.io.emit(self.socketMessageEvent, msg);
                    });
                    self.ice_msgs_ = [];
                }
                // answer send sdp&ice
                if (self.negoated && self.role_ == "answer" && data.message.hasOwnProperty('sdp')) {
                    if (self.sdp_msgs_) {
                        self.sdp_msgs_.remoteUserId = self.remote_user_id_;
                        self.sdp_msgs_.message.connectionDescription.sender = self.remote_user_id_;
                        self.io.emit(self.socketMessageEvent, self.sdp_msgs_);
                    }

                    self.ice_msgs_.forEach((msg) => {
                        msg.remoteUserId = self.remote_user_id_;
                        self.io.emit(self.socketMessageEvent, msg);
                    });
                    self.ice_msgs_ = [];
                }
                if (self.negoated && msg.hasOwnProperty('sdp')) {
                    self.handle_sdp(msg);
                }
                if (self.negoated && msg.hasOwnProperty('candidate')) {
                    self.handle_ice(msg);
                }
                // another peer closed
                if (data.message.hasOwnProperty('userLeft')) {
                    if (data.message.userLeft) {
                        self.close();
                    }
                }
            });
        });
        io.on('disconnect', (reason) => {
            console.log('webrtc socket.io disconnect: ' + reason);
            self.close();
        });
        io.on('leave', (reason) => {
            console.log(reason);
            self.close();
        });
        io.on('reconnect_failed', () => {
            self.close();
            throw Error('reconnect_failed');
        });
        io.on('connect_timeout', () => {
            io.open();
        });
        io.on('connect_error', (error) => {
            self.close();
            throw error;
        });
    }

    close() {
        if (this.io) {
            let data = {
                "remoteUserId": this.remote_user_id_,
                "message": {
                    "userLeft": true
                },
                "sender": this.id_
            };
            this.io.emit(this.socketMessageEvent, data);
            this.io.close();
            this.io = null;
        }
        if (!this.closed) {
            this.closed = true;
            // this.owner_.emit('webrtc-peer-closed', this.endpoint_name_);
        }

    }
}

module.exports = {
    WebRTC
};