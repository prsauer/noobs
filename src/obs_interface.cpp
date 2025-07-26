#include <iostream>
#include <windows.h>
#include <obs.h>
#include "utils.h"
#include <chrono>
#include "obs_interface.h"
#include <vector>
#include <thread>
#include <iostream>
#include <map>
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

void ObsInterface::load_module(const char* module) {
  blog(LOG_INFO, "Loading module: %s", module);

  obs_module_t *ptr = NULL;
  int success = obs_open_module(&ptr, module, NULL);

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
  
  std::string dp = dataPath;

  if (dp.back() != '/' && dp.back() != '\\') {
    // Add a trailing slash if not present, else libobs gets upset.
    dp += '/';
  }

  obs_add_data_path(dp.c_str());  // This is deprecated in libobs but it works for now.

  std::vector<std::string> modules = { 
    "obs-x264.dll", 
    "obs-ffmpeg.dll",
    "win-capture.dll",  // Required for basically all forms of capture on Windows.
    "image-source.dll", // Required for image sources.
    "win-wasapi.dll"    // Required for WASAPI audio input.
  };

  for (const auto& module : modules) {
    std::string path = pluginPath + "/" + module;
    load_module(path.c_str());
  }
  
  obs_post_load_modules();

  list_encoders();
  list_source_types();
  list_input_types();
  list_output_types();

  reset_video();
  reset_audio();

  blog(LOG_INFO, "Exit init_obs");
}

void ObsInterface::create_output(const std::string& recordingPath) {
  blog(LOG_INFO, "Create output");

  if (output) {
    blog(LOG_DEBUG, "Releasing output");
    obs_output_release(output);
  }

  output = obs_output_create("replay_buffer", "recording_output", NULL, NULL);

  if (!output) {
    blog(LOG_ERROR, "Failed to create output!");
    throw std::runtime_error("Failed to create output!");
  }
  
  blog(LOG_INFO, "Set output settings");
  obs_data_t *settings = obs_data_create();
  obs_data_set_int(settings, "max_time_sec", 60);
  obs_data_set_int(settings, "max_size_mb", 1024);
  obs_data_set_string(settings, "directory", recordingPath.c_str());
  obs_data_set_string(settings, "format", "%CCYY-%MM-%DD %hh-%mm-%ss");
  obs_data_set_string(settings, "extension", "mp4");
  obs_output_update(output, settings);
  obs_data_release(settings);

  // Add the signal handler callback.
  create_signal_handlers(output);
}

