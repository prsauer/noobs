#include <iostream>
#include <windows.h>
#include <obs.h>
#include "utils.h"
#include <chrono>
#include "obs_interface.h"
#include <vector>
#include <thread>

std::vector<std::string> ObsInterface::get_available_video_encoders()
{
  std::vector<std::string> encoders;
  size_t idx = 0;
  const char *encoder_type;

  while (obs_enum_encoder_types(idx++, &encoder_type)) {
    bool video = obs_get_encoder_type(encoder_type) == OBS_ENCODER_VIDEO;

    if (video)
      encoders.emplace_back(encoder_type);
  }

  return encoders;
}

void ObsInterface::list_encoders(obs_encoder_type type)
{
  std::cout << "List encoders" << std::endl;
  size_t idx = 0;
  const char *encoder_type;

  while (obs_enum_encoder_types(idx++, &encoder_type)) {
    if (obs_get_encoder_type(encoder_type) != type) {
      continue;
    }

    // if (obs_get_encoder_caps(encoder_type) & hide_flags) {
    //   continue;
    // }

    blog(LOG_INFO, "\t- %s (%s)", encoder_type, obs_encoder_get_display_name(encoder_type));
  }
};

void ObsInterface::list_source_types()
{
  std::cout << "List  src types" << std::endl;
  size_t idx = 0;
  const char *src = nullptr;

  while (obs_enum_source_types(idx++, &src)) {
    blog(LOG_INFO, "\t- %s", src);
  }
}

void ObsInterface::list_input_types()
{
  std::cout << "List  input types" << std::endl;
  size_t idx = 0;
  const char *src = nullptr;

  while (obs_enum_input_types(idx++, &src)) {
    blog(LOG_INFO, "\t- %s", src);
  }
}

void ObsInterface::list_output_types()
{
  std::cout << "List  output types" << std::endl;
  size_t idx = 0;
  const char *src = nullptr;

  while (obs_enum_output_types(idx++, &src)) {
    blog(LOG_INFO, "\t- %s", src);
  }
}

void ObsInterface::load_module(const char* module) {
  std::cout << "Loading module: "  << module << std::endl;

  obs_module_t *ptr = NULL;
  int success = obs_open_module(&ptr, module, NULL);

  if (success != MODULE_SUCCESS)
    throw std::runtime_error("Failed to open module!");

  bool initmod = obs_init_module(ptr);

  if (!initmod)
    throw std::runtime_error("Module initialization failed!");
}

void ObsInterface::reset_video() {
  std::cout << "Setup video info" << std::endl;

  obs_video_info ovi = {};

  ovi.base_width = 1920;
  ovi.base_height = 1080;
  ovi.output_width = 1920;
  ovi.output_height = 1080;
  ovi.fps_num = 60;
  ovi.fps_den = 1;

  ovi.output_format = VIDEO_FORMAT_NV12;
  ovi.colorspace = VIDEO_CS_DEFAULT;
  ovi.range = VIDEO_RANGE_DEFAULT;
  ovi.scale_type = OBS_SCALE_BILINEAR;
  ovi.adapter = 0;
  ovi.gpu_conversion = true;
  ovi.graphics_module = "libobs-d3d11.dll"; 

  int success = obs_reset_video(&ovi);

  if (success != OBS_VIDEO_SUCCESS)
    throw std::runtime_error("Failed to reset video!");
}

void ObsInterface::reset_audio() {
  struct obs_audio_info oai = {0};
  oai.samples_per_sec = 48000;
  oai.speakers = SPEAKERS_STEREO;
  bool reset = obs_reset_audio(&oai);

  if (!reset)
    throw std::runtime_error("Failed to reset audio!");
}

