const path = require('path');

process.env.Path += ";";
process.env.Path += path.resolve(__dirname, 'dist', 'bin');

const packageName = 'noobs.node';
const noobs = require(`./dist/${packageName}`)
module.exports = noobs;
