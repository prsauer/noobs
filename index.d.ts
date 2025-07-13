interface WCREngine {
  getUptime(): number;
}

declare const obs: WCREngine;
export = obs;
