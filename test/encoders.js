const noobs = require('../index.js');
const path = require('path');

async function test() {
  console.log('Starting obs...');

  const cb = (msg) => {
    console.log('Callback received:', msg);
  };

  const distPath = path.resolve(__dirname, '../dist');
  const logPath = path.resolve(__dirname, '../logs');
  const recordingPath = path.resolve(__dirname, '../recordings');

  console.log('Dist path:', distPath);
  console.log('Log path:', logPath);
  console.log('Recording path:', recordingPath);

  noobs.Init(distPath, logPath, recordingPath, cb);

  const encoders = noobs.ListVideoEncoders();
  console.log('Available video encoders:', encoders);

  noobs.SetVideoEncoder('obs_x264', { "rate_control": "CRF", "keyint_sec": 1, "crf": 22 });
  // noobs.SetVideoEncoder('h264_texture_amf', { "rate_control": "CRF", "keyint_sec": 1, "crf": 22 });

  noobs.Shutdown();
  console.log('Test Done');
}

console.log('Starting test...');
test();
console.log('Test now running async');
