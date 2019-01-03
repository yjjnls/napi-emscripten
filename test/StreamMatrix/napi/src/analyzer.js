'use strict'

var IApp = require('./app.js').IApp;
const CodecMap = require('./codec_map.js').TABLE;
const default_option = require('./analyze_source.js').option;
var Promise = require('bluebird');
var utils = require("./utils.js");

const bindingPath = require.resolve(`/root/Desktop/del/napi-emscripten/test/tesseract/napi/plugin/build/Release/plugin.node`);
const tesseract = require(bindingPath);

class RtspAnalyzer extends IApp {
    constructor(stream_matrix, id, url, option) {
        super(stream_matrix, id, "RtspAnalyzer");
        this.url_ = url;
        this.option = default_option;

        let video = this.option.video;
        let audio = this.option.audio;

        this.option.video.codec = option.video;
        this.option.audio.codec = option.audio;

        let video_codec = CodecMap[video.codec];
        let audio_codec = CodecMap[audio.codec];

        this.launch_ =
            `( rtspsrc location=${this.url_} name=r r. `
            + `! queue ! capsfilter caps="application/x-rtp,media=video" `
            + `! ${video_codec.depay} ! ${video_codec.dec} ! fakesink name=sink r. `
            + `! queue ! capsfilter caps="application/x-rtp,media=audio" ` // audio
            + `! ${audio_codec.depay} ! ${audio_codec.dec} ! audioconvert `
            + `! spectrum name=audio bands=${audio.bands} threshold=${audio.threshold} post-messages=TRUE message-phase=TRUE message-magnitude=TRUE `
            + `! fakesink )`;

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
    async initialize() {
        let self = this;
        return new Promise((resolve, reject) => {
            self.instance_.CreateRtspAnalyzer(self.id_, "", self.launch_, (code, data) => {
                if (code == 0) { resolve(data); }
                else { reject(data); }
            });
        });
    }
    async terminate() {
        super.terminate();
        this.api_.End();
        this.api_.delete();
        this.api_ = null;

        global.gc();
    }
    analyze_spectrum(magnitude) {
        let freq = this.option.audio.freq;
        let bands = this.option.audio.bands;
        let rate = this.option.audio.rate;

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
        // console.log(ms)
        let diff = ms - ocr;
        if (this.pre_image_data_ != 0) {
            if (Math.abs(this.pre_image_data_ - diff) < 5)
                this.image_passed_++;
        }
        this.pre_image_data_ = diff;
    }
    analyze_done() {
        return this.audio_passed_ >= 5 && this.image_passed_ >= 5;
    }
}
module.exports = {
    RtspAnalyzer
}