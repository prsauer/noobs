#pragma once

#include <obs.h>
#include <napi.h>
#include <windows.h>
#include <map>
#include <string>
#include <optional>

#define AUDIO_INPUT "wasapi_input_capture"
#define AUDIO_OUTPUT "wasapi_output_capture"
#define AUDIO_PROCESS "wasapi_process_output_capture"

class ObsInterface;

struct SignalData {
  std::string type;
  std::string id;
  long long code;
  std::optional<float> value;
};

struct SignalContext {
  ObsInterface* self;
  std::string id;
};

struct PreviewInfo {
  uint32_t canvasWidth, canvasHeight;
  uint32_t displayWidth, displayHeight;
};

struct SourceSize {
  uint32_t width;
  uint32_t height;
};

class ObsInterface {
  public:
    ObsInterface(
      const std::string& distPath,      // Where to look for plugins and data
      const std::string& logPath,       // Where to write logs to
      const std::string& recordingPath, // Where to save recordings
      Napi::ThreadSafeFunction cb       // JavaScript callback
    );

    ~ObsInterface();

    bool setBuffering(bool buffering); // In buffering mode, the recording is stored in memory and can be converted to a file later.
    void startBuffering(); // Start buffering to memory.
    void startRecording(int offset); // Convert the active buffered recording to a real one.
    void stopRecording(); // Stop the recording.
    void forceStopRecording(); // Force stop the recording, this will not save the current recording.
    std::string getLastRecording(); // Get the last recorded file path.
    void setRecordingDir(const std::string& recordingPath); // Output must not be active when calling this.
    void setVideoContext(int fps, int width, int height); // Reset video settings.

    std::string createSource(std::string name, std::string type); // Create a new source, returns the name of the source which can vary from the requested.
    void deleteSource(std::string name); // Release a source.
    obs_data_t* getSourceSettings(std::string name); // Get the current settings.
    void setSourceSettings(std::string name, obs_data_t* settings); // Set settings.
    obs_properties_t* getSourceProperties(std::string name); // Get the settings schema.
    void setMuteAudioInputs(bool mute); // Mute or unmute all audio inputs.
    void setSourceVolume(std::string name, float volume); // Set the volume of an audio source.
    void setVolmeterEnabled(bool enabled); // Enable volmeters.

    void addSourceToScene(std::string name); // Add source to scene.
    void removeSourceFromScene(std::string name); // Remove source from scene.
    void getSourcePos(std::string name, vec2* pos, vec2* size, vec2* scale); // Size is returned to allow clients to calculate scale.
    void setSourcePos(std::string name, vec2* pos, vec2* scale); // Size does not get set here because it's set by the source itself.

    void initPreview(HWND parent); // Must call this before showPreview to setup resources.
    void configurePreview(int x, int y, int width, int height); // Move and resize the preview display.
    void showPreview(); // Show the preview display.
    void hidePreview(); // Hide the preview display, but leave it running.
    void disablePreview(); // Disable the preview display, to save resources.
    PreviewInfo getPreviewInfo(); // Get the dimensions of the display, and the base canvas.
    void setDrawSourceOutline(bool enabled); // Red box around source
    bool getDrawSourceOutlineEnabled();

    std::vector<std::string> listAvailableVideoEncoders(); // Return a list of available video encoders.
    void setVideoEncoder(std::string id, obs_data_t* settings); // Set the video encoder to use.

    std::map<std::string, obs_source_t*> sources; // Map of source names to obs_source_t pointers. 
    std::map<std::string, SourceSize> sizes; // Map of source names to their last known size, used for firing callbacks on size changes. 
    std::map<std::string, obs_volmeter_t*> volmeters; // Map of source names to obs_volmeter_t pointers.
    void sourceCallback(std::string name); // Send callback for source change.
    void zeroVolmeter(std::string name); // Zero the volmeter for a source.

  private:
    obs_output_t *file_output = nullptr;
    obs_output_t *buffer_output = nullptr;

    obs_scene_t *scene = nullptr;

    obs_encoder_t *file_video_encoder = nullptr;
    obs_encoder_t *file_audio_encoder = nullptr;

    obs_encoder_t *buffer_video_encoder = nullptr;
    obs_encoder_t *buffer_audio_encoder = nullptr;
    
    obs_display_t *display = nullptr;
    HWND preview_hwnd = nullptr; // window handle for scene preview
    Napi::ThreadSafeFunction jscb; // javascript callback
    std::string recording_path = ""; 
    std::string unbuffered_output_filename = "";

    bool buffering = false; // Whether we are buffering the recording in memory.
    bool drawSourceOutline = false; // Draw red outline around source
    void init_obs(const std::string& distPath);
    int reset_video(int fps, int width, int height);
    bool reset_audio();
    void load_module(const char* module, const char* data, bool allowFail); // Load a module, data is optional.
    void connect_signal_handlers(obs_output_t *output);
    void disconnect_signal_handlers(obs_output_t *output);

    SignalContext* starting_ctx;
    SignalContext* start_ctx;
    SignalContext* stopping_ctx;
    SignalContext* stop_ctx;
    static void output_signal_handler(void *data, calldata_t *cd);

    void list_encoders(obs_encoder_type type = OBS_ENCODER_VIDEO);
    void list_source_types();
    void list_input_types();
    void list_output_types();

    void create_scene();
    void create_output();

    std::string video_encoder_id; // The video encoder ID to use.
    obs_data_t* video_encoder_settings; // Settings for the video encoder.
    void create_video_encoders();
    void create_audio_encoders();

    bool volmeter_enabled = false; // Whether the volmeter callback is enabled.

    static void volmeter_callback(
      void *data, 
      const float magnitude[MAX_AUDIO_CHANNELS],
      const float peak[MAX_AUDIO_CHANNELS], 
      const float inputPeak[MAX_AUDIO_CHANNELS]
    );
};
