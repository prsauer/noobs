#include <napi.h>
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <obs.h>
#include <iostream>
#include <future>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


void WindowThread(std::promise<HWND> hwndPromise) {
    const wchar_t CLASS_NAME[] = L"MyWindowClass";
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"My Window",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        nullptr, nullptr, hInstance, nullptr
    );

    hwndPromise.set_value(hwnd);  // Send hwnd back to main thread
    ShowWindow(hwnd, SW_SHOW);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void load_module(const char* module) {
    obs_module_t *ptr = NULL;
    int success = obs_open_module(&ptr, module, "D:/checkouts/warcraft-recorder-obs-engine/obs-plugins/64bit/");
    std::cout << "Loading module: "  << module << std::endl;

    if (success != MODULE_SUCCESS) {
        std::cerr << "Failed to open module!" << std::endl;
    } else {
        std::cout << "Module opened successfully!" << std::endl;
    }

    bool initmod = obs_init_module(ptr);

    if (!initmod) {
        std::cerr << "Module initialization failed!" << std::endl;
    } else {
        std::cout << "Module initialized successfully!" << std::endl;
    }
}

void draw_callback(void* data, uint32_t cx, uint32_t cy) {
    // Render the OBS preview scene here
    obs_render_main_texture();
}

void output_signal_handler(void *data, calldata_t *cd) {
    std::cout << "\n=== OUTPUT SIGNAL ===" << std::endl;
    
    // Try to get common signal parameters
    obs_output_t *output = (obs_output_t*)calldata_ptr(cd, "output");
    const char *signal_name = calldata_string(cd, "signal");
    long long code = calldata_int(cd, "code");
    const char *error = calldata_string(cd, "error");
    const char *path = calldata_string(cd, "path");
    
    std::cout << "signal: " << (signal_name ? signal_name : "unknown") << std::endl;
    std::cout << "Output: " << output << std::endl;
    std::cout << "Code: " << code << std::endl;
    
    if (error) std::cout << "Error: " << error << std::endl;
    if (path) std::cout << "Path: " << path << std::endl;
    
    std::cout << "===================\n" << std::endl;
}

static void listEncoders(obs_encoder_type type)
{
	constexpr uint32_t hide_flags = OBS_ENCODER_CAP_DEPRECATED | OBS_ENCODER_CAP_INTERNAL;

	size_t idx = 0;
	const char *encoder_type;

	while (obs_enum_encoder_types(idx++, &encoder_type)) {
		if (obs_get_encoder_caps(encoder_type) & hide_flags || obs_get_encoder_type(encoder_type) != type) {
			continue;
		}

		blog(LOG_INFO, "\t- %s (%s)", encoder_type, obs_encoder_get_display_name(encoder_type));
	}
};

static void listSourceTypes()
{
  size_t idx = 0;
  const char *src = nullptr;

  while (obs_enum_source_types(idx++, &src)) {
      blog(LOG_INFO, "\t- %s", src);
  }
}

static void listInputTypes()
{
  size_t idx = 0;
  const char *src = nullptr;

  while (obs_enum_input_types(idx++, &src)) {
     blog(LOG_INFO, "\t- %s", src);
  }
}

static void listOutputTypes()
{
  size_t idx = 0;
  const char *src = nullptr;

  while (obs_enum_output_types(idx++, &src)) {
      blog(LOG_INFO, "\t- %s", src);
  }
}

