const http = require('http');
const fs = require('fs');
const rootPath = '/root/Desktop/del/napi-emscripten/test/tesseract/browser';


http.createServer(function (request, response) {
    let filePath = rootPath + request.url;
    fs.readFile(filePath, 'binary', function (err, file) {
        if (err) {
            response.writeHead(500, { "Content-Type": "text/plain" });
            response.write(err + "\n");
            response.end();
            return;
        } else {
            if (/\.js/.test(filePath)) {
                response.writeHead(200, { "Content-Type": "text/javascript", "Access-Control-Allow-Origin": "*" });
            } else if (/\.css/.test(filePath)) {
                response.writeHead(200, { "Content-Type": "text/css", "Access-Control-Allow-Origin": "*" });
            } else if (/\.wasm/.test(filePath)) {
                response.writeHead(200, { "Content-Type": "application/wasm", "Access-Control-Allow-Origin": "*" });
            } else if (/\.html/.test(filePath)) {
                response.writeHead(200, { "Content-Type": "text/html", "Access-Control-Allow-Origin": "*" });
            } else if (/\.webm/.test(filePath)) {
                response.writeHead(200, { "Content-Type": "application/octet-stream", "Access-Control-Allow-Origin": "*" });
            } else if (/\.traineddata/.test(filePath)) {
                response.writeHead(200, { "Content-Type": "application/octet-stream", "Access-Control-Allow-Origin": "*" });
            } else {
                response.writeHead(200, { "Content-Type": "text/plain", "Access-Control-Allow-Origin": "*" });
            }
            response.write(file, 'binary');
            response.end();
        }
    });
}).listen(8088);