'use strict';

let option = {
    path: '/test',
    video: {
        codec: 'h264',
        pattern: 'white',
        width: 320,
        height: 240,
        fps: 10,
        bitrate: 512,
        kfi: 5, // Maximal distance between two key-frames (0 for automatic)
        overlay: {
            "type": 'time',
            "text": '',
            "font": 'Sans 48', // this font size is suitable for tesseract ocr to recognize correctly
            'draw-shadow': false,
            'draw-outline': false,
            'outline-color': 0xFF000000, // big-endian ARGB
            "xpos": 0, // float 0 - 1
            "ypos": 0, // float 0 - 1
            "color": 0xFF000000 // text color big-endian ARGB
        }
    },
    image: {
        fps: 10,
        location: 'tmp/%05d.jpg'
    },
    audio: {
        codec: 'g711a',
        freq: 440,
        rate: 8000,
        threshold: -60,
        bands: 128,
        channels: 1
    }
};

module.exports = {
    option
};