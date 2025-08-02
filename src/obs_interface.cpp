#include <windows.h>
#include <obs.h>
#include "utils.h"
#include "obs_interface.h"
#include <vector>
#include <string>
#include <graphics/matrix4.h>
#include <graphics/vec4.h>
#include <util/platform.h>

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
  blog(LOG_INFO, "List encoders");
  blog(LOG_INFO, "List encoders of type: %d", type);
  
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
  blog(LOG_INFO, "List  src types");
  size_t idx = 0;
  const char *src = nullptr;

  while (obs_enum_source_types(idx++, &src)) {
    blog(LOG_INFO, "\t- %s", src);
  }
}

void ObsInterface::list_input_types()
{
  blog(LOG_INFO, "List  input types");
  size_t idx = 0;
  const char *src = nullptr;

  while (obs_enum_input_types(idx++, &src)) {
    blog(LOG_INFO, "\t- %s", src);
  }
}

void ObsInterface::list_output_types()
{
  blog(LOG_INFO, "List  output types");
  size_t idx = 0;
  const char *src = nullptr;

  while (obs_enum_output_types(idx++, &src)) {
    blog(LOG_INFO, "\t- %s", src);
  }
}

void ObsInterface::load_module(const char* module, const char* data) {
  blog(LOG_INFO, "Loading module: %s", module);
  blog(LOG_INFO, "Data path: %s", data);

  obs_module_t *ptr = NULL;
  int success = obs_open_module(&ptr, module, data);

  if (success != MODULE_SUCCESS) {
    blog(LOG_ERROR, "Failed to open module: %s", module);
    throw std::runtime_error("Failed to open module!");
  }

  bool initmod = obs_init_module(ptr);

  if (!initmod) {
    blog(LOG_ERROR, "Failed to initialize module!");
    throw std::runtime_error("Module initialization failed!");
  }
}

void ObsInterface::reset_video() {
  blog(LOG_INFO, "Setup video info");

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

  obs_enter_graphics();
  int  dt = gs_get_device_type();
  blog(LOG_INFO, "Device type = %d", dt);  // should be 1 for D3D11
  obs_leave_graphics();

  if (success != OBS_VIDEO_SUCCESS) {
    blog(LOG_ERROR, "Failed to reset video!");
    throw std::runtime_error("Failed to reset video!");
  }
}

void ObsInterface::reset_audio() {
  struct obs_audio_info oai = {0};
  oai.samples_per_sec = 48000;
  oai.speakers = SPEAKERS_STEREO;
  bool reset = obs_reset_audio(&oai);

  if (!reset) {
    blog(LOG_ERROR, "Failed to reset audio!");
    throw std::runtime_error("Failed to reset audio!");
  }
}

void ObsInterface::init_obs(const std::string& pluginPath, const std::string& dataPath) {
  blog(LOG_INFO, "Enter init_obs");
  auto success = obs_startup("en-US", NULL, NULL);

  if (!success) {
    blog(LOG_ERROR, "Failed to start OBS!");
    throw std::runtime_error("OBS startup failed");
  }

  if (!obs_initialized()) {
    blog(LOG_ERROR, "OBS not initialized!");
    throw std::runtime_error("OBS initialization failed");
  }
  
  std::string effectsPath = dataPath;

  if (effectsPath.back() != '/' && effectsPath.back() != '\\') {
    // Add a trailing slash if not present.
    effectsPath += '\\';
  }

  effectsPath += "effects\\";

  // Add the effects path. We need this before resetting video and audio
  // to ensure the effects are available. The function is deprecated in
  // libobs but it works for now.
  obs_add_data_path(effectsPath.c_str());

  // This must come before loading modules to initialize D3D11.
  reset_video();
  reset_audio();

  std::string pluginDataPath = dataPath;

  if (pluginDataPath.back() != '/' && pluginDataPath.back() != '\\') {
    // Add a trailing slash if not present.
    pluginDataPath += '\\';
  }

  pluginDataPath += "obs-plugins\\";


  std::vector<std::string> modules = { 
    "obs-x264", 
    "obs-ffmpeg",
    "win-capture",  // Required for basically all forms of capture on Windows.
    "image-source", // Required for image sources.
    "win-wasapi"    // Required for WASAPI audio input.
  };

  for (const auto& module : modules) {
    std::string modulePath = pluginPath + "/" + module + ".dll";
    std::string moduleDataPath = pluginDataPath + module;
    load_module(modulePath.c_str(), moduleDataPath.c_str());
  }
  
  obs_post_load_modules();

  list_encoders();
  list_source_types();
  list_input_types();
  list_output_types();

  blog(LOG_INFO, "Exit init_obs");
}

