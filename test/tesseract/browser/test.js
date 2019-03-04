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
                response.writeHead(200, { "Content-Type": "text/javascript" });
            } else if (/\.css/.test(filePath)) {
                response.writeHead(200, { "Content-Type": "text/css" });
            } else if (/\.html/.test(filePath)) {
                response.writeHead(200, { "Content-Type": "text/html" });
            } else if (/\.webm/.test(filePath)) {
                response.writeHead(200, { "Content-Type": "application/octet-stream" });
            } else {
                response.writeHead(200, { "Content-Type": "text/plain" });
            }
            response.write(file, 'binary');
            response.end();
        }
    });
}).listen(8088);