const noobs = require('./index.js');
const path = require('path');

async function test() {
  console.log("Starting obs...");

  const cb = (msg) => {
    console.log("Callback received:", msg);
  }

  const pluginPath = path.resolve(__dirname, 'dist', 'plugins');
  const logPath = path.resolve(__dirname, 'logs');
  const dataPath = path.resolve(__dirname, 'dist', 'effects');
  const recordingPath = path.resolve(__dirname, 'recordings');
  
  console.log("Plugin path:", pluginPath);
  console.log("Log path:", logPath);
  console.log("Data path:", dataPath);
  console.log("Recording path:", recordingPath);

  noobs.ObsInit(pluginPath, logPath, dataPath, recordingPath, cb);
  await new Promise(resolve => setTimeout(resolve, 1000));

  for (let i = 0; i < 2; i++) {
    console.log("Test Recording Loop:", i + 1);
    noobs.ObsStartBuffer();
    await new Promise(resolve => setTimeout(resolve, 5000));

    noobs.ObsStartRecording(1);
    await new Promise(resolve => setTimeout(resolve, 5000));

    noobs.ObsStopRecording();
    await new Promise(resolve => setTimeout(resolve, 5000));

    const last = noobs.ObsGetLastRecording();
    console.log("Last recording:", last);

    await new Promise(resolve => setTimeout(resolve, 30000));
  }

  console.log("Stopping obs...");
  noobs.ObsShutdown();

  console.log("Test Done");
}

console.log("Starting test...");
test();
console.log("Test now running async");