void ObsInterface::create_output() {
  blog(LOG_INFO, "Create outputs");

  blog(LOG_INFO, "Creating replay buffer output");
  buffer_output = obs_output_create("replay_buffer", "Buffer Output", NULL, NULL);

  if (!buffer_output) {
    blog(LOG_ERROR, "Failed to create buffer output!");
    throw std::runtime_error("Failed to create buffer output!");
  }

  blog(LOG_INFO, "Creating file output");
  file_output = obs_output_create("ffmpeg_muxer", "File Output", NULL, NULL);

  if (!file_output) {
    blog(LOG_ERROR, "Failed to create file output!");
    throw std::runtime_error("Failed to create file output!");
  }

  obs_data_t *buffer_settings = obs_data_create();
  blog(LOG_INFO, "Set replay_buffer settings");
  obs_data_set_int(buffer_settings, "max_time_sec", 60);
  obs_data_set_int(buffer_settings, "max_size_mb", 1024);
  obs_data_set_string(buffer_settings, "directory", recording_path.c_str());
  obs_data_set_string(buffer_settings, "format", "%CCYY-%MM-%DD %hh-%mm-%ss");
  obs_data_set_string(buffer_settings, "extension", "mp4");
  obs_output_update(buffer_output, buffer_settings);
  obs_data_release(buffer_settings);

  blog(LOG_INFO, "Set ffmpeg_muxer settings");
  obs_data_t *ffmpeg_settings = obs_data_create();
  // Need to specify the exact path for ffmpeg_muxer.
  std::string filename = recording_path + "\\" + get_current_date_time() + ".mp4";
  obs_data_set_string(ffmpeg_settings, "path", filename.c_str());
  recording_path = filename;

  // Apply and release the settings.
  obs_output_update(file_output, ffmpeg_settings);
  obs_data_release(ffmpeg_settings);

  // Add the signal handler callback.
  connect_signal_handlers(buffer_output);
  connect_signal_handlers(file_output);
}

void ObsInterface::setRecordingDir(const std::string& recordingPath) {
  blog(LOG_INFO, "Set recording directory");
  // TODO make this work for file output also.

  obs_output_t *output = buffering ? buffer_output : file_output;

  if (!output) {
    blog(LOG_ERROR, "No output to update recording directory");
    throw std::runtime_error("Output not initialized");
  }

  // check its not active
  if (obs_output_active(output)) {
    blog(LOG_ERROR, "Output is active, cannot update recording path");
    throw std::runtime_error("Output is active, cannot update recording path");
  }

  obs_data_t *settings = obs_output_get_settings(output);

  if (!settings) {
    blog(LOG_ERROR, "Failed to get output settings");
    throw std::runtime_error("Failed to get output settings");
  }

  obs_data_set_string(settings, "directory", recordingPath.c_str());
  obs_output_update(output, settings);
  obs_data_release(settings);
}

