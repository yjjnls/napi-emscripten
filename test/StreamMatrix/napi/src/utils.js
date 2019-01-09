function parseTime(str) {
    var reg = /^(\d{1}):(\d{2}):(\d{2}).(\d{3})$/;
    var r = str.match(reg);
    if (!r) return;
    const Factor = [60 * 60 * 1000, 60 * 1000, 1000, 1];
    var ms = 0;
    Factor.forEach((factor, i) => {
        ms += parseInt(r[i + 1]) * factor;
    });
    return ms;
}

function poll(func, tick = 100, timeout = 3 * 1000, err_msg = 'poll time out!') {
    let elapse = 0;
    return new Promise(function (resolve, reject) {
        let interval = setInterval(function () {
            elapse += tick;
            if (func()) {
                clearInterval(interval);
                resolve();
            }

            if (elapse > timeout) {
                clearInterval(interval);
                reject(err_msg);
            }
        }, tick);
    });
}

module.exports = {
    parseTime: parseTime,
    poll: poll
};