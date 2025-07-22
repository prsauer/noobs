const wcr = require('./index.js');

async function test() {
  console.log('Uptime:', wcr.getUptime());

  console.log("Listing processes...");
  
  try {
    const processes = await wcr.listProcesses();
    console.log(`Found ${processes.length} processes`);
    console.log('First few processes:', processes.slice(0, 5));
  } catch (error) {
    console.error('Error:', error);
  }


  console.log("Starting obs...");
  wcr.StartOBS();

  console.log("Test Done");
}

console.log("Starting test...");
test();
console.log("Test now running async");