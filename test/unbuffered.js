const noobs = require('../index.js');
const path = require('path');

async function test() {
  console.log('Starting obs...');

  const cb = (msg) => {
    console.log('Callback received:', msg);
  };

  const pluginPath = path.resolve(__dirname, '../dist', 'plugins');
  const logPath = path.resolve(__dirname, '../logs');
  const dataPath = path.resolve(__dirname, '../dist', 'effects');
  const recordingPath = path.resolve(__dirname, '../recordings');

  console.log('Plugin path:', pluginPath);
  console.log('Log path:', logPath);
  console.log('Data path:', dataPath);
  console.log('Recording path:', recordingPath);

  noobs.Init(pluginPath, logPath, dataPath, recordingPath, cb);

  console.log('Creating source...');
  noobs.CreateSource('Test Source', 'monitor_capture');

  const settings1 = noobs.GetSourceSettings('Test Source');
  
  console.log(settings1);

  const p = noobs.GetSourceProperties('Test Source');
  console.log(p[1].items);

  // We need method 2 here which is WGC. We can't use 0 (Auto) or 1 (DCGI) unless we have a graphics context.
  // Or atlaest that's what ChatGPT thinks. This is only an issue when running as a CLI job. If we run in the context of an
  // electron process it just works fine.
  noobs.SetSourceSettings('Test Source', { ...settings1, monitor_id: p[1].items[1].value, method: 2 });

  const settings2 = noobs.GetSourceSettings('Test Source');
  console.log(settings2);

  console.log('Adding source to scene...');
  noobs.AddSourceToScene('Test Source');

  // Start the recording, with 1s offset into the past.
  noobs.StartRecording();
  await new Promise((resolve) => setTimeout(resolve, 5000));

  // Stop the recording.
  noobs.StopRecording();
  await new Promise((resolve) => setTimeout(resolve, 1000));

  // Get the path to the last recording.
  const last = noobs.GetLastRecording();

  console.log('Stopping obs...');
  noobs.Shutdown();

  console.log('Last recording:', last);
  if (last == '') {
    throw new Error(`No apparent recording`);
  }
  if (last.endsWith('noobs.mp4')) {
    throw new Error(`Last recording name not correct - ${last}`);
  }
  console.log('Test Done');
}

console.log('Starting test...');
test();
console.log('Test now running async');