void ObsInterface::updateRecordingDir(const std::string& recordingPath) {
  blog(LOG_INFO, "Updating recording directory");

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

void ObsInterface::configure_video_encoder() {
  blog(LOG_INFO, "Create video encoder");

  if (!output) {
    blog(LOG_ERROR, "No output on configure_video_encoder");
    throw std::runtime_error("Failed to create video encoder!");
  }

  if (video_encoder) {
    blog(LOG_DEBUG, "Releasing video encoder");
    obs_encoder_release(video_encoder);
    video_encoder = nullptr;
  }

  video_encoder = obs_video_encoder_create("obs_x264", "simple_h264_stream", NULL, NULL);

  if (!video_encoder) {
    blog(LOG_ERROR, "Failed to create video encoder!");
    throw std::runtime_error("Failed to create video encoder!");
  }

  blog(LOG_INFO, "Set video encoder settings");
  obs_data_t* venc_settings = obs_data_create();
  obs_data_set_string(venc_settings, "preset", "speed");  // Faster preset
  obs_data_set_string(venc_settings, "rate_control", "CRF");
  obs_data_set_int(venc_settings, "crf", 30);
  obs_data_set_string(venc_settings, "profile", "main");
  obs_data_set_int(venc_settings, "keyint_sec", 1); // Set keyframe interval to 1 second
  obs_encoder_update(video_encoder, venc_settings);
  obs_data_release(venc_settings);
}

void ObsInterface::configure_audio_encoder() {
  blog(LOG_INFO, "Create audio encoder");

  if (!output) {
    blog(LOG_ERROR, "No output on configure_audio_encoder");
    throw std::runtime_error("Failed to create audio encoder!");
  }

  if (audio_encoder) {
    blog(LOG_DEBUG, "Releasing audio encoder");
    obs_encoder_release(audio_encoder);
    audio_encoder = nullptr;
  }

  audio_encoder = obs_audio_encoder_create("ffmpeg_aac", "simple_aac", NULL, 0, NULL);

  if (!audio_encoder) {
    blog(LOG_ERROR, "Failed to create audio encoder!");
    throw std::runtime_error("Failed to create audio encoder!");
  }

  blog(LOG_INFO, "Set audio encoder settings");
  obs_data_t *aenc_settings = obs_data_create();
  obs_data_set_int(aenc_settings, "bitrate", 128);
  obs_encoder_update(audio_encoder, aenc_settings);
  obs_data_release(aenc_settings);

  obs_output_set_video_encoder(output, video_encoder);
  obs_output_set_audio_encoder(output, audio_encoder, 0);

  obs_encoder_set_video(video_encoder, obs_get_video());
  obs_encoder_set_audio(audio_encoder, obs_get_audio());
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
  // obs_scene_add(scene, video_source);
}

// void ObsInterface::configure_video_source() {
//   blog(LOG_INFO, "Create monitor capture source");

//   // Create settings for monitor capture
//   obs_data_t *monitor_settings = obs_data_create();
//   obs_data_set_int(monitor_settings, "monitor", 0);  // Monitor 0
//   obs_data_set_bool(monitor_settings, "capture_cursor", true);

//   video_source = obs_source_create("monitor_capture", "video_source", monitor_settings, NULL);
//   obs_data_release(monitor_settings);

//   if (!video_source) {
//     blog(LOG_ERROR, "Failed to create video source!");
//     throw std::runtime_error("Failed to create video source!");
//   }
// }

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

void ObsInterface::create_signal_handlers(obs_output_t *output) {
  signal_handler_t *sh = obs_output_get_signal_handler(output);
  signal_handler_connect(sh, "starting", output_signal_handler_starting,  this);
  signal_handler_connect(sh, "start", output_signal_handler_start,  this);
  signal_handler_connect(sh, "stopping", output_signal_handler_stopping,  this);
  signal_handler_connect(sh, "stop", output_signal_handler_stop,  this);
  signal_handler_connect(sh, "saved", output_signal_handler_saved,  this);
}


static vec4 ConvertColorToVec4(uint32_t color)
{
	vec4 colorVec4;
	vec4_set(&colorVec4, static_cast<float>(color & 0xFF) / 255.0f, static_cast<float>((color & 0xFF00) >> 8) / 255.0f,
		 static_cast<float>((color & 0xFF0000) >> 16) / 255.0f, static_cast<float>((color & 0xFF000000) >> 24) / 255.0f);
	return colorVec4;
}

	uint32_t m_paddingSize = 10;
	uint32_t m_paddingColor = 0xFF222222;
	/// Other
	uint32_t m_backgroundColor = 0xFF000000;     // 0, 0, 0
	uint32_t m_outlineColor = 0xFFA8E61A;        // 26, 230, 168
	uint32_t m_cropOutlineColor = 0xFFA8E61A;    // 26, 230, 168
	uint32_t m_guidelineColor = 0xFFA8E61A;      // 26, 230, 168
	uint32_t m_resizeOuterColor = 0xFF7E7E7E;    // 126, 126, 126
	uint32_t m_resizeInnerColor = 0xFFFFFFFF;    // 255, 255, 255
	uint32_t m_rotationHandleColor = 0xFFA8E61A; // 26, 230, 168

	vec4 m_paddingColorVec4 = ConvertColorToVec4(m_paddingColor);
	vec4 m_backgroundColorVec4 = ConvertColorToVec4(m_backgroundColor);
	vec4 m_outlineColorVec4 = ConvertColorToVec4(m_outlineColor);
	vec4 m_cropOutlineColorVec4 = ConvertColorToVec4(m_cropOutlineColor);
	vec4 m_guidelineColorVec4 = ConvertColorToVec4(m_guidelineColor);
	vec4 m_resizeOuterColorVec4 = ConvertColorToVec4(m_resizeOuterColor);
	vec4 m_resizeInnerColorVec4 = ConvertColorToVec4(m_resizeInnerColor);
	vec4 m_rotationHandleColorVec4 = ConvertColorToVec4(m_rotationHandleColor);
  

// Helper to draw rectangle outline with 4 lines using GS lines primitive
void draw_rectangle_outline(float x1, float y1, float x2, float y2) {
    // Vertex format: position (vec3), color (uint32)
    struct vertex {
        float x, y, z;
        uint32_t color;
    };

    const uint32_t white = 0xFFFFFFFF;

    vertex verts[5] = {
        {x1, y1, 0.0f, white},  // top-left
        {x2, y1, 0.0f, white},  // top-right
        {x2, y2, 0.0f, white},  // bottom-right
        {x1, y2, 0.0f, white},  // bottom-left
        {x1, y1, 0.0f, white}   // back to top-left to close the loop
    };

    gs_vertbuffer_t *vb = gs_vertexbuffer_create((gs_vb_data *)verts, 5);
    if (!vb)
        return;

    // Draw line strip (connected lines)
    gs_draw(GS_LINESTRIP, 0, 5);

    gs_vertexbuffer_destroy(vb);
}

bool draw_source_ui(obs_scene_t *scene, obs_sceneitem_t *item, void *param) {
      // Get the source and its bounds in the scene
    obs_source_t *source = obs_sceneitem_get_source(item);

    // Get the transform (position, scale, rotation)
    struct vec2 pos, scale;
    float rot;
    obs_sceneitem_get_pos(item, &pos);
    obs_sceneitem_get_scale(item, &scale);
    rot = obs_sceneitem_get_rot(item);

    // Get source size
    uint32_t width = obs_source_get_width(source);
    uint32_t height = obs_source_get_height(source);

    // Calculate box corners in scene space (ignoring rotation here for simplicity)
    float x1 = pos.x;
    float y1 = pos.y;
    float x2 = x1 + width * scale.x;
    float y2 = y1 + height * scale.y;

    // Draw a rectangle outline (using a helper function)
    draw_rectangle_outline(x1, y1, x2, y2);

    obs_source_release(source);
    return true;  // continue enumeration
}

void draw_callback(void* data, uint32_t cx, uint32_t cy) {
  // Initially, draw the OBS scene texture
  obs_render_main_texture();

  // // Set projection and viewport
  // gs_ortho(0.0f, float(cx), 0.0f, float(cy), -100.0f, 100.0f);
  // gs_set_viewport(0, 0, cx, cy);

  // // Solid effect
  // gs_effect_t *solid = obs_get_base_effect(OBS_EFFECT_SOLID);
  // gs_eparam_t *solid_color = gs_effect_get_param_by_name(solid, "color");
  // gs_technique_t *solid_tech = gs_effect_get_technique(solid, "Solid");

  // vec4 green = {0.0f, 1.0f, 0.0f, 1.0f};
  // gs_effect_set_vec4(solid_color, &green);

  // gs_technique_begin(solid_tech);
  // gs_technique_begin_pass(solid_tech, 0);

  // gs_matrix_push();
  // gs_matrix_identity();
  // gs_matrix_translate3f(200.0f, 200.0f, 50.0f); // Z = 50
  // gs_matrix_scale3f(100.0f, 100.0f, 1.0f);
  // gs_draw_sprite(nullptr, 0, 100, 100);
  // gs_matrix_pop();

  obs_scene_t* scene = obs_get_scene_by_name("WCR Scene");
  obs_scene_enum_items(scene, draw_source_ui, NULL);
  obs_scene_release(scene);

  // gs_technique_end_pass(solid_tech);
  // gs_technique_end(solid_tech);
}

void ObsInterface::initPreview(HWND parent) {
  blog(LOG_INFO, "ObsInterface::showPreview");

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

    obs_display_add_draw_callback(display, draw_callback, NULL);
  }

  obs_display_set_enabled(display, false);
}