void ObsInterface::create_video_encoders() {
  blog(LOG_INFO, "Create video encoder");

  file_video_encoder = obs_video_encoder_create("obs_x264", "h264_stream_file", NULL, NULL);


  if (!file_video_encoder) {
    blog(LOG_ERROR, "Failed to create video encoder!");
    throw std::runtime_error("Failed to create video encoder!");
  }

  buffer_video_encoder = obs_video_encoder_create("obs_x264", "h264_stream_buffer", NULL, NULL);

  if (!buffer_video_encoder) {
    blog(LOG_ERROR, "Failed to create buffer video encoder!");
    throw std::runtime_error("Failed to create buffer video encoder!");
  }

  blog(LOG_INFO, "Set file video encoder settings");
  obs_data_t* venc_settings = obs_data_create();
  // obs_data_set_string(venc_settings, "preset", "speed");  // Faster preset
  obs_data_set_string(venc_settings, "rate_control", "CRF");
  obs_data_set_int(venc_settings, "crf", 22);
  obs_data_set_string(venc_settings, "profile", "main");
  obs_data_set_int(venc_settings, "keyint_sec", 1); // Set keyframe interval to 1 second

  obs_encoder_update(file_video_encoder, venc_settings);
  obs_encoder_update(buffer_video_encoder, venc_settings);
  obs_data_release(venc_settings);

  obs_output_set_video_encoder(file_output, file_video_encoder);
  obs_encoder_set_video(file_video_encoder, obs_get_video());
  obs_output_set_video_encoder(buffer_output, buffer_video_encoder);
  obs_encoder_set_video(buffer_video_encoder, obs_get_video());
}

void ObsInterface::create_audio_encoders() {
  blog(LOG_INFO, "Create audio encoder");

  // if (!output) {
  //   blog(LOG_ERROR, "No output on create_audio_encoders");
  //   throw std::runtime_error("Failed to create audio encoder!");
  // }

  // if (audio_encoder) {
  //   blog(LOG_DEBUG, "Releasing audio encoder");
  //   obs_encoder_release(audio_encoder);
  //   audio_encoder = nullptr;
  // }

  file_audio_encoder = obs_audio_encoder_create("ffmpeg_aac", "aac_file", NULL, 0, NULL);

  if (!file_audio_encoder) {
    blog(LOG_ERROR, "Failed to create audio encoder!");
    throw std::runtime_error("Failed to create audio encoder!");
  }

  buffer_audio_encoder = obs_audio_encoder_create("ffmpeg_aac", "aac_buffer", NULL, 0, NULL);

  if (!buffer_audio_encoder) {
    blog(LOG_ERROR, "Failed to create buffer audio encoder!");
    throw std::runtime_error("Failed to create buffer audio encoder!");
  }

  blog(LOG_INFO, "Set audio encoder settings");
  obs_data_t *aenc_settings = obs_data_create();
  obs_data_set_int(aenc_settings, "bitrate", 128);
  obs_encoder_update(file_audio_encoder, aenc_settings);
  obs_encoder_update(buffer_audio_encoder, aenc_settings);
  obs_data_release(aenc_settings);

  obs_output_set_audio_encoder(file_output, file_audio_encoder, 0);
  obs_encoder_set_audio(file_audio_encoder, obs_get_audio());

  obs_output_set_audio_encoder(buffer_output, buffer_audio_encoder, 0);
  obs_encoder_set_audio(buffer_audio_encoder, obs_get_audio());
}

void ObsInterface::create_scene() {
  blog(LOG_INFO, "Create scene");
  scene = obs_scene_create("WCR Scene");

  if (!scene) {
    blog(LOG_ERROR, "Failed to create scene!");
    throw std::runtime_error("Failed to create scene!");
  }

  obs_source_t *scene_source = obs_scene_get_source(scene);

  if (!scene_source) {
    blog(LOG_ERROR, "Failed to get scene source!");
    throw std::runtime_error("Failed to get scene source!");
  }

  obs_set_output_source(0, scene_source); // 0 = video track
}

void ObsInterface::createSource(std::string name, std::string type) {
  blog(LOG_INFO, "Create source: %s of type %s", name.c_str(), type.c_str());

  obs_source_t *source = obs_source_create(
    type.c_str(), 
    name.c_str(), 
    NULL, // No settings.
    NULL  // No hotkey data.
  );

  if (!source) {
    blog(LOG_ERROR, "Failed to create source: %s", name.c_str());
    throw std::runtime_error("Failed to create source!");
  }
}

void ObsInterface::deleteSource(std::string name) {
  blog(LOG_INFO, "Delete source: %s", name.c_str());
  obs_source_t *source = obs_get_source_by_name(name.c_str());
  
  if (!source) {
    blog(LOG_WARNING, "Source not found: %s", name.c_str());
    return; // Source not found, nothing to delete.
  }

  obs_source_release(source);
  blog(LOG_INFO, "Source deleted: %s", name.c_str());
}

