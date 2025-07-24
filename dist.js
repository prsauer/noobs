const fs = require('fs');
const path = require('path');

const distRoot = path.resolve(__dirname, 'dist');
const distBin = path.join(distRoot, 'bin', '64bit');

// Clean the dist directory if it exists.
if (fs.existsSync(distRoot)) {
  fs.rmdirSync(distRoot, { recursive: true, force: true });
}

// Remake the dist directory structure.
fs.mkdirSync(distBin, { recursive: true });

// Copy the compiled .node file.
const addonSrc = path.resolve(__dirname, 'build', 'Release', 'warcraft-recorder-obs-engine.node');
const addonDest = path.join(distRoot, 'warcraft-recorder-obs-engine.node');
fs.copyFileSync(addonSrc, addonDest);

// Now copy the .dll files we need.
const binDir = path.resolve(__dirname, 'bin', '64bit');

fs.readdirSync(binDir).filter(file => file.endsWith('.dll')).forEach((file) => {
  const srcPath = path.join(binDir, file);
  const destPath = path.join(distBin, file);
  fs.copyFileSync(srcPath, destPath);
});
