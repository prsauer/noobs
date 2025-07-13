#include <napi.h>
#include <windows.h>

Napi::Number GetUptime(const Napi::CallbackInfo& info) {
  DWORD ticks = GetTickCount();
  return Napi::Number::New(info.Env(), static_cast<double>(ticks));
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("getUptime", Napi::Function::New(env, GetUptime));
  return exports;
}

NODE_API_MODULE(addon, Init)