void ObsInterface::init_obs() {
  set_cwd("D:/checkouts/warcraft-recorder-obs-engine");
  print_cwd();
  print_exe_path();

  std::cout << "Starting..." << std::endl;
  auto success = obs_startup("en-US", NULL, NULL);
  std::cout << "OBS has started!" << std::endl;

  if (!success)
    throw std::runtime_error("OBS startup failed");

  if (!obs_initialized())
    throw std::runtime_error("OBS initialization failed");

  obs_add_data_path("D:/checkouts/warcraft-recorder-obs-engine/effects/");
  obs_add_data_path("D:/checkouts/warcraft-recorder-obs-engine/effects/libobs");

  load_module("D:/checkouts/warcraft-recorder-obs-engine/obs-plugins/64bit/obs-x264.dll");
  load_module("D:/checkouts/warcraft-recorder-obs-engine/obs-plugins/64bit/obs-ffmpeg.dll");
  load_module("D:/checkouts/warcraft-recorder-obs-engine/obs-plugins/64bit/win-capture.dll");

  const char* version = obs_get_version_string();
  std::cout << "OBS version is: " << version <<  std::endl;

  list_encoders();
  list_source_types();
  list_input_types();
  list_output_types();
  reset_video();
  reset_audio();

  // TODO setup logs?
  std::cout << "Done initializing" << std::endl;
}

obs_output_t* ObsInterface::create_output() {
  std::cout << "Create output" << std::endl;
  obs_output_t *output = obs_output_create("replay_buffer", "recording_output", NULL, NULL);

  if (!output)
    throw std::runtime_error("Failed to create output!");

  std::cout << "Set output settings" << std::endl;
  obs_data_t *settings = obs_data_create();
  obs_data_set_int(settings, "max_time_sec", 30);
  obs_data_set_int(settings, "max_size_mb", 512);
  obs_data_set_string(settings, "directory", "D:/checkouts/warcraft-recorder-obs-engine"); // or wherever
  obs_data_set_string(settings, "format", "%CCYY-%MM-%DD %hh-%mm-%ss");
  obs_data_set_string(settings, "extension", "mp4");
  obs_output_update(output, settings);
  obs_data_release(settings);

  std::cout << "Create venc" << std::endl;
  video_encoder = obs_video_encoder_create("h264_texture_amf", "simple_h264_stream", NULL, NULL);

  if (!video_encoder)
    throw std::runtime_error("Failed to create video encoder!");

  std::cout << "Set video encoder settings" << std::endl;
  obs_data_t* amf_settings = obs_data_create();
  obs_data_set_string(amf_settings, "preset", "speed");  // Faster preset
  //obs_data_set_int(amf_settings, "bitrate", 2500);
  obs_data_set_string(amf_settings, "rate_control", "CQP");
  obs_data_set_int(amf_settings, "cqp", 30);
  obs_data_set_string(amf_settings, "profile", "main");
  obs_data_set_int(amf_settings, "keyint_sec", 1); // Set keyframe interval to 1 second
  obs_encoder_update(video_encoder, amf_settings);
  obs_data_release(amf_settings);

  std::cout << "Create aenc" << std::endl;
  audio_encoder = obs_audio_encoder_create("ffmpeg_aac", "simple_aac", NULL, 0, NULL);

  if (!audio_encoder)
    throw std::runtime_error("Failed to create audio encoder!");

  std::cout << "Set audio encoder settings" << std::endl;
  obs_data_t *aenc_settings = obs_data_create();
  obs_data_set_int(aenc_settings, "bitrate", 128);
  obs_encoder_update(audio_encoder, aenc_settings);
  obs_data_release(aenc_settings);

  obs_output_set_video_encoder(output, video_encoder);
  obs_output_set_audio_encoder(output, audio_encoder, 0);

  obs_encoder_set_video(video_encoder, obs_get_video());
  obs_encoder_set_audio(audio_encoder, obs_get_audio());

  return output;
}

obs_scene_t* ObsInterface::create_scene() {
  std::cout << "Create scene and src" << std::endl;
  obs_scene_t *scene = obs_scene_create("WCR Scene");

  if (!scene)
    throw std::runtime_error("Failed to create scene!");

  obs_source_t *scene_source = obs_scene_get_source(scene);

  if (!scene_source)
    throw std::runtime_error("Failed to get scene src!");

  obs_set_output_source(0, scene_source);  // 0 = video track
  return scene;
}

obs_source_t* ObsInterface::create_video_source() {
  std::cout << "Create display capture source" << std::endl;
  // Create settings for monitor capture
  obs_data_t *monitor_settings = obs_data_create();
  obs_data_set_int(monitor_settings, "monitor", 0);  // Monitor 0
  obs_data_set_bool(monitor_settings, "capture_cursor", true);

  obs_source_t *source = obs_source_create("monitor_capture", "Monitor", monitor_settings, NULL);
  obs_data_release(monitor_settings);

  if (!source)
    throw std::runtime_error("Failed to create video source!");

  return source;
}

