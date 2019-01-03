'use strict';

async function sleep(timeout) {
    return new Promise((resolve, reject) => {
        setTimeout(function () {
            resolve();
        }, timeout);
    });
}

module.exports = {
    sleep
}