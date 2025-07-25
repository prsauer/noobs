interface ProcessInfo {
  name: string;
  pid: number;
  parentPid: number;
  threads: number;
}

type Signal = { // TODO export type? 
  id: string;
  code: number;
}

interface WCREngine {
  getUptime(): number;
  listProcesses(): Promise<ProcessInfo[]>;

  ObsInit(pluginPath: string, logPath: string, cb: (signal: Signal) => void): void;
  ObsShutdown(): void;
  ObsStartBuffer(): void;
  ObsStartRecording(offset: number): void;
  ObsStopRecording(): void;
  ObsGetLastRecording(): string;

  ObsShowPreview(hwnd: Buffer): void;
  ObsHidePreview(): void;
  ObsResizePreview(width: number, height: number): void;
  ObsMovePreview(x: number, y: number): void;
}

declare const wcre: WCREngine;
export = wcre;