obs_data_t* ObsInterface::getSourceSettings(std::string name) {
  blog(LOG_INFO, "Get source settings for: %s", name.c_str());
  obs_source_t *source = obs_get_source_by_name(name.c_str());
  
  if (!source) {
    blog(LOG_ERROR, "Source not found: %s", name.c_str());
    throw std::runtime_error("Source not found!");
  }

  obs_data_t *settings = obs_source_get_settings(source);
  
  if (!settings) {
    blog(LOG_ERROR, "Failed to get settings for source: %s", name.c_str());
    throw std::runtime_error("Failed to get source settings!");
  }

  return settings;
}

void ObsInterface::setSourceSettings(std::string name, obs_data_t* settings) {
  blog(LOG_INFO, "Set source settings for: %s", name.c_str());
  obs_source_t *source = obs_get_source_by_name(name.c_str());

  if (!source) {
    blog(LOG_ERROR, "Source not found: %s", name.c_str());
    throw std::runtime_error("Source not found!");
  }

  obs_source_update(source, settings);
}

obs_properties_t* ObsInterface::getSourceProperties(std::string name) {
  blog(LOG_INFO, "Get source properties for: %s", name.c_str());
  obs_source_t *source = obs_get_source_by_name(name.c_str());

  if (!source) {
    blog(LOG_ERROR, "Source not found: %s", name.c_str());
    throw std::runtime_error("Source not found!");
  }

  obs_properties_t *props = obs_source_properties(source);

  if (!props) {
    blog(LOG_ERROR, "Failed to get properties for source: %s", name.c_str());
    throw std::runtime_error("Failed to get source properties!");
  }

  return props;
}

void call_jscb(Napi::Env env, Napi::Function cb, SignalData* sd) {
  Napi::Object obj = Napi::Object::New(env);
  obj.Set("id", Napi::String::New(env, sd->id));
  obj.Set("code", Napi::Number::New(env, sd->code));
  cb.Call({ obj });
  delete sd;
}

void ObsInterface::output_signal_handler_starting(void *data, calldata_t *cd) {
  long long code = calldata_int(cd, "code");
  ObsInterface* self = static_cast<ObsInterface*>(data);
  SignalData* sd = new SignalData{ "starting", code };
  self->jscb.NonBlockingCall(sd, call_jscb);
}

void ObsInterface::output_signal_handler_start(void *data, calldata_t *cd) {
  long long code = calldata_int(cd, "code");
  ObsInterface* self = static_cast<ObsInterface*>(data);
  SignalData* sd = new SignalData{ "start", code };
  self->jscb.NonBlockingCall(sd, call_jscb);
}

void ObsInterface::output_signal_handler_stop(void *data, calldata_t *cd) {
  long long code = calldata_int(cd, "code");
  ObsInterface* self = static_cast<ObsInterface*>(data);
  SignalData* sd = new SignalData{ "stop", code };
  self->jscb.NonBlockingCall(sd, call_jscb);
}

void ObsInterface::output_signal_handler_stopping(void *data, calldata_t *cd) {
  long long code = calldata_int(cd, "code");
  ObsInterface* self = static_cast<ObsInterface*>(data);
  SignalData* sd = new SignalData{ "stopping", code };
  self->jscb.NonBlockingCall(sd, call_jscb);
}

void ObsInterface::output_signal_handler_saved(void *data, calldata_t *cd) {
  long long code = calldata_int(cd, "code");
  ObsInterface* self = static_cast<ObsInterface*>(data);
  SignalData* sd = new SignalData{ "saved", code };
  self->jscb.NonBlockingCall(sd, call_jscb);
}

void ObsInterface::connect_signal_handlers(obs_output_t *output) {
  signal_handler_t *sh = obs_output_get_signal_handler(output);
  signal_handler_connect(sh, "starting", output_signal_handler_starting,  this);
  signal_handler_connect(sh, "start", output_signal_handler_start,  this);
  signal_handler_connect(sh, "stopping", output_signal_handler_stopping,  this);
  signal_handler_connect(sh, "stop", output_signal_handler_stop,  this);
  signal_handler_connect(sh, "saved", output_signal_handler_saved,  this);
}

