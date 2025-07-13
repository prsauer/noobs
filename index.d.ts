interface WarcraftRecorderEngine {
  getUptime(): number;
}

declare const addon: WarcraftRecorderEngine;
export = addon;
