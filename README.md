# Noobs
>  No-(de)-obs

Node bindings to libobs, built for Warcraft Recorder. An alternative but less mature `obs-studio-node` with a focus on recording gameplay from a replay buffer.

Uses a [custom](https://github.com/aza547/warcraft-recorder-obs-studio) fork of libobs to enable "replay_buffer to recording" as well as some less interesting improvements to allow libobs to be easier used as a library and not through OBS studio itself.

## Installation

```bash
npm install noobs
```

## Usage

```javascript
import noobs from 'noobs';

...

noobs.ObsInit(pluginPath, logPath, dataPath, recordingPath, cb);
```

See `test.js` for more.

### TypeScript

This package includes TypeScript definitions.

## Requirements

TODO

- Node.js ??
- A C++ compiler (Visual Studio Build Tools on Windows, Xcode on macOS, GCC on Linux) ??
- Node-gyp ??

## Building from Source

```bash
npm install
npm run build
```

## License

GPL-2.0