void ObsInterface::disconnect_signal_handlers(obs_output_t *output) {
  signal_handler_t *sh = obs_output_get_signal_handler(output);
  signal_handler_disconnect(sh, "starting", output_signal_handler_starting,  this);
  signal_handler_disconnect(sh, "start", output_signal_handler_start,  this);
  signal_handler_disconnect(sh, "stopping", output_signal_handler_stopping,  this);
  signal_handler_disconnect(sh, "stop", output_signal_handler_stop,  this);
  signal_handler_disconnect(sh, "saved", output_signal_handler_saved,  this);
}

bool draw_source_outline(obs_scene_t *scene, obs_sceneitem_t *item, void *p) {
  // Get the item position and size
  vec2 pos; vec2 scale;
  obs_sceneitem_get_pos(item, &pos);
  obs_sceneitem_get_scale(item, &scale);

  // Calculate actual size with scaling
  obs_source_t *src = obs_sceneitem_get_source(item);
  float width =  obs_source_get_width(src) * scale.x;
  float height = obs_source_get_height(src) * scale.y;

  if (width <= 0 || height <= 0) {
    // Don't want to call gs_draw_sprite with zero width or height.
    // It is obviously nonsense and leads to log spam. Just return early.
    return true;
  }

  // Draw rectangle around the source using the position and size
  gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
  gs_eparam_t *color = gs_effect_get_param_by_name(solid, "color");
  gs_technique_t *tech = gs_effect_get_technique(solid, "Solid");

  vec4 col = {0.733f, 0.267f, 0.125f, 1.0f}; // #BB4420
  gs_effect_set_vec4(color, &col);

  gs_technique_begin(tech);
  gs_technique_begin_pass(tech, 0);

  gs_matrix_push();
  gs_matrix_identity();

  // Top border
  gs_matrix_push();
  gs_matrix_translate3f(pos.x, pos.y, 0.0f);
  gs_draw_sprite(nullptr, 0, width, 4.0f);
  gs_matrix_pop();

  // Bottom border
  gs_matrix_push();
  gs_matrix_translate3f(pos.x, pos.y + height - 4.0f, 0.0f);
  gs_draw_sprite(nullptr, 0, width, 4.0f);
  gs_matrix_pop();

  // Left border
  gs_matrix_push();
  gs_matrix_translate3f(pos.x, pos.y, 0.0f);
  gs_draw_sprite(nullptr, 0, 4.0f, height);
  gs_matrix_pop();

  // Right border
  gs_matrix_push();
  gs_matrix_translate3f(pos.x + width - 4.0f, pos.y, 0.0f);
  gs_draw_sprite(nullptr, 0, 4.0f, height);
  gs_matrix_pop();

  gs_matrix_pop();

  gs_technique_end_pass(tech);
  gs_technique_end(tech);

  return true;
}

void draw_callback(void* data, uint32_t cx, uint32_t cy) {
  ObsInterface* obsInterface = (ObsInterface*)data;

  obs_video_info ovi;
  obs_get_video_info(&ovi);

  float scaleX = float(cx) / float(ovi.base_width);
  float scaleY = float(cy) / float(ovi.base_height);

  float previewScale;

  // Pick the limiting scale factor.
  if (scaleX < scaleY) {
    previewScale = scaleX;
  } else {
    previewScale = scaleY;
  }

  int previewCX = int(previewScale * ovi.base_width);
  int previewCY = int(previewScale * ovi.base_height);
  int previewX = (cx - previewCX) / 2;
  int previewY = (cy - previewCY) / 2;

  gs_viewport_push();
	gs_projection_push();

  gs_ortho(0.0f, float(ovi.base_width), 0.0f, float(ovi.base_height), -100.0f, 100.0f);
  gs_set_viewport(previewX, previewY, previewCX, previewCY);

  // Renders the scene now the graphics context is setup.
  obs_render_main_texture();

  // Draw boxes around sources.
  obs_scene_t* scene = obs_get_scene_by_name("WCR Scene");
  if (obsInterface->getDrawSourceOutlineEnabled()) {
    obs_scene_enum_items(scene, draw_source_outline, NULL);
  }
  obs_scene_release(scene);

	gs_projection_pop();
	gs_viewport_pop();
}

