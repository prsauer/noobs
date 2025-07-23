interface ProcessInfo {
  name: string;
  pid: number;
  parentPid: number;
  threads: number;
}

interface WCREngine {
  getUptime(): number;
  listProcesses(): Promise<ProcessInfo[]>;

  ObsInit(): void;
  ObsShutdown(): void;
  ObsStartBuffer(): void;
  ObsStartRecording(): void;
  ObsStopRecording(): void;

  ObsShowPreview(hwnd: Buffer): void;
  ObsHidePreview(): void;
  ObsResizePreview(width: number, height: number): void;
  ObsMovePreview(x: number, y: number): void;
}

declare const wcre: WCREngine;
export = wcre;
