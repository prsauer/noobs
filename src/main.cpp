#include <napi.h>
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <obs.h>
#include <iostream>
#include <future>
#include <chrono>

#include "obs_interface.h"

ObsInterface* obs = nullptr;

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

Napi::Value ObsInit(const Napi::CallbackInfo& info) {
  obs = new ObsInterface();

  // std::promise<HWND> hwndPromise;
  // std::future<HWND> hwndFuture = hwndPromise.get_future();

  // std::thread winThread(WindowThread, std::move(hwndPromise));
  // HWND hwnd = hwndFuture.get();  // blocks until hwnd is ready

  // gs_init_data gs_data = {};

  // gs_data.adapter = 0;
  // gs_data.cx = 1920;  // Window width
  // gs_data.cy = 1080;  // Window height
  // gs_data.format = GS_BGRA;
  // gs_data.zsformat = GS_ZS_NONE;
  // gs_data.num_backbuffers = 1;
  // gs_data.window.hwnd = hwnd;

  // obs_display_t* display = nullptr;
  // display = obs_display_create(&gs_data, 0x0);
  // obs_display_add_draw_callback(display, draw_callback, NULL);
  

  // std::cout << "Start rec" << std::endl;
  // bool started = obs_output_start(output);

  // if (started) {
  //   std::cout << "Recording started successfully!" << std::endl;
  // } else {
  //   std::cerr << "Failed to start recording!" << std::endl;
  // }

  // std::this_thread::sleep_for(std::chrono::milliseconds(5000)); 

  // std::cout << "calling save proc handler" << std::endl;
  // calldata cd;
  // calldata_init(&cd);
  // calldata_set_int(&cd, "offset_seconds", 3);
  // proc_handler_t *ph = obs_output_get_proc_handler(output);
  // proc_handler_call(ph, "convert", &cd);
  // calldata_free(&cd);
  // std::this_thread::sleep_for(std::chrono::milliseconds(5000)); 


  // obs_output_stop(output);
  // std::this_thread::sleep_for(std::chrono::milliseconds(5000)); 

  // // Leaves the last frame present on the screen
  // // obs_display_remove_draw_callback(display, draw_callback, NULL);
  // // obs_display_destroy(display);

  // std::cout << "END FN" << std::endl;

  // winThread.detach();  // or detach() if you don’t care about waiting
  return info.Env().Undefined();
}


Napi::Value ObsShutdown(const Napi::CallbackInfo& info) {
  delete obs;
  obs = nullptr;
  return info.Env().Undefined();
}

Napi::Value ObsStartBuffer(const Napi::CallbackInfo& info) {
  if (!obs) 
    throw std::runtime_error("Obs not initialized");

  obs->startBuffering();
  return info.Env().Undefined();
}

Napi::Value ObsStartRecording(const Napi::CallbackInfo& info) {
  if (!obs) 
    throw std::runtime_error("Obs not initialized");

  obs->startRecording(2);
  return info.Env().Undefined();
}

Napi::Value ObsStopRecording(const Napi::CallbackInfo& info) {
  if (!obs) 
    throw std::runtime_error("Obs not initialized");

  obs->stopRecording();
  return info.Env().Undefined();
}

Napi::Value ObsShowPreview(const Napi::CallbackInfo& info) {
  blog(LOG_INFO, "ObsShowPreview called");

  if (!obs) 
    throw std::runtime_error("Obs not initialized");

  if (info.Length() < 1 || !info[0].IsBuffer()) {
    Napi::TypeError::New(info.Env(), "Expected HWND as Buffer").ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  // Get HWND from JavaScript (passed as a number)

  // Handle Buffer (from Electron's getNativeWindowHandle())
  Napi::Buffer<uint8_t> buffer = info[0].As<Napi::Buffer<uint8_t>>();

  if (buffer.Length() < sizeof(HWND)) {
    Napi::TypeError::New(info.Env(), "Buffer too small for HWND").ThrowAsJavaScriptException();
    return info.Env().Undefined();
  } else {
    blog(LOG_INFO, "ObsShowPreview buffer length: %zu", buffer.Length());
    blog(LOG_INFO, "ObsShowPreview hwnd length: %zu", sizeof(HWND));
  }

  // Union to safely read the bytes
  union {
    uint8_t bytes[8];
    uint64_t value;
    HWND hwnd;
  } hwndUnion;
  
  memcpy(hwndUnion.bytes, buffer.Data(), sizeof(hwndUnion.bytes));

  std::cout << "Received HWND: " << hwndUnion.hwnd << " (0x" << std::hex << buffer << std::dec << ")" << std::endl;
  std::cout << "Received HWND: " << hwndUnion.hwnd << " (0x" << std::hex << hwndUnion.value << std::dec << ")" << std::endl;

  obs->showPreview(hwndUnion.hwnd);
  return info.Env().Undefined();
}

Napi::Value ObsResizePreview(const Napi::CallbackInfo& info) {
  if (!obs) 
    throw std::runtime_error("Obs not initialized");

  if (info.Length() < 2) {
    Napi::TypeError::New(info.Env(), "Expected width and height").ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  int width = info[0].As<Napi::Number>().Int32Value();
  int height = info[1].As<Napi::Number>().Int32Value();

  obs->resizePreview(width, height);
  return info.Env().Undefined();
}

Napi::Value ObsMovePreview(const Napi::CallbackInfo& info) {
  if (!obs) 
    throw std::runtime_error("Obs not initialized");

  if (info.Length() < 2) {
    Napi::TypeError::New(info.Env(), "Expected x and y").ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  int x = info[0].As<Napi::Number>().Int32Value();
  int y = info[1].As<Napi::Number>().Int32Value();

  obs->movePreview(x, y);
  return info.Env().Undefined();
}

Napi::Value ObsHidePreview(const Napi::CallbackInfo& info) {
  if (!obs) 
    throw std::runtime_error("Obs not initialized");

  obs->hidePreview();
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
  exports.Set("ObsInit", Napi::Function::New(env, ObsInit));
  exports.Set("ObsShutdown", Napi::Function::New(env, ObsShutdown));
  exports.Set("ObsStartBuffer", Napi::Function::New(env, ObsStartBuffer));
  exports.Set("ObsStartRecording", Napi::Function::New(env, ObsStartRecording));
  exports.Set("ObsStopRecording", Napi::Function::New(env, ObsStopRecording));

  // Add preview functions
  exports.Set("ObsShowPreview", Napi::Function::New(env, ObsShowPreview));
  exports.Set("ObsResizePreview", Napi::Function::New(env, ObsResizePreview));
  exports.Set("ObsHidePreview", Napi::Function::New(env, ObsHidePreview));

  exports.Set("getUptime", Napi::Function::New(env, GetUptime));
  exports.Set("listProcesses", Napi::Function::New(env, ListProcesses));
  return exports;
}

NODE_API_MODULE(addon, Init)