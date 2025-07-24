const fs = require('fs');
const path = require('path');

// Make the dist directory.
const dist = path.resolve(__dirname, 'dist');
fs.mkdirSync(dist);

// Copy the compiled .node file.
const addonSrc = path.resolve(__dirname, 'build', 'Release', 'warcraft-recorder-obs-engine.node');
const addonDest = path.join(dist, 'warcraft-recorder-obs-engine.node');
fs.copyFileSync(addonSrc, addonDest);

// Now copy the .dll files we need.
const binDir = path.resolve(__dirname, 'bin', '64bit');

fs.readdirSync(binDir).filter(file => file.endsWith('.dll')).forEach((file) => {
  const srcPath = path.join(binDir, file);
  const destPath = path.join(dist, 'bin', '64bit', file);
  fs.mkdirSync(dist);
  fs.copyFileSync(srcPath, destPath);
});
