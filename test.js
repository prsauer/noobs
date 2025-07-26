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
  await new Promise((resolve) => setTimeout(resolve, 1000));

  console.log('Creating source...');
  noobs.CreateSource('Test Source', 'monitor_capture');

  const settings1 = noobs.GetSourceSettings('Test Source');
  console.log(settings1);
  noobs.SetSourceSettings('Test Source', { ...settings1, monitor: 1 });

  const settings2 = noobs.GetSourceSettings('Test Source');
  console.log(settings2);

  const properties = noobs.GetSourceProperties('Test Source');
  console.log('Source properties:', properties);
  console.log('Source properties:', properties[0].items);

  await new Promise((resolve) => setTimeout(resolve, 1000));

  for (let i = 0; i < 2; i++) {
    console.log('Test Recording Loop:', i + 1);
    // noobs.StartBuffer();
    // await new Promise(resolve => setTimeout(resolve, 5000));

    noobs.StartRecording(1);
    await new Promise((resolve) => setTimeout(resolve, 5000));

    console.log('Adding source to scene...');
    noobs.AddSourceToScene('Test Source');
    await new Promise((resolve) => setTimeout(resolve, 1000));
    console.log('Removing source from scene...');
    noobs.RemoveSourceFromScene('Test Source');
    await new Promise((resolve) => setTimeout(resolve, 1000));

    noobs.StopRecording();
    await new Promise((resolve) => setTimeout(resolve, 5000));

    const last = noobs.GetLastRecording();
    console.log('Last recording:', last);

    await new Promise((resolve) => setTimeout(resolve, 1000));
  }

  console.log('Stopping obs...');
  noobs.Shutdown();

  console.log('Test Done');
}

console.log('Starting test...');
test();
console.log('Test now running async');
