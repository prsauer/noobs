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

interface Noobs {
  getUptime(): number;
  listProcesses(): Promise<ProcessInfo[]>;

  ObsInit(
    pluginPath: string, 
    logPath: string, 
    dataPath: string,
    recordingPath: string,
    cb: (signal: Signal) => void
  ): void;

  ObsShutdown(): void;
  ObsStartBuffer(): void;
  ObsStartRecording(offset: number): void;
  ObsStopRecording(): void;
  ObsGetLastRecording(): string;

  ObsShowPreview(hwnd: Buffer, x: number, y: number, width: number, height: number): void;
  ObsHidePreview(): void;
  ObsResizePreview(width: number, height: number): void;
  ObsMovePreview(x: number, y: number): void;
}

declare const noobs: Noobs;
export = noobs;
