var forsakeN = require('./build/Release/forsaken.node');
var forsake = module.exports = {};

var PAD = forsakeN.RSA_PKCS1_PADDING;
var supported = [];

Object.keys(forsakeN).forEach(function (key) {
    var value = forsakeN[key];

    if (typeof value !== 'function') {
        supported.push(forsakeN[key]);
        return forsake[key] = forsakeN[key];
    }

    forsake[key] = function (input, key, opts) {
        var padding = PAD;
        if (typeof opts === 'object' && ~supported.indexOf(+opts.padding)) {
            padding = +opts.padding;
        }

        var passphrase = key.passphrase || null;
        key = key.key || key;

        if (!Buffer.isBuffer(input))
            input = new Buffer(input);
        if (!Buffer.isBuffer(key))
            key = new Buffer(key);

        return value(input, key, passphrase, padding);
    };
});