const noobs = require('../index.js');

async function test() {
  console.time('TestProcs');
  const procs = noobs.TestProcs();
  console.timeEnd('TestProcs');
  console.log(`${procs.length} processes found, first: ${procs[0]}`);
  console.log('Test Done');
}

console.log('Starting test...');
test();
console.log('Test now running async');
