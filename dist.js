const fs = require('fs');
const path = require('path');

// Make the dist directory.
const distRoot = path.resolve(__dirname, 'dist');
const distBin = path.join(distRoot, 'bin', '64bit');
fs.mkdirSync(distBin, { recursive: true });

// Copy the compiled .node file.
const addonSrc = path.resolve(__dirname, 'build', 'Release', 'warcraft-recorder-obs-engine.node');
const addonDest = path.join(dist, 'warcraft-recorder-obs-engine.node');
fs.copyFileSync(addonSrc, addonDest);

// Now copy the .dll files we need.
const binDir = path.resolve(__dirname, 'bin', '64bit');

fs.readdirSync(binDir).filter(file => file.endsWith('.dll')).forEach((file) => {
  const srcPath = path.join(binDir, file);
  const destPath = path.join(distBin, file);
  fs.copyFileSync(srcPath, destPath);
});
