const path = require('path');

process.env.Path += ';';
process.env.Path += path
  .resolve(__dirname, 'dist', 'bin')
  .replace('app.asar', 'app.asar.unpacked');

const packageName = 'noobs.node';
// Calling toString on the result of join ensures that webpack won't do anything too crazy
//  with the result but will still give you __dirname expanding
const noobs = require(path.join(__dirname, 'dist', packageName).toString());
module.exports = noobs;
