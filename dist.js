const fs = require('fs');
const path = require('path');

const distRoot = path.resolve(__dirname, 'dist');
const distBin = path.join(distRoot, 'bin');
const distPlugins = path.join(distRoot, 'plugins');
const distEffects = path.join(distRoot, 'effects');

// Clean the dist directory if it exists.
if (fs.existsSync(distRoot)) {
  fs.rmSync(distRoot, { recursive: true, force: true });
}

// Remake the dist directory structure.
fs.mkdirSync(distRoot);
fs.mkdirSync(distBin);
fs.mkdirSync(distPlugins);
fs.mkdirSync(distEffects);

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

// Copy the plugins we use.
const pluginDir = path.resolve(__dirname, 'bin', 'plugins');

const plugins = ["obs-x264.dll", "obs-ffmpeg.dll", "win-capture.dll"];

plugins.forEach((file) => {
  const srcPath = path.join(pluginDir, file);
  const destPath = path.join(distPlugins, file);
  fs.copyFileSync(srcPath, destPath);
});

// Copy the effects OBS requires.
const effectsDir = path.resolve(__dirname, 'bin', 'effects');

fs.readdirSync(effectsDir).forEach((file) => {
  const srcPath = path.join(effectsDir, file);
  const destPath = path.join(distEffects, file);
  fs.copyFileSync(srcPath, destPath);
});

// Copy executable files required on the PATH.
const exeFiles = ['obs-amf-test.exe'];

exeFiles.forEach((file) => {
  const srcPath = path.join(binDir, file);
  const destPath = path.join(distBin, file);
  fs.copyFileSync(srcPath, destPath);
});

