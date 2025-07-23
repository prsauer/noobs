interface ProcessInfo {
  name: string;
  pid: number;
  parentPid: number;
  threads: number;
}

interface WCREngine {
  getUptime(): number;
  listProcesses(): Promise<ProcessInfo[]>;

  ObsInit(): undefined;
  ObsShutdown(): undefined;
  ObsStartBuffer(): undefined;
  ObsStartRecording(): undefined;
  ObsStopRecording(): undefined;
}

declare const obs: WCREngine;
export = obs;
