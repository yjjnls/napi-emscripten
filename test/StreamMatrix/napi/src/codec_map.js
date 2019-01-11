var codec_table = {
    g711a: {
        enc: 'alawenc',
        dec: 'alawdec',
        pay: 'rtppcmapay',
        depay: 'rtppcmadepay',
        caps: 'application/x-rtp,media=audio,encoding-name=PCMA,payload=8'
    },

    g711u: {
        enc: 'mulawenc',
        dec: 'mulawdec',
        pay: 'rtppcmupay',
        depay: 'rtppcmudepay'
    },

    opus: {
        enc: 'opusenc',
        dec: 'opusdec',
        pay: 'rtpopuspay',
        depay: 'rtpopusdepay',
        caps: 'application/x-rtp,media=audio,payload=97,encoding-name=OPUS'
    },


    // Video
    h264: {
        enc: 'x264enc',
        dec: 'avdec_h264',
        pay: 'rtph264pay config-interval=-1',
        depay: 'rtph264depay ! h264parse',
        caps: 'application/x-rtp,media=video,encoding-name=H264,payload=96'
    },

    openh264: {
        enc: 'openh264enc',
        dec: 'openh264dec',
        pay: 'rtph264pay',
        depay: 'rtph264depay'
    },

    mjpeg: {
        enc: 'jpegenc',
        dec: 'jpegdec',
        pay: 'rtpjpegpay',
        depay: 'rtpjpegdepay'
    },

    vp8: {
        enc: 'vp8enc',
        dec: 'vp8dec',
        pay: 'rtpvp8pay',
        depay: 'rtpvp8depay',
        caps: 'application/x-rtp,media=video,payload=96,encoding-name=VP8'
    }
};

codec_table.pcma = codec_table.g711a;
codec_table.pcmu = codec_table.g711u;

module.exports = {
    TABLE: codec_table
};