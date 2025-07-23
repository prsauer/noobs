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

  ObsShowPreview(hwnd: Buffer): undefined;
  ObsHidePreview(): undefined;
  ObsSetPreviewSize(width: number, height: number): undefined;
}

declare const obs: WCREngine;
export = obs;
