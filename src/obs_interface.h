#pragma once

#include <napi.h>

struct SignalData {
  std::string id;
  long long code;
};

class ObsInterface {
  public:
    ObsInterface(
      const std::string& pluginPath,    // Where to look for plugins
      const std::string& logPath,       // Where to write logs to
      const std::string& dataPath,      // Where to look for effects
      const std::string& recordingPath, // Where to save recordings
      Napi::ThreadSafeFunction cb       // JavaScript callback
    );

    ~ObsInterface();

    bool setBuffering(bool buffering); // In buffering mode, the recording is stored in memory and can be converted to a file later.
    void startBuffering();             // Start buffering to memory.
    void startRecording(int offset);   // Convert the active buffered recording to a real one.
    void stopRecording();              // Stop the recording.
    std::string getLastRecording();    // Get the last recorded file path.
    void setRecordingDir(const std::string& recordingPath); // Output must not be active when calling this.

    void createSource(std::string name, std::string type);          // Create a new source
    void deleteSource(std::string name);                            // Release a source.
    obs_data_t* getSourceSettings(std::string name);                // Get the current settings.
    void setSourceSettings(std::string name, obs_data_t* settings); // Set settings.
    obs_properties_t* getSourceProperties(std::string name);        // Get the settings schema.

    void addSourceToScene(std::string name); // Add source to scene.
    void removeSourceFromScene(std::string name); // Remove source from scene.
    void getSourcePos(std::string name, vec2* pos, vec2* size, vec2* scale); // Size is returned to allow clients to calculate scale.
    void setSourcePos(std::string name, vec2* pos, vec2* scale); // Size does not get set here because it's set by the source itself.

    void initPreview(HWND parent); // Must call this before showPreview to setup resources.
    void showPreview(int x, int y, int width, int height); // Also used for moving and resizing.
    void hidePreview(); // Hide the preview display.

    std::vector<std::string> get_available_video_encoders();

    // TODO
    // Configure video 
    // Configure audio
    // List audio source
    // Reconfigure audio sources
    // Reconfigure video sources
  
  private:
    obs_output_t *file_output = nullptr;
    obs_output_t *buffer_output = nullptr;
    obs_scene_t *scene = nullptr;
    obs_source_t *video_source = nullptr;
    obs_source_t *audio_source = nullptr;
    obs_encoder_t *file_video_encoder = nullptr;
    obs_encoder_t *file_audio_encoder = nullptr;
    obs_encoder_t *buffer_video_encoder = nullptr;
    obs_encoder_t *buffer_audio_encoder = nullptr;
    obs_display_t *display = nullptr;
    HWND preview_hwnd = nullptr; // window handle for scene preview
    Napi::ThreadSafeFunction jscb; // javascript callback
    std::string recording_path = ""; 
    bool buffering = false; // Whether we are buffering the recording in memory.

    void init_obs(const std::string& pluginPath, const std::string& dataPath);
    void reset_video();
    void reset_audio();
    void load_module(const char* module);
    void connect_signal_handlers(obs_output_t *output);
    void disconnect_signal_handlers(obs_output_t *output);

    static void output_signal_handler_starting(void *data, calldata_t *cd);
    static void output_signal_handler_start(void *data, calldata_t *cd);
    static void output_signal_handler_stop(void *data, calldata_t *cd);
    static void output_signal_handler_stopping(void *data, calldata_t *cd);
    static void output_signal_handler_saved(void *data, calldata_t *cd);

    void list_encoders(obs_encoder_type type = OBS_ENCODER_VIDEO);
    void list_source_types();
    void list_input_types();
    void list_output_types();

    void create_scene();
    void create_output();

    void configure_video_encoder();
    void configure_audio_encoder();
};
