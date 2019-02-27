'use strict';

var IApp = require('./app.js').IApp;
const CodecMap = require('./codec_map.js').TABLE;
const default_option = require('./analyze_source.js').option;
var Promise = require('bluebird');
var utils = require("./utils.js");
var WebRTC = require('./webrtc.js').WebRTC;

const bindingPath = require.resolve(`/root/Desktop/del/napi-emscripten/test/tesseract/napi/plugin/build/Release/plugin.node`);
const tesseract = require(bindingPath);

class Analyzer extends IApp {
    constructor(stream_matrix, id, type, option) {
        super(stream_matrix, id, type);
        this.option_ = default_option;

        this.option_.video.codec = option.video;
        this.option_.audio.codec = option.audio;


        this.audio_passed_ = 0;
        this.on('spectrum', function (meta, data) {
            var obj = JSON.parse(data.toString('utf8'));
            let magnitude = obj.magnitude;
            // console.log(meta)
            this.analyze_spectrum(magnitude);
        });
        this.on('image_data', function (meta, data) {
            this.analyze_image(meta, data);
            // console.log(meta)
            // console.log(data)
        });

        this.image_passed_ = 0;
        this.pre_image_data_ = 0;
        this.api_ = new tesseract.TessBaseAPI();
        this.api_.Init("/opt", "eng");
        this.api_.SetVariable("tessedit_char_whitelist", "0123456789:.");
    }
    async startup() {
        this.audio_passed_ = 0;
        this.image_passed_ = 0;
        super.startup();
    }
    async terminate() {
        super.terminate();
        this.api_.End();
        this.api_.delete();
        this.api_ = null;

        global.gc();
    }
    analyze_spectrum(magnitude) {
        let freq = this.option_.audio.freq;
        let bands = this.option_.audio.bands;
        let rate = this.option_.audio.rate;

        let bandwith = (rate / bands) / 2;
        let pos = freq / bandwith;
        pos = Math.max(pos, 0);
        pos = Math.round(pos);
        let max = Math.max.apply(null, magnitude);
        let mag = magnitude[pos];

        if (Math.abs(mag - max) < 5) {
            this.audio_passed_++;
        }
    }

    analyze_image(meta, data) {
        var obj = JSON.parse(data.toString('utf8'));
        var pointer = obj.data;
        var size = obj.size;
        let image = tesseract.pixReadMemBmp(pointer, size);
        this.api_.SetImage(image);
        let result = this.api_.GetUTF8Text();
        tesseract.pixDestroy(image);
        result = result.slice(0, 11);

        let ocr = utils.parseTime(result);
        // console.log(ocr)
        let ms = obj["stream-time"] / 1000000;
        console.log(`ocr: ${ocr} ms:${ms}`);
        let diff = ms - ocr;
        console.log(this.pre_image_data_ - diff);
        if (this.pre_image_data_ != 0) {
            if (Math.abs(this.pre_image_data_ - diff) < 5)
                this.image_passed_++;
        }
        this.pre_image_data_ = diff;
    }
    analyze_done() {
        // console.log(this.audio_passed_)
        // console.log(this.image_passed_)
        return this.audio_passed_ >= 5 && this.image_passed_ >= 5;
    }
}
class RtspAnalyzer extends Analyzer {
    constructor(stream_matrix, id, option) {
        super(stream_matrix, id, "RtspAnalyzer", option);
        this.url_ = option.url;

        let video = this.option_.video;
        let audio = this.option_.audio;

        this.launch_ =
            `( rtspsrc location=${this.url_} name=r r. `
            + `! queue ! capsfilter caps="application/x-rtp,media=video" `
            + `! ${CodecMap[video.codec].depay} ! ${CodecMap[video.codec].dec} ! fakesink name=sink r. `
            + `! queue ! capsfilter caps="application/x-rtp,media=audio" ` // audio
            + `! ${CodecMap[audio.codec].depay} ! ${CodecMap[audio.codec].dec} ! audioconvert `
            + `! spectrum name=audio bands=${audio.bands} threshold=${audio.threshold} post-messages=TRUE message-phase=TRUE message-magnitude=TRUE `
            + `! fakesink )`;
    }
    async initialize() {
        let self = this;
        return new Promise((resolve, reject) => {
            self.stream_matrix().CreateRtspAnalyzer(self.id_, self.launch_, (code, data) => {
                if (code == 0) { resolve(data); } else { reject(data); }
            });
        });
    }
}
class WebrtcAnalyzer extends Analyzer {
    constructor(stream_matrix, id, option) {
        super(stream_matrix, id, "WebrtcAnalyzer", option);

        this.signal_bridge_ = option.signal_bridge;
        this.role_ = option.role;
        this.connection_id_ = option.connection_id;
        this.webrtc_ = null;

        let video = this.option_.video;
        let audio = this.option_.audio;

        this.launch_ =
            `webrtcbin name=webrtc `
            // + ` webrtc. ! ${CodecMap[video.codec].depay} name=video_payloader ! ${CodecMap[video.codec].dec} name=video_decoder ! fakesink sync=false name=sink`
            // // + ` webrtc. ! ${CodecMap[video.codec].depay} name=video_payloader ! ${CodecMap[video.codec].dec} name=video_decoder ! videoconvert ! ximagesink sync=false name=sink`
            // + ` webrtc. ! ${CodecMap[audio.codec].depay} ! ${CodecMap[audio.codec].dec} ! audioconvert ! audioresample `
            // + `! spectrum name=audio bands=${audio.bands} threshold=${audio.threshold} post-messages=TRUE message-phase=TRUE message-magnitude=TRUE `
            // + `! fakesink sync=false `
            + `rtph264pay config-interval=-1 name=pay0 ! queue ! application/x-rtp,media=video,encoding-name=H264,payload=96 ! webrtc.  rtppcmapay name=pay1 ! queue ! application/x-rtp,media=audio,encoding-name=PCMA,payload=8 ! webrtc.`;
        // + ` )`;

        let self = this;
        this.on(`webrtc`, (meta, data) => {
            var meta_obj = JSON.parse(meta.toString('utf8'));
            let type = meta_obj.type;

            if (type == 'sdp')
                self.webrtc_.send_sdp(data);
            else
                self.webrtc_.send_ice(data);
        });
    }
    async initialize() {
        let self = this;
        self.webrtc_ = new WebRTC(self.signal_bridge_, self.connection_id_, "analyzer", this, self.role_);
        self.webrtc_.connect();
        // await utils.poll(() => {
        //     return self.webrtc_.negoated;
        // }, 100, 10000, "another webrtc peer negoates overtime");
        return new Promise((resolve, reject) => {
            self.stream_matrix().CreateWebrtcAnalyzer(self.id_,
                self.role_,
                self.launch_,
                (code, data) => {
                    if (code == 0) { resolve(data); } else { reject(data); }
                });
        });
    }
}
module.exports = {
    RtspAnalyzer,
    WebrtcAnalyzer
};