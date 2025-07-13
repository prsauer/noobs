#include <napi.h>
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <obs.h>

Napi::Number GetUptime(const Napi::CallbackInfo& info) {
  DWORD ticks = GetTickCount();
  return Napi::Number::New(info.Env(), static_cast<double>(ticks));
}

Napi::Value StartOBS(const Napi::CallbackInfo& info) {
  obs_startup("en-US", "obs", NULL);
  return info.Env().Undefined();
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
  exports.Set("listProcesses", Napi::Function::New(env, ListProcesses));
  return exports;
}

NODE_API_MODULE(addon, Init)