# Warcraft Recorder Engine

A native Node.js addon for Warcraft gameplay recording built with N-API.

## Installation

```bash
npm install warcraft-recorder-engine
```

## Usage

```javascript
const warcraftRecorder = require('warcraft-recorder-engine');

// Get a greeting from the native addon
const message = warcraftRecorder.hello();
console.log(message); // "hello from napi"
```

### TypeScript

This package includes TypeScript definitions:

```typescript
import * as warcraftRecorder from 'warcraft-recorder-engine';

const message: string = warcraftRecorder.hello();
console.log(message);
```

## Requirements

- Node.js >= 14.0.0
- A C++ compiler (Visual Studio Build Tools on Windows, Xcode on macOS, GCC on Linux)

## Building from Source

```bash
npm install
npm run build
```

## API

### `hello(): string`

Returns a greeting message from the native addon.

## License

ISC
