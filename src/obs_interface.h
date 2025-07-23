#pragma once

class ObsInterface {
  public:
    ObsInterface();
    ~ObsInterface();

    void startBuffering();
    void startRecording(int offset);
    void stopRecording();

    void showPreview(HWND hwnd);
    void hidePreview();
    void resizePreview(int width, int height);
    void movePreview(int x, int y);

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
    HWND previewHwnd = nullptr;

    void init_obs();
    void reset_video();
    void reset_audio();
    void load_module(const char* module);
    void create_signal_handlers(obs_output_t *output);
    static void output_signal_handler(void *data, calldata_t *cd);
    static void obs_log_handler(int lvl, const char *msg, va_list args, void *p);

    void list_encoders(obs_encoder_type type = OBS_ENCODER_VIDEO);
    void list_source_types();
    void list_input_types();
    void list_output_types();

    obs_output_t* create_output();
    obs_scene_t* create_scene();
    obs_source_t* create_video_source();
};