void ObsInterface::initPreview(HWND parent) {
  blog(LOG_INFO, "ObsInterface::initPreview");

  if (!preview_hwnd) {
    blog(LOG_INFO, "Creating preview child window");

    preview_hwnd = CreateWindowExA(
      0,                      // No extended styles
      "STATIC",               // Simple static control class (ANSI string)
      "OBS Preview",          // Window name (ANSI string)
      WS_CHILD | WS_BORDER,   // Child + border, NOT visible initially
      0, 0,                   // Initial position (x, y)
      0, 0,                   // Initial size (width, height)
      parent,                 // Parent window (your Electron app)
      NULL,                   // No menu
      GetModuleHandle(NULL),
      NULL
    );

    if (!preview_hwnd) {
      blog(LOG_ERROR, "Failed to create preview child window");
      return;
    }
  }

  if (!display) {
    blog(LOG_INFO, "Create OBS display in child window");

    gs_init_data gs_data = {};
    gs_data.adapter = 0;
    gs_data.cx = 1920; // TODO get from video context?
    gs_data.cy = 1080; // TODO get from video context?
    gs_data.format = GS_BGRA;
    gs_data.zsformat = GS_ZS_NONE;
    gs_data.num_backbuffers = 1;
    gs_data.window.hwnd = preview_hwnd;

    display = obs_display_create(&gs_data, 0x0);

    if (!display) {
      blog(LOG_ERROR, "Failed to create OBS display");
      return;
    }

    obs_display_add_draw_callback(display, draw_callback, this);
  }

  obs_display_set_enabled(display, false);
}

void ObsInterface::showPreview(int x, int y, int width, int height) {
  blog(LOG_INFO, "ObsInterface::showPreview");

  if (!preview_hwnd || !display) {
    blog(LOG_ERROR, "Preview window not initialized");
    return;
  }

  blog(LOG_INFO, "Showing preview child window at (%d, %d) with size (%d x %d)", x, y, width, height);

  // Resize and move the existing child window.
  bool success = SetWindowPos(
    preview_hwnd,                  // Handle to the child window
    NULL,                          // No Z-order change
    x, y,                          // New position (x, y)
    width, height,                 // New size (width, height)
    SWP_NOACTIVATE                 // Flags
  );

  if (!success) {
    blog(LOG_ERROR, "Failed to resize preview window to (%d x %d)", width, height);
    return;
  }

  uint32_t w, h;
  obs_display_size(display, &w, &h); // Get the display size to match the video context.
  blog(LOG_INFO, "Current Display size set to (%d x %d)", w, h);

  obs_display_resize(display, width, height);
  ShowWindow(preview_hwnd, SW_SHOW);
  obs_display_set_enabled(display, true);
}

void ObsInterface::hidePreview() {
  blog(LOG_INFO, "ObsInterface::hidePreview");

  if (preview_hwnd) {
    ShowWindow(preview_hwnd, SW_HIDE);
    blog(LOG_INFO, "Preview child window hidden");
  }

  obs_display_set_enabled(display, false);
}

float ObsInterface::getPreviewScaleFactor() {
  if (!display) {
    blog(LOG_WARNING, "Display not initialized");
    return 1.0f; // Default scale
  }

  obs_video_info ovi;
  obs_get_video_info(&ovi);

  uint32_t width, height;
	obs_display_size(display, &width, &height);

  float scaleX = float(width) / float(ovi.base_width);
  float scaleY = float(height) / float(ovi.base_height);

  float previewScale;

  // Pick the limiting scale factor.
  if (scaleX < scaleY) {
    previewScale = scaleX;
  } else {
    previewScale = scaleY;
  }

  return previewScale;
}

void ObsInterface::setDrawSourceOutline(bool enabled) {
  drawSourceOutline = enabled;
}

bool ObsInterface::getDrawSourceOutlineEnabled() {
  return drawSourceOutline;
}

