const bindingPath = require.resolve(`./build/Release/plugin.node`);

var cv = require(bindingPath); // eslint-disable-line no-var

// var helper = require('../helper.js');

var Module = {}
Module['imread'] = function (imageSource) {
    var img = null;
    if (typeof imageSource === 'string') {
        img = document.getElementById(imageSource);
    } else {
        img = imageSource;
    }
    var canvas = null;
    var ctx = null;
    if (img instanceof HTMLImageElement) {
        canvas = document.createElement('canvas');
        canvas.width = img.width;
        canvas.height = img.height;
        ctx = canvas.getContext('2d');
        ctx.drawImage(img, 0, 0, img.width, img.height);
    } else if (img instanceof HTMLCanvasElement) {
        canvas = img;
        ctx = canvas.getContext('2d');
    } else {
        throw new Error('Please input the valid canvas or img id.');
        return;
    }

    var imgData = ctx.getImageData(0, 0, canvas.width, canvas.height);
    return cv.matFromImageData(imgData);
};

Module['imshow'] = function (canvasSource, mat) {
    var canvas = null;
    if (typeof canvasSource === 'string') {
        canvas = document.getElementById(canvasSource);
    } else {
        canvas = canvasSource;
    }
    if (!(canvas instanceof HTMLCanvasElement)) {
        throw new Error('Please input the valid canvas element or id.');
        return;
    }
    if (!(mat instanceof cv.Mat)) {
        throw new Error('Please input the valid cv.Mat instance.');
        return;
    }

    // convert the mat type to cv.CV_8U
    var img = new cv.Mat();
    var depth = mat.type() % 8;
    var scale = depth <= cv.CV_8S ? 1.0 : (depth <= cv.CV_32S ? 1.0 / 256.0 : 255.0);
    var shift = (depth === cv.CV_8S || depth === cv.CV_16S) ? 128.0 : 0.0;
    mat.convertTo(img, cv.CV_8U, scale, shift);

    // convert the img type to cv.CV_8UC4
    switch (img.type()) {
        case cv.CV_8UC1:
            cv.cvtColor(img, img, cv.COLOR_GRAY2RGBA);
            break;
        case cv.CV_8UC3:
            cv.cvtColor(img, img, cv.COLOR_RGB2RGBA);
            break;
        case cv.CV_8UC4:
            break;
        default:
            throw new Error('Bad number of channels (Source image must have 1, 3 or 4 channels)');
            return;
    }
    var imgData = new ImageData(new Uint8ClampedArray(img.data), img.cols, img.rows);
    var ctx = canvas.getContext('2d');
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    canvas.width = imgData.width;
    canvas.height = imgData.height;
    ctx.putImageData(imgData, 0, 0);
    img.delete();
};

Module['VideoCapture'] = function (videoSource) {
    var video = null;
    if (typeof videoSource === 'string') {
        video = document.getElementById(videoSource);
    } else {
        video = videoSource;
    }
    if (!(video instanceof HTMLVideoElement)) {
        throw new Error('Please input the valid video element or id.');
        return;
    }
    var canvas = document.createElement('canvas');
    canvas.width = video.width;
    canvas.height = video.height;
    var ctx = canvas.getContext('2d');
    this.video = video;
    this.read = function (frame) {
        if (!(frame instanceof cv.Mat)) {
            throw new Error('Please input the valid cv.Mat instance.');
            return;
        }
        if (frame.type() !== cv.CV_8UC4) {
            throw new Error('Bad type of input mat: the type should be cv.CV_8UC4.');
            return;
        }
        if (frame.cols !== video.width || frame.rows !== video.height) {
            throw new Error('Bad size of input mat: the size should be same as the video.');
            return;
        }
        ctx.drawImage(video, 0, 0, video.width, video.height);
        frame.data.set(ctx.getImageData(0, 0, video.width, video.height).data);
    };
};

function Range(start, end) {
    this.start = typeof (start) === 'undefined' ? 0 : start;
    this.end = typeof (end) === 'undefined' ? 0 : end;
}

Module['Range'] = Range;

function Point(x, y) {
    this.x = typeof (x) === 'undefined' ? 0 : x;
    this.y = typeof (y) === 'undefined' ? 0 : y;
}

Module['Point'] = Point;

function Size(width, height) {
    this.width = typeof (width) === 'undefined' ? 0 : width;
    this.height = typeof (height) === 'undefined' ? 0 : height;
}

Module['Size'] = Size;

