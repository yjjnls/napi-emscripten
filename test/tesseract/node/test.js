const Module = require('./build/tesseract.js');
Module.onRuntimeInitialized = function () {
    Module.onload();
    let api = new Module.TessBaseAPI();
    console.log(Module.version());
    let tessdata = require('path').resolve(__dirname, 'build/tessdata');
    api.Init("fs" + tessdata, "eng");

    let filename = require('path').resolve(__dirname, '../cosmic.png');
    console.log(filename);
    Module.test_file('fs' + filename);
    let image = Module.pixRead('fs' + filename);
    api.SetImage(image);

    let result = api.GetUTF8Text();

    console.log(result);

    api.End();
    Module.pixDestroy(image);

    api.delete();
    api = null;

    global.gc();
};