const wcre = require('./index.js');

async function test() {
  console.log('Uptime:', wcre.getUptime());

  console.log("Listing processes...");
  
  try {
    const processes = await wcre.listProcesses();
    console.log(`Found ${processes.length} processes`);
    console.log('First few processes:', processes.slice(0, 5));
  } catch (error) {
    console.error('Error:', error);
  }


  console.log("Starting obs...");

  const cb = (msg) => {
    console.log("Callback received:", msg);
  }

  wcre.ObsInit(cb);
  await new Promise(resolve => setTimeout(resolve, 1000));

  for (let i = 0; i < 2; i++) {
    console.log("Test Recording Loop:", i + 1);
    wcre.ObsStartBuffer();
    await new Promise(resolve => setTimeout(resolve, 5000));

    wcre.ObsStartRecording(1);
    await new Promise(resolve => setTimeout(resolve, 5000));

    wcre.ObsStopRecording();
    await new Promise(resolve => setTimeout(resolve, 5000));

    const last = wcre.ObsGetLastRecording();
    console.log("Last recording:", last);

    await new Promise(resolve => setTimeout(resolve, 30000));
  }

  console.log("Stopping obs...");
  wcre.ObsShutdown();

  console.log("Test Done");
}

console.log("Starting test...");
test();
console.log("Test now running async");