Napi::Value StartOBS(const Napi::CallbackInfo& info) {
  std::cout << "Starting..." << std::endl;
  obs_startup("en-US", NULL, NULL);
  std::cout << "OBS has started!" << std::endl;

  obs_add_data_path("D:/checkouts/warcraft-recorder-obs-engine/effects/");
  obs_add_data_path("D:/checkouts/warcraft-recorder-obs-engine/effects/libobs");

  load_module("D:/checkouts/warcraft-recorder-obs-engine/obs-plugins/64bit/obs-x264.dll");
  load_module("D:/checkouts/warcraft-recorder-obs-engine/obs-plugins/64bit/obs-ffmpeg.dll");
  load_module("D:/checkouts/cpp-httplib/plugins/64bit/win-capture.dll");

  // obs-ffmpeg-nvenc.c
  // obs-qsv11
  // AMD?

  bool init = obs_initialized();

    if (!init) {
      std::cerr << "OBS initialization failed!" << std::endl;
    } else {
      std::cout << "OBS is initialized!" << std::endl;
    }

    const char* version = obs_get_version_string();
    std::cout << "OBS version is: " << version <<  std::endl;

    const char* n = "obs_x264";
    const char * encoder = obs_get_encoder_codec(n);

    if (encoder == nullptr) {
      std::cerr << "Failed to get encoder codec!" << std::endl;
    } else {
      std::cout << "Codec is: " << encoder <<  std::endl;
    }

    
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

    int reset = obs_reset_video(&ovi);

    if (reset != 0) {
        std::cerr << "Failed to reset video!" << reset << std::endl;
    } else {
        std::cout << "Video reset successfully!" << std::endl;
    }
    struct obs_video_info *ovi2 = (struct obs_video_info *)malloc(sizeof(struct obs_video_info));
    bool testvideosettings = obs_get_video_info(ovi2);

    if (testvideosettings) {
      std::cout << "Got video settings" << std::endl;
    } else {
      std::cerr << "Failed to get video settings!" << std::endl;
    }

    
    struct obs_audio_info aoi = {0};
    aoi.samples_per_sec = 48000;
    aoi.speakers = SPEAKERS_STEREO;
    reset = obs_reset_audio(&aoi);

    if (!reset) {
        std::cerr << "Failed to reset audio!" << std::endl;
    } else {
        std::cout << "Audio reset successfully!" << std::endl;
    }

    std::cout << "Create output" << std::endl;
    obs_output_t *output = obs_output_create("ffmpeg_output", "simple_output", NULL, NULL);
    std::cout << "Created output" << std::endl;

    std::cout << "Set output settings" << std::endl;
    obs_data_t *settings = obs_data_create();
    obs_data_set_string(settings, "url", "D:/checkouts/warcraft-recorder-obs-engine/recording.mp4");  // Use "url" not "path"
    obs_data_set_string(settings, "format_name", "mp4");
    obs_output_update(output, settings);
    obs_data_release(settings);

    std::cout << "Create venc" << std::endl;
    obs_encoder_t *venc = obs_video_encoder_create("obs_x264", "simple_h264_stream", NULL, NULL);

    std::cout << "Set video encoder settings" << std::endl;
    obs_data_t *venc_settings = obs_data_create();
    obs_data_set_string(venc_settings, "preset", "fast");
    obs_data_set_int(venc_settings, "bitrate", 2500);
    obs_data_set_string(venc_settings, "profile", "main");
    obs_data_set_string(venc_settings, "rate_control", "CBR");
    obs_encoder_update(venc, venc_settings);
    obs_data_release(venc_settings);
    obs_output_set_video_encoder(output, venc);


    std::cout << "Create aenc" << std::endl;
    obs_encoder_t *aenc = obs_audio_encoder_create("ffmpeg_aac", "simple_aac", NULL, 0, NULL);
    std::cout << "Set audio encoder settings" << std::endl;
    obs_data_t *aenc_settings = obs_data_create();
    obs_data_set_int(aenc_settings, "bitrate", 128);
    obs_encoder_update(aenc, aenc_settings);
    obs_data_release(aenc_settings);

    obs_output_set_audio_encoder(output, aenc, 0);
    
    std::cout << "Create scene and src" << std::endl;
    obs_scene_t *scene = obs_scene_create("Scene");
  
    std::cout << "Create display capture source" << std::endl;
    // Create settings for monitor capture
    obs_data_t *monitor_settings = obs_data_create();
    obs_data_set_int(monitor_settings, "monitor", 0);  // Monitor 0
    obs_data_set_bool(monitor_settings, "capture_cursor", true);

    obs_source_t *source = obs_source_create("monitor_capture", "Monitor", monitor_settings, NULL);
    obs_data_release(monitor_settings);

    if (source) {
      obs_scene_add(scene, source);
      std::cout << "Added display capture to scene" << std::endl;
    } else {
        std::cerr << "Failed to create display capture source!" << std::endl;
    }

    obs_source_t *scene_source = obs_scene_get_source(scene);
    obs_set_output_source(0, scene_source);  // 0 = video track

    std::cout << "Get video" << std::endl;
    video_t *video = obs_get_video();
    std::cout << "Got video" << std::endl;
    obs_encoder_set_video(venc, obs_get_video());
    obs_encoder_set_audio(aenc, obs_get_audio());



    signal_handler_t *sh = obs_output_get_signal_handler(output);
    signal_handler_connect(sh, "starting", output_signal_handler, NULL);
    signal_handler_connect(sh, "start", output_signal_handler, NULL);
    signal_handler_connect(sh, "stop", output_signal_handler, NULL);

    std::cout << "List  a encoders" << std::endl;
    listEncoders(OBS_ENCODER_AUDIO);
    std::cout << "List  v encoders" << std::endl;
    listEncoders(OBS_ENCODER_VIDEO);

    std::cout << "List  src types" << std::endl;
    listSourceTypes();
    std::cout << "List  input types" << std::endl;
    listInputTypes();
    std::cout << "List  output types" << std::endl;
    listOutputTypes();

    std::promise<HWND> hwndPromise;
    std::future<HWND> hwndFuture = hwndPromise.get_future();

    std::thread winThread(WindowThread, std::move(hwndPromise));
    HWND hwnd = hwndFuture.get();  // blocks until hwnd is ready

    gs_init_data gs_data = {};

    gs_data.adapter = 0;
    gs_data.cx = 1920;  // Window width
    gs_data.cy = 1080;  // Window height
    gs_data.format = GS_BGRA;
    gs_data.zsformat = GS_ZS_NONE;
    gs_data.num_backbuffers = 1;
    gs_data.window.hwnd = hwnd;

    obs_display_t* display = nullptr;
    display = obs_display_create(&gs_data, 0x0);
    obs_display_add_draw_callback(display, draw_callback, NULL);
    

    std::cout << "Start rec" << std::endl;
    bool started = obs_output_start(output);

    if (started) {
        std::cout << "Recording started successfully!" << std::endl;
    } else {
        std::cerr << "Failed to start recording!" << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5000)); 
    obs_output_stop(output);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000)); 


    std::cout << "END FN" << std::endl;

     winThread.join();  // or detach() if you don’t care about waiting
  return info.Env().Undefined();
}
















