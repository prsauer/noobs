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
  const sourceName = 'Test Source';
  noobs.CreateSource(sourceName, 'monitor_capture');

  const settings1 = noobs.GetSourceSettings(sourceName);

  const properties = noobs.GetSourceProperties(sourceName);
  console.log('Source properties:', { properties });
  for (const prop of properties) {
    if (prop.items) {
      console.log(prop);
      if (prop.name === 'monitor_id') {
        const monitor_id = prop.items[1].value;
        // 0th item here is 'DUMMY'; I think this is the placeholder in their list for "Select a monitor"
        console.log(`Setting monitor_id to ${monitor_id}`);
        noobs.SetSourceSettings(sourceName, {
          ...settings1,
          monitor_id,
        });
      }
    } else {
      console.log(prop);
    }
  }

  const settings2 = noobs.GetSourceSettings(sourceName);
  console.log('Settings 2:', settings2);

  console.log('Adding source to scene...');
  noobs.AddSourceToScene(sourceName);

  // Start the recording, with 1s offset into the past.
  noobs.StartRecording(0);
  await new Promise((resolve) => setTimeout(resolve, 2000));

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