void ObsInterface::output_signal_handler(void *data, calldata_t *cd) {
  std::cout << "\n=== OUTPUT SIGNAL ===" << std::endl;
  
  // Try to get common signal parameters
  obs_output_t *output = (obs_output_t*)calldata_ptr(cd, "output");
  long long code = calldata_int(cd, "code");

  auto now = std::chrono::system_clock::now();
  std::time_t time = std::chrono::system_clock::to_time_t(now);
  
  std::cout << "Signal: " << (const char *)data << std::endl;
  std::cout << "Code: " << code << std::endl;
  std::cout << "Time: " << std::ctime(&time) << std::endl;
  
  std::cout << "===================\n" << std::endl;
}

void ObsInterface::create_signal_handlers(obs_output_t *output) {
  signal_handler_t *sh = obs_output_get_signal_handler(output);
  signal_handler_connect(sh, "starting", output_signal_handler,  (void *)"starting");
  signal_handler_connect(sh, "start", output_signal_handler,  (void *)"start");
  signal_handler_connect(sh, "stopping", output_signal_handler,  (void *)"stopping");
  signal_handler_connect(sh, "stop", output_signal_handler,  (void *)"stop");
  signal_handler_connect(sh, "saved", output_signal_handler,  (void *)"saved");
}

void ObsInterface::obs_log_handler(int lvl, const char *msg, va_list args, void *p) {
  char buffer[4096]; // surely should malloc here
  vsnprintf(buffer, sizeof(buffer), msg, args);
  std::cout << buffer << std::endl;
  std::cout.flush();
}

void draw_callback(void* data, uint32_t cx, uint32_t cy) {
    // Render the OBS preview scene here
    obs_render_main_texture();
}

void ObsInterface::showPreview(HWND hwnd) {
  blog(LOG_INFO, "ObsInterface::showPreview");

  if (display) {
    blog(LOG_INFO, "Display already exists, returning early");
    return;  // Return early if display already exists
  }

  // Create an embedded child window for OBS preview
  HWND previewWindow = CreateWindowExA(
    0,                    // No extended styles
    "STATIC",            // Simple static control class (ANSI string)
    "OBS Preview",       // Window name (ANSI string)
    WS_CHILD | WS_VISIBLE | WS_BORDER,  // Child + visible + border
    20, 20,              // Position within parent (x, y)
    1920, 1080,           // Size (width, height)
    hwnd,                // Parent window (your Electron app)
    NULL,                // No menu
    GetModuleHandle(NULL), 
    NULL
  );

  if (!previewWindow) {
    blog(LOG_ERROR, "Failed to create preview child window");
    return;
  }

  // Store for cleanup
  previewHwnd = previewWindow;

  blog(LOG_INFO, "Create OBS display in child window");
  gs_init_data gs_data = {};
  gs_data.adapter = 0;
  gs_data.cx = 1920;              // Match child window size
  gs_data.cy = 1080;              
  gs_data.format = GS_BGRA;
  gs_data.zsformat = GS_ZS_NONE;
  gs_data.num_backbuffers = 1;
  gs_data.window.hwnd = previewWindow;  // Use child window, not parent

  display = obs_display_create(&gs_data, 0x0);
  if (display) {
    obs_display_add_draw_callback(display, draw_callback, NULL);
    blog(LOG_INFO, "OBS preview embedded successfully");
  } else {
    blog(LOG_ERROR, "Failed to create OBS display");
    DestroyWindow(previewWindow);
    previewHwnd = nullptr;
  }
}

void ObsInterface::resizePreview(int width, int height) {
  blog(LOG_INFO, "ObsInterface::resizePreview to size (%d x %d)", width, height);

  if (!previewHwnd) {
    blog(LOG_WARNING, "No preview window to resize");
    return;
  }

  // Resize the child window
  BOOL windowSuccess = SetWindowPos(
    previewHwnd,           // Handle to the child window
    NULL,                  // No Z-order change
    0, 0,                  // Keep current position (ignored due to SWP_NOMOVE)
    width, height,         // New size (width, height)
    SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE  // Don't change position, Z-order, or activation
  );

  if (!windowSuccess) {
    blog(LOG_ERROR, "Failed to resize preview window to (%d x %d)", width, height);
    return;
  }

  blog(LOG_INFO, "Preview window resized successfully to (%d x %d)", width, height);

  // Resize the OBS display to match the new window size
  if (display) {
    obs_display_resize(display, width, height);
    blog(LOG_INFO, "OBS display resized to (%d x %d)", width, height);
  } else {
    blog(LOG_WARNING, "No OBS display to resize");
  }
}

