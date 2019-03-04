const Module = require('./build/tesseract.js');
const http = require('http');

let api = null;
Module.onRuntimeInitialized = function () {
    Module.onload();
    api = new Module.TessBaseAPI();
    console.log(Module.version());
    let tessdata = require('path').resolve(__dirname, 'build/tessdata');
    api.Init("fs" + tessdata, "eng");
    api.SetVariable("tessedit_char_whitelist", "0123456789:.");
};

http.createServer(function (req, res) {
    var serverData = '';
    req.on('data', function (chunk) {
        serverData += chunk;
    });

    req.on('end', function () {
        let data = serverData.split("&");
        let width = parseInt(data[0]);
        let height = parseInt(data[1]);
        let imgbin = data[2].split(",");


        let dataPtr = Module._malloc(width * height);
        let dataHeap = new Uint8Array(Module.HEAPU8.buffer, dataPtr, width * height);
        dataHeap.set(new Uint8Array(imgbin));
        api.SetImage(dataPtr, width, height, 1, width);

        let result = api.GetUTF8Text();
        result = result.slice(0, 11);
        console.log(result);
        res.writeHead(200, { 'Content-Type': 'text/html', "Access-Control-Allow-Origin": "*" });
        res.write(result);
        res.end();

        // api.End();
        // // Module.pixDestroy(image);
        // api.delete();
        // api = null;


    });

}).listen(8089);
