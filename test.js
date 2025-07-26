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

  noobs.Init(pluginPath, logPath, dataPath, recordingPath, cb);
  await new Promise(resolve => setTimeout(resolve, 1000));
  noobs.UpdateSource(100, 100, 0.5);

  for (let i = 0; i < 2; i++) {
    console.log("Test Recording Loop:", i + 1);
    noobs.StartBuffer();
    await new Promise(resolve => setTimeout(resolve, 5000));

    noobs.StartRecording(1);
    await new Promise(resolve => setTimeout(resolve, 5000));

    noobs.StopRecording();
    await new Promise(resolve => setTimeout(resolve, 5000));

    const last = noobs.GetLastRecording();
    console.log("Last recording:", last);

    await new Promise(resolve => setTimeout(resolve, 1000));
  }

  console.log("Stopping obs...");
  noobs.Shutdown();

  console.log("Test Done");
}

console.log("Starting test...");
test();
console.log("Test now running async");