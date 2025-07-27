const noobs = require('../index.js');
const path = require('path');

async function test() {
  console.log('Starting obs...');

  const pluginPath = path.resolve(__dirname, '../dist', 'plugins');
  const logPath = path.resolve(__dirname, '../logs');
  const dataPath = path.resolve(__dirname, '../dist', 'effects');
  const recordingPath = path.resolve(__dirname, '../recordings');
  const cb = () => {};

  console.log('Plugin path:', pluginPath);
  console.log('Log path:', logPath);
  console.log('Data path:', dataPath);
  console.log('Recording path:', recordingPath);

  noobs.Init(pluginPath, logPath, dataPath, recordingPath, cb);

  // TODO - work out how to get a HWND to actually launch this. Maybe need some CPP code.
  // noobs.InitPreview(null); // Pass null for now, as we don't have a HWND yet.
  // noobs.ShowPreview(0, 0, 800, 600); // Show the preview at 800x600.
  const s = noobs.GetPreviewScaleFactor();
  console.log(s);

  noobs.Shutdown();
  console.log('Test Done');
}

console.log('Starting test...');
test();
console.log('Test now running async');