function Rect() {
    switch (arguments.length) {
        case 0: {
            // new cv.Rect()
            this.x = 0;
            this.y = 0;
            this.width = 0;
            this.height = 0;
            break;
        }
        case 1: {
            // new cv.Rect(rect)
            var rect = arguments[0];
            this.x = rect.x;
            this.y = rect.y;
            this.width = rect.width;
            this.height = rect.height;
            break;
        }
        case 2: {
            // new cv.Rect(point, size)
            var point = arguments[0];
            var size = arguments[1];
            this.x = point.x;
            this.y = point.y;
            this.width = size.width;
            this.height = size.height;
            break;
        }
        case 4: {
            // new cv.Rect(x, y, width, height)
            this.x = arguments[0];
            this.y = arguments[1];
            this.width = arguments[2];
            this.height = arguments[3];
            break;
        }
        default: {
            throw new Error('Invalid arguments');
        }
    }
}

Module['Rect'] = Rect;

function RotatedRect() {
    switch (arguments.length) {
        case 0: {
            this.center = { x: 0, y: 0 };
            this.size = { width: 0, height: 0 };
            this.angle = 0;
            break;
        }
        case 3: {
            this.center = arguments[0];
            this.size = arguments[1];
            this.angle = arguments[2];
            break;
        }
        default: {
            throw new Error('Invalid arguments');
        }
    }
}

RotatedRect.points = function (obj) {
    return Module.rotatedRectPoints(obj);
};

RotatedRect.boundingRect = function (obj) {
    return Module.rotatedRectBoundingRect(obj);
};

RotatedRect.boundingRect2f = function (obj) {
    return Module.rotatedRectBoundingRect2f(obj);
};

Module['RotatedRect'] = RotatedRect;

function Scalar(v0, v1, v2, v3) {
    this.push(typeof (v0) === 'undefined' ? 0 : v0);
    this.push(typeof (v1) === 'undefined' ? 0 : v1);
    this.push(typeof (v2) === 'undefined' ? 0 : v2);
    this.push(typeof (v3) === 'undefined' ? 0 : v3);
}

Scalar.prototype = new Array; // eslint-disable-line no-array-constructor

Scalar.all = function (v) {
    return new Scalar(v, v, v, v);
};

Module['Scalar'] = Scalar;

function MinMaxLoc() {
    switch (arguments.length) {
        case 0: {
            this.minVal = 0;
            this.maxVal = 0;
            this.minLoc = new Point();
            this.maxLoc = new Point();
            break;
        }
        case 4: {
            this.minVal = arguments[0];
            this.maxVal = arguments[1];
            this.minLoc = arguments[2];
            this.maxLoc = arguments[3];
            break;
        }
        default: {
            throw new Error('Invalid arguments');
        }
    }
}

Module['MinMaxLoc'] = MinMaxLoc;

function Circle() {
    switch (arguments.length) {
        case 0: {
            this.center = new Point();
            this.radius = 0;
            break;
        }
        case 2: {
            this.center = arguments[0];
            this.radius = arguments[1];
            break;
        }
        default: {
            throw new Error('Invalid arguments');
        }
    }
}

Module['Circle'] = Circle;

function TermCriteria() {
    switch (arguments.length) {
        case 0: {
            this.type = 0;
            this.maxCount = 0;
            this.epsilon = 0;
            break;
        }
        case 3: {
            this.type = arguments[0];
            this.maxCount = arguments[1];
            this.epsilon = arguments[2];
            break;
        }
        default: {
            throw new Error('Invalid arguments');
        }
    }
}

Module['TermCriteria'] = TermCriteria;

