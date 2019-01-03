'use strict';

var IApp = require('./app.js').IApp;
const CodecMap = require('./codec_map.js').TABLE;
const option = require('./analyze_source.js').option;
var Promise = require('bluebird');

function video_description() {
    var v = option.video;
    if (!v) {
        return;
    }

    var line = `videotestsrc pattern=${v.pattern}`;

    var o = v.overlay;
    if (o) {
        line = `${line} ! ${o.type}overlay valignment=3 halignment=4 time-mode=2`;
        line = `${line} xpos=${o.xpos} ypos=${o.xpos} color=${o.color} font-desc="${o.font}"`;
        if (o.text) {
            line = `${line}="${o.text}"`;
        }

        line = `${line} draw-shadow=${o['draw-shadow']} draw-outline=${o['draw-outline']} outline-color=${o['outline-color']}`;

    }

    line = `${line}  ! video/x-raw, format=(string)I420, width=(int)${v.width}, height=(int)${v.height}`;
    var codec = CodecMap[v.codec];
    line = `${line}  ! ${codec.enc} bitrate=${v.bitrate} key-int-max=${v.kfi} ! ${codec.pay} pt=96`;
    // line = `${line}  ! jpegenc ! rtph264pay pt=96`;
    return line;
}
function audio_description() {

    var a = option.audio;
    if (!a) {
        return;
    }
    var line = `audiotestsrc freq=${a.freq} wave=sine ! audio/x-raw, rate=(int)${a.rate}, channels=(int)${a.channels}`;

    var codec = CodecMap[a.codec];
    line = `${line} ! ${codec.enc} ! ${codec.pay} pt=97`;
    return line;
}
function source_bin_description() {
    var video = video_description();
    var audio = audio_description();
    var desc = '( ';
    if (video) {
        desc += ` ${video} name=pay0 `;
    }

    if (audio) {
        if (video) {
            desc += ` ${audio} name=pay1`;
        } else {
            desc += ` ${audio} name=pay0`;
        }
    }
    return desc + ' )';

}

class RtspTestServer extends IApp {
    constructor(stream_matrix, id, port = 8554, path = "/test") {
        super(stream_matrix, id, "RtspTestServer");
        this.port_ = port;
        this.path_ = path;
    }
    async initialize() {
        let self = this;
        let launch = source_bin_description();
        return new Promise((resolve, reject) => {
            self.instance_.CreateRtspTestServer(self.id_, self.port_, self.path_, launch, (code, data) => {
                if (code == 0) { resolve(data); }
                else { reject(data); }
            });
        });
    }
}

module.exports = {
    RtspTestServer
}