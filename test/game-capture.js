const noobs = require('../index.js');
const path = require('path');

async function test() {
  console.log('Starting obs...');

  const cb = (msg) => {
    console.log('Callback received:', msg);
  };

  const pluginPath = path.resolve(__dirname, '../dist', 'obs-plugins');
  const logPath = path.resolve(__dirname, '../logs');
  const dataPath = path.resolve(__dirname, '../dist', 'data');
  const recordingPath = path.resolve(__dirname, '../recordings');

  console.log('Plugin path:', pluginPath);
  console.log('Log path:', logPath);
  console.log('Data path:', dataPath);
  console.log('Recording path:', recordingPath);

  noobs.Init(pluginPath, logPath, dataPath, recordingPath, cb);

  console.log('Creating source...');
  noobs.CreateSource('Test Source', 'game_capture');
  const p = noobs.GetSourceProperties('Test Source');
  console.log('AHKKKK');
  console.log(p);
  console.log(p[1].items);

  const s = noobs.GetSourceSettings('Test Source');

  noobs.SetSourceSettings('Test Source', {
    ...s,
    capture_mode: 'any_fullscreen',
    // window: 'World of Warcraft:waApplication Window:Wow.exe',
  });

  const s1 = noobs.GetSourceSettings('Test Source');
  console.log('s1', s1);

  noobs.AddSourceToScene('Test Source');

  // Start the recording, with 1s offset into the past.
  noobs.StartRecording();
  await new Promise((resolve) => setTimeout(resolve, 5000));

  // Stop the recording.
  noobs.StopRecording();
  await new Promise((resolve) => setTimeout(resolve, 2000));

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
