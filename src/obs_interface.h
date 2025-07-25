#pragma once

#include <napi.h>

struct SignalData {
  std::string id;
  long long code;
};

class ObsInterface {
  public:
    ObsInterface(
      const std::string& pluginPath,    // where to look for plugins
      const std::string& logPath,       // where to write logs to
      const std::string& dataPath,      // where to look for effects
      const std::string& recordingPath, // where to save recordings
      Napi::ThreadSafeFunction cb       // JavaScript callback
    );

    ~ObsInterface();

    void startBuffering();
    void startRecording(int offset);
    void stopRecording();
    std::string getLastRecording();

    void showPreview(HWND hwnd, int x, int y, int width, int height); // Also used for moving and resizing.
    void hidePreview();

    std::vector<std::string> get_available_video_encoders();

    // TODO
    // Show preview
    // Hide preview
    // Configure video 
    // Configure audio
    // List audio source
    // Reconfigure audio sources
    // Reconfigure video sources
  
  private:
    obs_output_t *output = nullptr;
    obs_scene_t *scene = nullptr;
    obs_source_t *video_source = nullptr;
    obs_source_t *audio_source = nullptr;
    obs_encoder_t *video_encoder = nullptr;
    obs_encoder_t *audio_encoder = nullptr;
    obs_display_t *display = nullptr;
    HWND previewHwnd = nullptr; // window handle for scene preview
    Napi::ThreadSafeFunction jscb; // javascript callback

    void init_obs(const std::string& pluginPath, const std::string& dataPath);
    void reset_video();
    void reset_audio();
    void load_module(const char* module);
    void create_signal_handlers(obs_output_t *output);

    static void output_signal_handler_starting(void *data, calldata_t *cd);
    static void output_signal_handler_start(void *data, calldata_t *cd);
    static void output_signal_handler_stop(void *data, calldata_t *cd);
    static void output_signal_handler_stopping(void *data, calldata_t *cd);
    static void output_signal_handler_saved(void *data, calldata_t *cd);

    void list_encoders(obs_encoder_type type = OBS_ENCODER_VIDEO);
    void list_source_types();
    void list_input_types();
    void list_output_types();

    obs_output_t* create_output(const std::string& recordingPath);
    obs_scene_t* create_scene();
    obs_source_t* create_video_source();
};
