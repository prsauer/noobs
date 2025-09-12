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

  noobs.CreateSource('Test Source', 'monitor_capture');
  const settings1 = noobs.GetSourceSettings('Test Source');
  const p = noobs.GetSourceProperties('Test Source');
  // We need method 2 here which is WGC. We can't use 0 (Auto) or 1 (DXGI) unless we have a graphics context.
  // Or atlaest that's what ChatGPT thinks. This is only an issue when running as a CLI job. If we run in the context of an
  // electron process it just works fine.
  noobs.SetSourceSettings('Test Source', { ...settings1, monitor_id: p[1].items[1].value, method: 2 });
  noobs.AddSourceToScene('Test Source');

  // Now do the reset. Output file should be 30fps and 3000x2000.
  console.log('Reconfigure to 30ps 3000x2000');
  noobs.ResetVideoContext(30, 3000, 2000);
  
  noobs.StartRecording(0);
  await new Promise((resolve) => setTimeout(resolve, 2000));
  noobs.StopRecording();
  await new Promise((resolve) => setTimeout(resolve, 2000));
  
  // Now do the reset. Output file should be 45fps and 500x300.
  console.log('Reconfigure to 45ps 500x300');
  noobs.ResetVideoContext(45, 500, 300);
  
  noobs.StartRecording(0);
  await new Promise((resolve) => setTimeout(resolve, 2000));
  noobs.StopRecording();
  await new Promise((resolve) => setTimeout(resolve, 2000));
  
  noobs.Shutdown();
  console.log('Test Done');
}

console.log('Starting test...');
test();
console.log('Test now running async');
