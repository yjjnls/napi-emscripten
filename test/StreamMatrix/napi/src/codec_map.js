var codec_table = {
    g711a: {
        enc: 'alawenc',
        dec: 'alawdec',
        pay: 'rtppcmapay',
        depay: 'rtppcmadepay'
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
        depay: 'rtpopusdepay'
    },


    // Video
    h264: {
        enc: 'x264enc',
        dec: 'avdec_h264',
        pay: 'rtph264pay',
        depay: 'rtph264depay ! h264parse'
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
};

codec_table.pcma = codec_table.g711a;
codec_table.pcmu = codec_table.g711u;

module.exports = {
    TABLE: codec_table
};