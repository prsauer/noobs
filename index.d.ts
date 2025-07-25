type Signal = { // TODO export type? 
  id: string;
  code: number;
}

interface Noobs {
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

  ObsInitPreview(hwnd: Buffer): void;
  ObsShowPreview(x: number, y: number, width: number, height: number): void;
  ObsHidePreview(): void;
}

declare const noobs: Noobs;
export = noobs;