Module['matFromArray'] = function (rows, cols, type, array) {
    var mat = new cv.Mat(rows, cols, type);
    switch (type) {
        case cv.CV_8U:
        case cv.CV_8UC1:
        case cv.CV_8UC2:
        case cv.CV_8UC3:
        case cv.CV_8UC4: {
            mat.data.set(array);
            break;
        }
        case cv.CV_8S:
        case cv.CV_8SC1:
        case cv.CV_8SC2:
        case cv.CV_8SC3:
        case cv.CV_8SC4: {
            mat.data8S.set(array);
            break;
        }
        case cv.CV_16U:
        case cv.CV_16UC1:
        case cv.CV_16UC2:
        case cv.CV_16UC3:
        case cv.CV_16UC4: {
            mat.data16U.set(array);
            break;
        }
        case cv.CV_16S:
        case cv.CV_16SC1:
        case cv.CV_16SC2:
        case cv.CV_16SC3:
        case cv.CV_16SC4: {
            mat.data16S.set(array);
            break;
        }
        case cv.CV_32S:
        case cv.CV_32SC1:
        case cv.CV_32SC2:
        case cv.CV_32SC3:
        case cv.CV_32SC4: {
            mat.data32S.set(array);
            break;
        }
        case cv.CV_32F:
        case cv.CV_32FC1:
        case cv.CV_32FC2:
        case cv.CV_32FC3:
        case cv.CV_32FC4: {
            mat.data32F.set(array);
            break;
        }
        case cv.CV_64F:
        case cv.CV_64FC1:
        case cv.CV_64FC2:
        case cv.CV_64FC3:
        case cv.CV_64FC4: {
            mat.data64F.set(array);
            break;
        }
        default: {
            throw new Error('Type is unsupported');
        }
    }
    return mat;
};

Module['matFromImageData'] = function (imageData) {
    var mat = new cv.Mat(imageData.height, imageData.width, cv.CV_8UC4);
    mat.data.set(imageData.data);
    return mat;
};


/********************************************************************** */
var HEAP,
    /** @type {ArrayBuffer} */
    buffer,
    /** @type {Int8Array} */
    HEAP8,
    /** @type {Uint8Array} */
    HEAPU8,
    /** @type {Int16Array} */
    HEAP16,
    /** @type {Uint16Array} */
    HEAPU16,
    /** @type {Int32Array} */
    HEAP32,
    /** @type {Uint32Array} */
    HEAPU32,
    /** @type {Float32Array} */
    HEAPF32,
    /** @type {Float64Array} */
    HEAPF64;

var ref = require('ref');
var buffer = new ArrayBuffer(1024 * 1024 * 100);
const address = Buffer.from(buffer).address();
Module['HEAP8'] = HEAP8 = new Int8Array(buffer);
Module['HEAP16'] = HEAP16 = new Int16Array(buffer);
Module['HEAP32'] = HEAP32 = new Int32Array(buffer);
Module['HEAPU8'] = HEAPU8 = new Uint8Array(buffer);
Module['HEAPU16'] = HEAPU16 = new Uint16Array(buffer);
Module['HEAPU32'] = HEAPU32 = new Uint32Array(buffer);
Module['HEAPF32'] = HEAPF32 = new Float32Array(buffer);
Module['HEAPF64'] = HEAPF64 = new Float64Array(buffer);
var malloc_offset = 0;
// Module['malloc'] = (num) => { return HEAPU8.byteOffset + num }