ObsInterface::ObsInterface(
  const std::string& pluginPath, 
  const std::string& logPath, 
  const std::string& dataPath,  
  const std::string& recordingPath,
  Napi::ThreadSafeFunction cb
) {
  // Setup logs first so we have logs for the initialization.
  base_set_log_handler(log_handler, (void*)logPath.c_str());
  blog(LOG_DEBUG, "Creating ObsInterface");

  // Initialize OBS and load required modules.
  init_obs(pluginPath, dataPath);

  // Setup callback function.
  jscb = cb;
  recording_path = recordingPath;

  // Create the resources we rely on.
  create_output();
  create_scene();

  create_video_encoders();
  create_audio_encoders();
}

ObsInterface::~ObsInterface() {
  blog(LOG_DEBUG, "Destroying ObsInterface");

  if (jscb) {
    blog(LOG_DEBUG, "Releasing JavaScript callback");
    jscb.Release();
  }

  if (video_source) {
    blog(LOG_DEBUG, "Releasing video source");
    obs_source_release(video_source);
  }

  if (scene) {
    blog(LOG_DEBUG, "Releasing scene");
    obs_scene_release(scene);
  }

  if (buffer_output) {
    if (obs_output_active(buffer_output)) {
      blog(LOG_DEBUG, "Force stopping output");
      obs_output_force_stop(buffer_output);
    }
      
    blog(LOG_DEBUG, "Releasing output");
    obs_output_release(buffer_output);
  }

  if (file_output) {
    if (obs_output_active(file_output)) {
      blog(LOG_DEBUG, "Force stopping output");
      obs_output_force_stop(file_output);
    }
      
    blog(LOG_DEBUG, "Releasing output");
    obs_output_release(file_output);
  }

  // if (video_encoder) {
  //   blog(LOG_DEBUG, "Releasing video encoder");
  //   obs_encoder_release(video_encoder);
  // }

  // if (audio_encoder) {
  //   blog(LOG_DEBUG, "Releasing audio encoder");
  //   obs_encoder_release(audio_encoder);
  // }

  blog(LOG_DEBUG, "Now shutting down OBS");
  obs_shutdown();
}

bool ObsInterface::setBuffering(bool value) {
  obs_output_t* output = buffering ? buffer_output : file_output;

  if (obs_output_active(output)) {
    blog(LOG_ERROR, "Cannot change buffering state while output is active");
    return false;
  }

  buffering = value;
  return buffering;
}

void ObsInterface::startBuffering() {
  blog(LOG_INFO, "ObsInterface::startBuffering called");

  if (!buffering) {
    blog(LOG_ERROR, "Buffering is not enabled!");
    throw std::runtime_error("Buffering is not enabled!");
  }

  obs_output_t* output = buffer_output;

  if (!output) {
    blog(LOG_ERROR, "Output is not initialized!");
    throw std::runtime_error("Output is not initialized!");
  }

  bool is_active = obs_output_active(output);

  if (is_active) {
    blog(LOG_WARNING, "Output is already active");
    return;
  }

  bool success = obs_output_start(output);

  if (!success) {
    blog(LOG_ERROR, "Failed to start buffering!");
    throw std::runtime_error("Failed to start buffering!");
  }
    
  blog(LOG_INFO, "ObsInterface::startBuffering exited");
}

void ObsInterface::startRecording(int offset) {
  blog(LOG_INFO, "ObsInterface::startRecording enter");
  obs_output_t* output = buffering ? buffer_output : file_output;

  if (buffering) {
    bool is_active = obs_output_active(output);

    if (!is_active) {
      blog(LOG_WARNING, "Buffer is not active");
      throw std::runtime_error("Buffer is not active");
    }

    blog(LOG_INFO, "calling save proc handler");
    calldata cd;
    calldata_init(&cd);
    calldata_set_int(&cd, "offset_seconds", offset);
    proc_handler_t *ph = obs_output_get_proc_handler(output);
    bool success = proc_handler_call(ph, "convert", &cd);
    calldata_free(&cd);

    if (!success) {
      throw std::runtime_error("Failed to call convert procedure handler");
    }
  } else {
    blog(LOG_INFO, "Starting ffmpeg_muxer output");

    bool is_active = obs_output_active(output);

    if (is_active) {
      blog(LOG_WARNING, "Output already active");
      return;
    }

    blog(LOG_WARNING, "Call start");
    bool success = obs_output_start(output);

    if (!success) {
      const char *err = obs_output_get_last_error(output);
      blog(LOG_ERROR, "Failed to start recording: %s", err ? err : "Unknown error");
      throw std::runtime_error("Failed to start recording");
    }
  }

  blog(LOG_INFO, "ObsInterface::startRecording exit");
}

