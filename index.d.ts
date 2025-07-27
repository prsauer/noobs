// OBS Data Types
export type ObsDataValue =
  | string
  | number
  | boolean
  | ObsData
  | ObsData[]
  | null;

export interface ObsData {
  [key: string]: ObsDataValue;
}

// OBS Property Types
export type ObsPropertyType =
  | 'invalid'
  | 'bool'
  | 'int'
  | 'float'
  | 'text'
  | 'path'
  | 'list'
  | 'color'
  | 'button'
  | 'font'
  | 'editable_list'
  | 'frame_rate'
  | 'group'
  | 'color_alpha'
  | 'unknown';

export type ObsNumberType = 'scroller' | 'slider';

export type ObsTextType = 'default' | 'password' | 'multiline' | 'info';

export type ObsPathType = 'file' | 'file_save' | 'directory';

export type ObsComboType = 'invalid' | 'editable' | 'list' | 'radio';

export type ObsComboFormat = 'invalid' | 'int' | 'float' | 'string';

export interface ObsListItem {
  name: string;
  value: string | number;
  disabled: boolean;
}

export interface ObsPropertyBase {
  name: string;
  description: string;
  type: ObsPropertyType;
  enabled: boolean;
  visible: boolean;
}

export interface ObsIntProperty extends ObsPropertyBase {
  type: 'int';
  min: number;
  max: number;
  step: number;
  number_type: ObsNumberType;
}

export interface ObsFloatProperty extends ObsPropertyBase {
  type: 'float';
  min: number;
  max: number;
  step: number;
  number_type: ObsNumberType;
}

export interface ObsTextProperty extends ObsPropertyBase {
  type: 'text';
  text_type: ObsTextType;
}

export interface ObsPathProperty extends ObsPropertyBase {
  type: 'path';
  path_type: ObsPathType;
  filter: string;
  default_path: string;
}

export interface ObsListProperty extends ObsPropertyBase {
  type: 'list';
  combo_type: ObsComboType;
  combo_format: ObsComboFormat;
  items: ObsListItem[];
}

export interface ObsBoolProperty extends ObsPropertyBase {
  type: 'bool';
}

export interface ObsColorProperty extends ObsPropertyBase {
  type: 'color' | 'color_alpha';
}

export interface ObsButtonProperty extends ObsPropertyBase {
  type: 'button';
}

export interface ObsFontProperty extends ObsPropertyBase {
  type: 'font';
}

export interface ObsEditableListProperty extends ObsPropertyBase {
  type: 'editable_list';
}

export interface ObsFrameRateProperty extends ObsPropertyBase {
  type: 'frame_rate';
}

export interface ObsGroupProperty extends ObsPropertyBase {
  type: 'group';
}

export interface ObsGenericProperty extends ObsPropertyBase {
  type: 'invalid' | 'unknown';
}

export type ObsProperty =
  | ObsIntProperty
  | ObsFloatProperty
  | ObsTextProperty
  | ObsPathProperty
  | ObsListProperty
  | ObsBoolProperty
  | ObsColorProperty
  | ObsButtonProperty
  | ObsFontProperty
  | ObsEditableListProperty
  | ObsFrameRateProperty
  | ObsGroupProperty
  | ObsGenericProperty;

export type Signal = {
  id: string; // Signal identifier, e.g. "stop"
  code: number; // 0 for success, other values for errors
};

export type SceneItemPosition = {
  x: number; // X position in pixels
  y: number; // Y position in pixels
  scaleX: number; // X scaling factor
  scaleY: number; // Y scaling factor
};

export type SourceDimensions = {
  height: number; // Height in pixels, before scaling
  width: number; // Width in pixels, before scaling
};

interface Noobs {
  Init(
    /** Required configuration */
    configuration: {
      /**
       * The path to the directory where plugins will be loaded from.
       */
      pluginPath: string;
      /**
       * The path to the directory where OBS logs will be saved.
       */
      logPath: string;
      /**
       The path to the directory where OBS will load effects from.
       */
      effectsPath: string;
      /**
       * The path to the directory where recordings will be saved.
       */
      recordingPath: string;
      /**
       * The callback function to be called when a signal is received.
       */
      cb: (signal: Signal) => void;
    },
    /** Optional params / settings */
    options?: {
      /** If true, noobs will use an in-memory video buffer to make sure we don't miss the start of video recordings. StartRecording must still be called.
       * If false, noobs will not buffer and simplyh start recording when startRecording is called.
       *
       * Default: *true*
       */
      buffering?: boolean;
    }
  ): void;

  Shutdown(): void;

  // Recording functions
  StartBuffer(): void;
  StartRecording(offset: number): void;
  StopRecording(): void;
  GetLastRecording(): string;
  SetRecordingDir(recordingPath: string): void;

  // Source management functions
  CreateSource(name: string, type: string): void;
  DeleteSource(name: string): void;
  GetSourceSettings(name: string): ObsData;
  SetSourceSettings(name: string, settings: ObsData): void;
  GetSourceProperties(name: string): ObsProperty[];

  // Scene management functions
  AddSourceToScene(sourceName: string): void;
  RemoveSourceFromScene(sourceName: string): void;
  GetSourcePos(name: string): SceneItemPosition & SourceDimensions;
  SetSourcePos(name: string, pos: SceneItemPosition): void;
  // TODO: Cropping?

  // Preview functions
  InitPreview(hwnd: Buffer): void;
  ShowPreview(x: number, y: number, width: number, height: number): void;
  HidePreview(): void;
}

declare const noobs: Noobs;
export default noobs;
