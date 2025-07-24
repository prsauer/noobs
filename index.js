const path = require('path');

process.env.Path += ";";
process.env.Path += path.resolve(__dirname, 'dist', 'bin', '64bit');

const obs = require('./dist/warcraft-recorder-obs-engine.node')
module.exports = obs;