class Mat {
    constructor() {
        switch (arguments.length) {
            case 0: return new cv.Mat();
            case 1: return new cv.Mat(arguments[0]);
            case 2: return new cv.Mat(arguments[0], arguments[1]);
            case 3: return new cv.Mat(arguments[0], arguments[1], arguments[2]);
            case 4: return new cv.Mat(arguments[0], arguments[1], arguments[2], arguments[3]);
            case 5: return new cv.Mat(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4]);
        }
        // return this.mat
    }
    // unwrap(arg) {
    //     if (arg instanceof Mat)
    //         return arg.mat;
    //     else
    //         return arg;
    // }
    // delete() {
    //     return this.mat.delete();
    // }
    // isDeleted() {
    //     return this.mat.isDeleted();
    // }
    static ones() {
        switch (arguments.length) {
            case 2: return new Mat(cv.ones(arguments[0], arguments[1])); break;
            case 3: return new Mat(cv.ones(arguments[0], arguments[1], arguments[2])); break;
        }
    }
    static eye() {
        switch (arguments.length) {
            case 2: return new Mat(cv.eye(arguments[0], arguments[1])); break;
            case 3: return new Mat(cv.eye(arguments[0], arguments[1], arguments[2])); break;
        }
    }
    static zeros() {
        switch (arguments.length) {
            case 2: return new Mat(cv.zeros(arguments[0], arguments[1])); break;
            case 3: return new Mat(cv.zeros(arguments[0], arguments[1], arguments[2])); break;
        }
    }
    // get rows() {
    //     return this.mat.rows;
    // }
    // set rows(data) {
    //     this.mat.rows = data;
    // }
    // get cols() {
    //     return this.mat.cols;
    // }
    // set cols(data) {
    //     this.mat.cols = data;
    // }
    // get matSize() {
    //     return this.mat.matSize;
    // }
    // get step() {
    //     return this.mat.step;
    // }
    // get data() {
    //     return this.mat.data;
    // }
    // get data8S() {
    //     return this.mat.data8S;
    // }
    // get data16U() {
    //     return this.mat.data16U;
    // }
    // get data16S() {
    //     return this.mat.data16S;
    // }
    // get data32F() {
    //     return this.mat.data32F;
    // }
    // get data64F() {
    //     return this.mat.data64F;
    // }

    // elemSize() {
    //     return this.mat.elemSize();
    // }
    // elemSize1() {
    //     return this.mat.elemSize1();
    // }
    // channels() {
    //     return this.mat.channels();
    // }
    // convertTo(arg0, arg1, arg2, arg3) {
    //     switch (arguments.length) {
    //         case 2: return this.mat.convertTo(arg0.mat, arg1); break;
    //         case 3: return this.mat.convertTo(arg0.mat, arg1, arg2); break;
    //         case 4: return this.mat.convertTo(arg0.mat, arg1, arg2, arg3); break;
    //     }
    // }
    // total() {
    //     return this.mat.total();
    // }
    // row(arg0) {
    //     return this.mat.row(arg0);
    // }
    // create(arg0, arg1, arg2) {
    //     switch (arguments.length) {
    //         case 2: return this.mat.create(arg0, arg1); break;
    //         case 3: return this.mat.create(arg0, arg1, arg2); break;
    //     }
    // }
    // rowRange(arg0, arg1) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.rowRange(arg0); break;
    //         case 2: return this.mat.rowRange(arg0, arg1); break;
    //     }
    // }
    // copyTo(arg0, arg1) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.copyTo(arg0.mat); break;
    //         case 2: return this.mat.copyTo(arg0.mat, arg1.mat); break;
    //     }
    // }
    // type() {
    //     return this.mat.type();
    // }
    // empty() {
    //     return this.mat.empty();
    // }
    // colRange(arg0, arg1) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.colRange(arg0); break;
    //         case 2: return this.mat.colRange(arg0, arg1); break;
    //     }
    // }
    // step1(arg0) {
    //     return this.mat.step1(arg0);
    // }
    // clone() {
    //     return this.mat.clone();
    // }
    // depth() {
    //     return this.mat.depth();
    // }
    // col(arg0) {
    //     return this.mat.col(arg0);
    // }
    // dot(arg0) {
    //     return this.mat.dot(arg0.mat);
    // }
    // mul(arg0, arg1) {
    //     return this.mat.mul(arg0.mat, arg1);
    // }
    // inv(arg0) {
    //     return this.mat.inv(arg0);
    // }
    // t() {
    //     return this.mat.t();
    // }
    // roi(arg0) {
    //     return this.mat.roi(arg0);
    // }
    // diag(arg0, arg1) {
    //     switch (arguments.length) {
    //         case 0: return this.mat.diag(); break;
    //         case 1: return this.mat.diag(arg0); break;
    //     }
    // }
    // isContinuous() {
    //     return this.mat.isContinuous();
    // }
    // setTo(arg0, arg1) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.setTo(arg0); break;
    //         case 2: return this.mat.setTo(arg0, arg1); break;
    //     }
    // }
    // size() {
    //     return this.mat.size();
    // }
    // ptr(arg0, arg1) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.ptr(arg0); break;
    //         case 2: return this.mat.ptr(arg0, arg1); break;
    //     }
    // }
    // ucharPtr(arg0, arg1) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.ucharPtr(arg0); break;
    //         case 2: return this.mat.ucharPtr(arg0, arg1); break;
    //     }
    // }
    // charPtr(arg0, arg1) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.charPtr(arg0); break;
    //         case 2: return this.mat.charPtr(arg0, arg1); break;
    //     }
    // }
    // shortPtr(arg0, arg1) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.shortPtr(arg0); break;
    //         case 2: return this.mat.shortPtr(arg0, arg1); break;
    //     }
    // }
    // ushortPtr(arg0, arg1) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.ushortPtr(arg0); break;
    //         case 2: return this.mat.ushortPtr(arg0, arg1); break;
    //     }
    // }
    // intPtr(arg0, arg1) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.intPtr(arg0); break;
    //         case 2: return this.mat.intPtr(arg0, arg1); break;
    //     }
    // }
    // floatPtr(arg0, arg1) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.floatPtr(arg0); break;
    //         case 2: return this.mat.floatPtr(arg0, arg1); break;
    //     }
    // }
    // doublePtr(arg0, arg1) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.doublePtr(arg0); break;
    //         case 2: return this.mat.doublePtr(arg0, arg1); break;
    //     }
    // }
    // charAt(arg0, arg1, arg2) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.charAt(arg0); break;
    //         case 2: return this.mat.charAt(arg0, arg1); break;
    //         case 3: return this.mat.charAt(arg0, arg1, arg2); break;
    //     }
    // }
    // ucharAt(arg0, arg1, arg2) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.ucharAt(arg0); break;
    //         case 2: return this.mat.ucharAt(arg0, arg1); break;
    //         case 3: return this.mat.ucharAt(arg0, arg1, arg2); break;
    //     }
    // }
    // shortAt(arg0, arg1, arg2) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.shortAt(arg0); break;
    //         case 2: return this.mat.shortAt(arg0, arg1); break;
    //         case 3: return this.mat.shortAt(arg0, arg1, arg2); break;
    //     }
    // }
    // ushortAt(arg0, arg1, arg2) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.ushortAt(arg0); break;
    //         case 2: return this.mat.ushortAt(arg0, arg1); break;
    //         case 3: return this.mat.ushortAt(arg0, arg1, arg2); break;
    //     }
    // }
    // intAt(arg0, arg1, arg2) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.intAt(arg0); break;
    //         case 2: return this.mat.intAt(arg0, arg1); break;
    //         case 3: return this.mat.intAt(arg0, arg1, arg2); break;
    //     }
    // }
    // floatAt(arg0, arg1, arg2) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.floatAt(arg0); break;
    //         case 2: return this.mat.floatAt(arg0, arg1); break;
    //         case 3: return this.mat.floatAt(arg0, arg1, arg2); break;
    //     }
    // }
    // doubleAt(arg0, arg1, arg2) {
    //     switch (arguments.length) {
    //         case 1: return this.mat.doubleAt(arg0); break;
    //         case 2: return this.mat.doubleAt(arg0, arg1); break;
    //         case 3: return this.mat.doubleAt(arg0, arg1, arg2); break;
    //     }
    // }
}

