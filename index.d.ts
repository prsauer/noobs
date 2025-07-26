type Signal = { // TODO export type? 
  id: string;
  code: number;
}

interface Noobs {
  Init(
    pluginPath: string, 
    logPath: string, 
    dataPath: string,
    recordingPath: string,
    cb: (signal: Signal) => void
  ): void;

  Shutdown(): void;
  StartBuffer(): void;
  StartRecording(offset: number): void;
  StopRecording(): void;
  GetLastRecording(): string;

  UpdateSource(x: number, y: number, scale: number): void;

  InitPreview(hwnd: Buffer): void;
  ShowPreview(x: number, y: number, width: number, height: number): void;
  HidePreview(): void;
}

declare const noobs: Noobs;
export = noobs;
