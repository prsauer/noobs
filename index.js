const obs = require('./build/Release/wcr-obs-engine.node')

console.log('addon', obs.getUptime())
module.exports = obs