void ObsInterface::movePreview(int x, int y) {
  blog(LOG_INFO, "ObsInterface::movePreview to position (%d, %d)", x, y);

  if (!previewHwnd) {
    blog(LOG_WARNING, "No preview window to move");
    return;
  }

  // Move the child window to the new position within the parent
  BOOL success = SetWindowPos(
    previewHwnd,           // Handle to the child window
    NULL,                  // No Z-order change
    x, y,                  // New position (x, y)
    0, 0,                  // Keep current size (ignored due to SWP_NOSIZE)
    SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE  // Don't change size, Z-order, or activation
  );

  if (success) {
    blog(LOG_INFO, "Preview window moved successfully to (%d, %d)", x, y);
  } else {
    blog(LOG_ERROR, "Failed to move preview window to (%d, %d)", x, y);
  }
}

void ObsInterface::hidePreview() {
  blog(LOG_INFO, "ObsInterface::hidePreview");

  if (display) {
    obs_display_remove_draw_callback(display, draw_callback, NULL);
    obs_display_destroy(display);
    display = nullptr;
    blog(LOG_INFO, "OBS display destroyed");
  }

  // Destroy the child window to fully clean up the preview
  if (previewHwnd) {
    DestroyWindow(previewHwnd);
    previewHwnd = nullptr;
    blog(LOG_INFO, "Preview child window destroyed");
  }
}

ObsInterface::ObsInterface() {
  blog(LOG_INFO, "OBS constructor called");
  init_obs();
  output = create_output();
  scene = create_scene();
  video_source = create_video_source();
  obs_scene_add(scene, video_source);
  create_signal_handlers(output);
  base_set_log_handler(obs_log_handler, NULL);
}

ObsInterface::~ObsInterface() {
  blog(LOG_INFO, "OBS destructor called");

  if (video_source) {
    blog(LOG_INFO, "Releasing video source");
    obs_source_release(video_source);
  }

  if (scene) {
    blog(LOG_INFO, "Releasing scene");
    obs_scene_release(scene);
  }

  if (output) {
    if (obs_output_active(output)) {
      blog(LOG_INFO, "Force stopping output");
      obs_output_force_stop(output);
    }
      
    blog(LOG_INFO, "Releasing output");
    obs_output_release(output);
  }

  if (video_encoder) {
    blog(LOG_INFO, "Releasing video encoder");
    obs_encoder_release(video_encoder);
  }

  if (audio_encoder) {
    blog(LOG_INFO, "Releasing audio encoder");
    obs_encoder_release(audio_encoder);
  }

  blog(LOG_INFO, "Shutting down OBS");
  obs_shutdown();
}

void ObsInterface::startBuffering() {
  blog(LOG_INFO, "ObsInterface::startBuffering called");

  if (!output) {
    throw std::runtime_error("Output is not initialized!");
  }

  bool is_active = obs_output_active(output);
      
  if (is_active) {
    blog(LOG_WARNING, "Output is already active");
    return;
  }

  bool success = obs_output_start(output);

  if (!success) {
    throw std::runtime_error("Failed to start buffering!");
  }
    
  blog(LOG_INFO, "ObsInterface::startBuffering exited");
}

void ObsInterface::startRecording(int offset) {
  bool is_active = obs_output_active(output);

  if (!is_active)
    blog(LOG_ERROR, "Buffer is not active");
    throw std::runtime_error("Buffer is not active");

  std::cout << "calling save proc handler" << std::endl;
  calldata cd;
  calldata_init(&cd);
  calldata_set_int(&cd, "offset_seconds", offset);
  proc_handler_t *ph = obs_output_get_proc_handler(output);
  bool success = proc_handler_call(ph, "convert", &cd);
  calldata_free(&cd);

  if (!success)
    throw std::runtime_error("Failed to call convert procedure handler");
}

void ObsInterface::stopRecording() {
  bool is_active = obs_output_active(output);

  if (!is_active)
    return

  obs_output_stop(output);
}