class HOGDescriptor {
    constructor() {
        switch (arguments.length) {
            case 0: return new cv.HOGDescriptor();
            case 1: return new cv.HOGDescriptor(arguments[0]);
            case 12: return new cv.HOGDescriptor(arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5], arguments[6], arguments[7], arguments[8], arguments[9], arguments[10], arguments[10]);
        }
    }

    static getDefaultPeopleDetector() {
        return cv.getDefaultPeopleDetector();
    }
    static getDaimlerPeopleDetector() {
        return cv.getDaimlerPeopleDetector();
    }
}

class _RotatedRect {
    constructor() {
        switch (arguments.length) {
            case 0: return new cv.RotatedRect();
            case 3: return new cv.RotatedRect(arguments[0], arguments[1], arguments[2]);
        }
    }
    static points(arg0) {
        return cv.rotatedRectPoints(arg0);
    }
    static boundingRect2f(arg0) {
        return cv.rotatedRectBoundingRect2f(arg0);
    };
}
module.exports = {
    //////////////////////////
    // '_malloc': cv._malloc,
    '_malloc': (size) => {
        var buf = Buffer.from(HEAPU8.buffer, malloc_offset, size);
        malloc_offset += Math.ceil(size / 8) * 8;
        return buf.address() - address;
    },
    'address': address,
    '_free': () => { },
    //////////////////////////
    // 'Mat': cv.Mat,
    'Mat': Mat,
    'Range': cv.Range,
    'TermCriteria': cv.TermCriteria,
    'Size': cv.Size,
    'Size2f': cv.Size2f,
    'Point': cv.Point,
    'Point2f': cv.Point2f,
    'Rect': cv.Rect,
    'Rect2f': cv.Rect2f,
    'RotatedRect': _RotatedRect,
    'MinMaxLoc': cv.MinMaxLoc,
    'Circle': cv.Circle,
    'Moments': cv.Moments,
    'Exception': cv.Exception,
    'eye': cv.eye,
    'ones': cv.ones,
    'zeros': cv.zeros,
    'Scalar': cv.Scalar,
    'CV_8UC1': cv.CV_8UC1,
    'CV_8UC2': cv.CV_8UC2,
    'CV_8UC3': cv.CV_8UC3,
    'CV_8UC4': cv.CV_8UC4,
    'CV_8SC1': cv.CV_8SC1,
    'CV_8SC2': cv.CV_8SC2,
    'CV_8SC3': cv.CV_8SC3,
    'CV_8SC4': cv.CV_8SC4,
    'CV_16UC1': cv.CV_16UC1,
    'CV_16UC2': cv.CV_16UC2,
    'CV_16UC3': cv.CV_16UC3,
    'CV_16UC4': cv.CV_16UC4,
    'CV_16SC1': cv.CV_16SC1,
    'CV_16SC2': cv.CV_16SC2,
    'CV_16SC3': cv.CV_16SC3,
    'CV_16SC4': cv.CV_16SC4,
    'CV_32SC1': cv.CV_32SC1,
    'CV_32SC2': cv.CV_32SC2,
    'CV_32SC3': cv.CV_32SC3,
    'CV_32SC4': cv.CV_32SC4,
    'CV_32FC1': cv.CV_32FC1,
    'CV_32FC2': cv.CV_32FC2,
    'CV_32FC3': cv.CV_32FC3,
    'CV_32FC4': cv.CV_32FC4,
    'CV_64FC1': cv.CV_64FC1,
    'CV_64FC2': cv.CV_64FC2,
    'CV_64FC3': cv.CV_64FC3,
    'CV_64FC4': cv.CV_64FC4,
    'CV_8U': cv.CV_8U,
    'CV_8S': cv.CV_8S,
    'CV_16U': cv.CV_16U,
    'CV_16S': cv.CV_16S,
    'CV_32S': cv.CV_32S,
    'CV_32F': cv.CV_32F,
    'CV_64F': cv.CV_64F,
    'INT_MIN': cv.INT_MIN,
    'INT_MAX': cv.INT_MAX,
    'minMaxLoc': cv.minMaxLoc,
    'IntVector': cv.IntVector,
    'FloatVector': cv.FloatVector,
    'DoubleVector': cv.DoubleVector,
    'PointVector': cv.PointVector,
    'MatVector': cv.MatVector,
    'RectVector': cv.RectVector,
    'matFromArray': Module['matFromArray'],
    //////////////////////////
    'HEAP8': Module['HEAP8'],
    'HEAP16': Module['HEAP16'],
    'HEAP32': Module['HEAP32'],
    'HEAPU8': Module['HEAPU8'],
    'HEAPU16': Module['HEAPU16'],
    'HEAPU32': Module['HEAPU32'],
    'HEAPF32': Module['HEAPF32'],
    'HEAPF64': Module['HEAPF64'],
    //////////////////////////
    'split': cv.split,
    'calcHist': cv.calcHist,
    'cvtColor': cv.cvtColor,
    'COLOR_BGR2GRAY': cv.COLOR_BGR2GRAY,
    'COLOR_BGR2BGRA': cv.COLOR_BGR2BGRA,
    'equalizeHist': cv.equalizeHist,
    'threshold': cv.threshold,
    'adaptiveThreshold': cv.adaptiveThreshold,
    'moments': cv.moments,
    'contourArea': cv.contourArea,
    'minEnclosingCircle': cv.minEnclosingCircle,
    'blur': cv.blur,
    'GaussianBlur': cv.GaussianBlur,
    'medianBlur': cv.medianBlur,
    'transpose': cv.transpose,
    'bilateralFilter': cv.bilateralFilter,
    'watershed': cv.watershed,
    'vconcat': cv.vconcat,
    'hconcat': cv.hconcat,
    'distanceTransform': cv.distanceTransform,
    'distanceTransformWithLabels': cv.distanceTransformWithLabels,
    'integral': cv.integral,
    'integral2': cv.integral2,
    'DIST_L2': cv.DIST_L2,
    'DIST_LABEL_CCOMP': cv.DIST_LABEL_CCOMP,
    "BORDER_DEFAULT": cv.BORDER_DEFAULT,
    'groupRectangles': cv.groupRectangles,
    'CascadeClassifier': cv.CascadeClassifier,
    'HOGDescriptor': HOGDescriptor,
    'meanShift': cv.meanShift,
    'min': cv.min,
    'max': cv.max,
    'bitwise_not': cv.bitwise_not,
    'bitwise_and': cv.bitwise_and,
    'bitwise_or': cv.bitwise_or,
    'bitwise_xor': cv.bitwise_xor,
    'absdiff': cv.absdiff,
    'add': cv.add,
    'addWeighted': cv.addWeighted,
    'invert': cv.invert

}