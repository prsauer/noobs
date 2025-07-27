const noobs = require('./index.js');
const path = require('path');

async function test() {
  console.log('Starting obs...');

  const cb = (msg) => {
    console.log('Callback received:', msg);
  };

  const pluginPath = path.resolve(__dirname, 'dist', 'plugins');
  const logPath = path.resolve(__dirname, 'logs');
  const dataPath = path.resolve(__dirname, 'dist', 'effects');
  const recordingPath = path.resolve(__dirname, 'recordings');

  console.log('Plugin path:', pluginPath);
  console.log('Log path:', logPath);
  console.log('Data path:', dataPath);
  console.log('Recording path:', recordingPath);

  noobs.Init(pluginPath, logPath, dataPath, recordingPath, cb, false);

  console.log('Creating source...');
  noobs.CreateSource('Test Source', 'monitor_capture');

  const settings1 = noobs.GetSourceSettings('Test Source');
  noobs.SetSourceSettings('Test Source', { ...settings1, monitor: 1 });

  // Start the recording, with 1s offset into the past.
  noobs.StartRecording();
  await new Promise((resolve) => setTimeout(resolve, 5000));

  // Add the source to the scene for a second.
  console.log('Adding source to scene...');
  noobs.AddSourceToScene('Test Source');
  await new Promise((resolve) => setTimeout(resolve, 1000));

  // Remove the source from the scene.
  console.log('Removing source from scene...');
  noobs.RemoveSourceFromScene('Test Source');
  await new Promise((resolve) => setTimeout(resolve, 1000));

  // Stop the recording.
  noobs.StopRecording();
  await new Promise((resolve) => setTimeout(resolve, 5000));

  // Get the path to the last recording.
  const last = noobs.GetLastRecording();

  console.log('Stopping obs...');
  noobs.Shutdown();

  console.log('Last recording:', last);
  if (last.endsWith('noobs.mp4')) {
    throw new Error(`Last recording name not correct - ${last}`);
  }
  console.log('Test Done');
}

console.log('Starting test...');
test();
console.log('Test now running async');