void ObsInterface::stopRecording() {
  blog(LOG_INFO, "ObsInterface::stopRecording enter");
  obs_output_t* output = buffering ? buffer_output : file_output;
  bool is_active = obs_output_active(output);

  if (!is_active) {
    blog(LOG_WARNING, "Output is not active");
    return;
  }

  obs_output_stop(output);
  blog(LOG_INFO, "ObsInterface::stopRecording exited");
}

std::string ObsInterface::getLastRecording() {
  blog(LOG_INFO, "calling get last replay proc handler");
  calldata cd;
  calldata_init(&cd);

  obs_output_t* output = buffering ? buffer_output : file_output;
  proc_handler_t *ph = obs_output_get_proc_handler(output);

  const char* type = obs_output_get_id(output);

  if (!buffering) {
    blog(LOG_INFO, "Getting last recording path from ffmpeg_muxer");
    return recording_path;
  }

  bool success = proc_handler_call(ph, "get_last_replay", &cd);

  if (!success) {
    blog(LOG_ERROR, "Failed to call procedure handler");
    const char *err = obs_output_get_last_error(output);
    blog(LOG_ERROR, "%s", err ? err : "Unknown error");
    calldata_free(&cd);
    return "";
  }

  const char* p = calldata_string(&cd, "path");
  std::string path = p ? p : "" ;
  calldata_free(&cd);
  
  blog(LOG_INFO, "return path: %s", path.c_str());
  return path;
}

void ObsInterface::addSourceToScene(std::string name) {
  blog(LOG_INFO, "ObsInterface::addSourceToScene called for source: %s", name.c_str());

  obs_source_t *src = obs_get_source_by_name(name.c_str());
  
  if (!src) {
    blog(LOG_WARNING, "Did not find source for video source: %s", name.c_str());
    return;
  }

  // TODO refuse to add twice?

  obs_sceneitem_t *item = obs_scene_add(scene, src);
  
  if (!item) {
    blog(LOG_ERROR, "Failed to add source to scene: %s", name.c_str());
    obs_source_release(src);
    return;
  }
  
  blog(LOG_INFO, "ObsInterface::addSourceToScene exited");
}

void ObsInterface::removeSourceFromScene(std::string name) {
  blog(LOG_INFO, "ObsInterface::removeSourceFromScene called for source: %s", name.c_str());

  obs_sceneitem_t *item = obs_scene_find_source(scene, name.c_str());
  
  if (!item) {
    blog(LOG_WARNING, "Did not find scene item for video source: %s", name.c_str());
    return;
  }

  obs_sceneitem_remove(item);
  blog(LOG_INFO, "ObsInterface::removeSourceFromScene exited");
}

void ObsInterface::getSourcePos(std::string name, vec2* pos, vec2* size, vec2* scale) 
{
  obs_source_t *src = obs_get_source_by_name(name.c_str());
  obs_sceneitem_t *item = obs_scene_find_source(scene, name.c_str());

  if (!src) {
    blog(LOG_WARNING, "Did not find source for video source: %s", name.c_str());
    return;
  }

  if (!item) {
    blog(LOG_WARNING, "Did not find scene item for video source: %s", name.c_str());
    return;
  }

  obs_sceneitem_get_pos(item, pos);
  obs_sceneitem_get_scale(item, scale);

  // Pre-scaled sizes.
  size->x = obs_source_get_width(src);
  size->y = obs_source_get_height(src);
}

void ObsInterface::setSourcePos(std::string name, vec2* pos, vec2* scale) {
  obs_sceneitem_t *item = obs_scene_find_source(scene, name.c_str());

  if (!item) {
    blog(LOG_WARNING, "Did not find scene item for video source: %s", name.c_str());
    return;
  }

  obs_sceneitem_set_pos(item, pos);
  obs_sceneitem_set_scale(item, scale);
}