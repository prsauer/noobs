interface ProcessInfo {
  name: string;
  pid: number;
  parentPid: number;
  threads: number;
}

interface WCREngine {
  getUptime(): number;
  listProcesses(): Promise<ProcessInfo[]>;
}

declare const obs: WCREngine;
export = obs;
