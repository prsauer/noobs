import * as obs from './index';

// TypeScript will now provide intellisense and type checking
console.log('Testing typed addon...');

// This will be type-checked as number
const uptime: number = obs.getUptime();
console.log('System uptime:', uptime, 'milliseconds');

// If you have the hello function available, uncomment this:
// const message: string = obs.hello();
// console.log('Message:', message);

// TypeScript will catch type errors at compile time
// For example, this would cause a TypeScript error:
// const wrongType: string = obs.getUptime(); // Error: Type 'number' is not assignable to type 'string'

export {}; // Make this file a module