void ObsInterface::showPreview(int x, int y, int width, int height) {
  blog(LOG_INFO, "ObsInterface::showPreview");

  if (!preview_hwnd || !display) {
    blog(LOG_ERROR, "Preview window not initialized");
    return;
  }

  // Resize and move the existing child window.
  bool success = SetWindowPos(
    preview_hwnd,                  // Handle to the child window
    NULL,                          // No Z-order change
    x, y,                          // New position (x, y)
    width, height,                 // New size (width, height)
    SWP_NOZORDER | SWP_NOACTIVATE  // Don't change position, Z-order, or activation
  );

  if (!success) {
    blog(LOG_ERROR, "Failed to resize preview window to (%d x %d)", width, height);
    return;
  }

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

  // Create the resources we rely on.
  create_output(recordingPath);
  create_scene();

  configure_video_encoder();
  configure_audio_encoder();
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

  if (output) {
    if (obs_output_active(output)) {
      blog(LOG_DEBUG, "Force stopping output");
      obs_output_force_stop(output);
    }
      
    blog(LOG_DEBUG, "Releasing output");
    obs_output_release(output);
  }

  if (video_encoder) {
    blog(LOG_DEBUG, "Releasing video encoder");
    obs_encoder_release(video_encoder);
  }

  if (audio_encoder) {
    blog(LOG_DEBUG, "Releasing audio encoder");
    obs_encoder_release(audio_encoder);
  }

  blog(LOG_DEBUG, "Now shutting down OBS");
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
  blog(LOG_INFO, "ObsInterface::startRecording enter");
  bool is_active = obs_output_active(output);

  if (!is_active) {
    blog(LOG_ERROR, "Buffer is not active");
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

   blog(LOG_INFO, "ObsInterface::startRecording exit");
}

void ObsInterface::stopRecording() {
  blog(LOG_INFO, "ObsInterface::stopRecording enter");
  bool is_active = obs_output_active(output);

  if (!is_active) {
    blog(LOG_WARNING, "Buffer is not active");
    return;
  }

  obs_output_stop(output);
  blog(LOG_INFO, "ObsInterface::stopRecording exited");
}

std::string ObsInterface::getLastRecording() {
  blog(LOG_INFO, "calling get last replay proc handler");
  calldata cd;
  calldata_init(&cd);
  proc_handler_t *ph = obs_output_get_proc_handler(output);
  bool success = proc_handler_call(ph, "get_last_replay", &cd);

  if (!success) {
    blog(LOG_ERROR, "Failed to call get_last_replay procedure handler");
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
  blog(LOG_INFO, "ObsInterface::getSourcePos called");

  obs_source_t *src = obs_get_source_by_name(name.c_str());
  obs_sceneitem_t *item = obs_scene_find_source(scene, name.c_str());

  if (!src) {
    blog(LOG_WARNING, "Did not find source for video source: %s", name);
    return;
  }

  if (!item) {
    blog(LOG_WARNING, "Did not find scene item for video source: %s", name);
    return;
  }

  obs_sceneitem_get_pos(item, pos);
  obs_sceneitem_get_scale(item, scale);

  // Pre-scaled sizes.
  size->x = obs_source_get_width(src);
  size->y = obs_source_get_height(src);

  blog(LOG_INFO, "ObsInterface::getSourcePos exited");
}

void ObsInterface::setSourcePos(std::string name, vec2* pos, vec2* scale) {
  blog(LOG_INFO, "ObsInterface::moveSource called");
  obs_sceneitem_t *item = obs_scene_find_source(scene, name.c_str());

  if (!item) {
    blog(LOG_WARNING, "Did not find scene item for video source: %s", name);
    return;
  }

  obs_sceneitem_set_pos(item, pos);
  obs_sceneitem_set_scale(item, scale);
}