Napi::Number GetUptime(const Napi::CallbackInfo& info) {
  DWORD ticks = GetTickCount();
  return Napi::Number::New(info.Env(), static_cast<double>(ticks));
}

class ProcessListWorker : public Napi::AsyncWorker {
public:
    ProcessListWorker(Napi::Function& callback) 
        : Napi::AsyncWorker(callback) {}

    ~ProcessListWorker() {}

    // This runs on a background thread - won't block JS
    void Execute() override {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) {
            SetError("Failed to create process snapshot");
            return;
        }
        
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        
        if (!Process32FirstW(snapshot, &pe32)) {
            CloseHandle(snapshot);
            SetError("Failed to get first process");
            return;
        }
        
        do {
            ProcessInfo info;
            
            // Convert wide string to UTF-8
            int utf8Length = WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, nullptr, 0, nullptr, nullptr);
            if (utf8Length > 0) {
                std::vector<char> utf8Buffer(utf8Length);
                WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, utf8Buffer.data(), utf8Length, nullptr, nullptr);
                
                info.name = std::string(utf8Buffer.data());
                info.pid = pe32.th32ProcessID;
                info.parentPid = pe32.th32ParentProcessID;
                info.threads = pe32.cntThreads;
                
                processes.push_back(info);
            }
        } while (Process32NextW(snapshot, &pe32));
        
        CloseHandle(snapshot);
    }

    // This runs back on the main thread
    void OnOK() override {
        Napi::HandleScope scope(Env());
        Napi::Array result = Napi::Array::New(Env());
        
        for (size_t i = 0; i < processes.size(); i++) {
            Napi::Object process = Napi::Object::New(Env());
            process.Set("name", Napi::String::New(Env(), processes[i].name));
            process.Set("pid", Napi::Number::New(Env(), processes[i].pid));
            process.Set("parentPid", Napi::Number::New(Env(), processes[i].parentPid));
            process.Set("threads", Napi::Number::New(Env(), processes[i].threads));
            result.Set(i, process);
        }
        
        Callback().Call({Env().Null(), result});
    }

    void OnError(const Napi::Error& e) override {
        Napi::HandleScope scope(Env());
        Callback().Call({e.Value(), Env().Undefined()});
    }

private:
    struct ProcessInfo {
        std::string name;
        DWORD pid;
        DWORD parentPid;
        DWORD threads;
    };
    
    std::vector<ProcessInfo> processes;
};

Napi::Value ListProcesses(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    auto deferred = Napi::Promise::Deferred::New(env);
    
    auto callback = Napi::Function::New(env, [deferred](const Napi::CallbackInfo& info) {
        if (!info[0].IsNull()) {
            // Error case
            deferred.Reject(info[0]);
        } else {
            // Success case
            deferred.Resolve(info[1]);
        }
        return info.Env().Undefined();
    });
    
    ProcessListWorker* worker = new ProcessListWorker(callback);
    worker->Queue();
    
    return deferred.Promise();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("getUptime", Napi::Function::New(env, GetUptime));
  exports.Set("StartOBS", Napi::Function::New(env, StartOBS));
  exports.Set("listProcesses", Napi::Function::New(env, ListProcesses));
  return exports;
}

NODE_API_MODULE(addon, Init)