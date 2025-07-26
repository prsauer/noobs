export type Signal = {
  id: string;   // Signal identifier, e.g. "stop"
  code: number; // 0 for success, other values for errors
}

export type SceneItemPosition = {
  x: number;      // X position in pixels
  y: number;      // Y position in pixels
  width: number;  // Width in pixels, before scaling
  height: number; // Height in pixels, before scaling
  scaleX: number; // X scaling factor
  scaleY: number; // Y scaling factor
};

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

  GetSourcePos(src: string): SceneItemPosition;
  SetSourcePos(src: string, pos: SceneItemPosition): void;

  InitPreview(hwnd: Buffer): void;
  ShowPreview(x: number, y: number, width: number, height: number): void;
  HidePreview(): void;
}

declare const noobs: Noobs;
export default noobs;
