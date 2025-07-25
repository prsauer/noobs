# No(de)obs
> Previously named "warcraft-recorder-obs-engine"

This library is still very young, you probably don't want to use it.

Native node bindings to libobs, built for Warcraft Recorder. 

An alternative but less mature `obs-studio-node` with a focus on recording gameplay from a replay buffer.

Uses a [custom](https://github.com/aza547/warcraft-recorder-obs-studio) fork of libobs to:
- Enable "replay_buffer to recording".
- Allow libobs to be used as a library easier.

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
npm install       # install deps
npm run build     # compile it

npm version patch # version bump
npm pack          # build tgz locally
npm publish       # publish to npm
```

## License

GPL-2.0
