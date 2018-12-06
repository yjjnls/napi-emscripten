const Module = require('./build/tesseract.js');
Module.onRuntimeInitialized = function () {
    console.log('-------------start------------')
    let api = new Module.TessBaseAPI();
    // api.Init("/opt", "eng");
    
    // // let image = Module.pixRead("./cpp/test.bmp");
    // let image = Module.pixRead("../cosmic.png");
    // api.SetImage(image);
    
    // let result = api.GetUTF8Text();
    
    // console.log(result);
    
    // api.End();
    // Module.pixDestroy(image);
    
    api.delete();
    api = null;
    
    global.gc();
}