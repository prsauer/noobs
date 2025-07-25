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

Napi::Value ObsInit(const Napi::CallbackInfo& info) {
  bool valid = info.Length() == 5 &&
   info[0].IsString() && // Plugin path
   info[1].IsString() && // Log path
   info[2].IsString() && // Data path
   info[3].IsString() && // Recording path
   info[4].IsFunction(); // JavaScript callback

  if (!valid) {
    Napi::Error::New(info.Env(), "Invalid arguments passed to ObsInit").ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  std::string pluginPath = info[0].As<Napi::String>().Utf8Value();
  std::string logPath = info[1].As<Napi::String>().Utf8Value();
  std::string dataPath = info[2].As<Napi::String>().Utf8Value();
  std::string recordingPath = info[3].As<Napi::String>().Utf8Value();
  Napi::Function fn = info[4].As<Napi::Function>();

  Napi::ThreadSafeFunction jscb =
    Napi::ThreadSafeFunction::New(info.Env(), fn, "JavaScript callback", 0, 1);

  obs = new ObsInterface(pluginPath, logPath, dataPath, recordingPath, jscb);
  return info.Env().Undefined();
}

Napi::Value ObsShutdown(const Napi::CallbackInfo& info) {
  delete obs;
  obs = nullptr;
  return info.Env().Undefined();
}

Napi::Value ObsStartBuffer(const Napi::CallbackInfo& info) {
  blog(LOG_INFO, "ObsStartBuffer called");

  if (!obs) {
    blog(LOG_ERROR, "ObsStartBuffer called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  obs->startBuffering();
  return info.Env().Undefined();
}

Napi::Value ObsStartRecording(const Napi::CallbackInfo& info) {
  if (!obs) {
    blog(LOG_ERROR, "ObsStartRecording called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  int offset = 0;

  if (info.Length() == 1 && info[0].IsNumber()) {
    offset = info[0].As<Napi::Number>().Int32Value();
  }

  obs->startRecording(offset);
  return info.Env().Undefined();
}

Napi::Value ObsStopRecording(const Napi::CallbackInfo& info) {
  if (!obs) {
    blog(LOG_ERROR, "ObsStopRecording called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  obs->stopRecording();
  return info.Env().Undefined();
}

Napi::Value ObsGetLastRecording(const Napi::CallbackInfo& info) {
  if (!obs) {
    blog(LOG_ERROR, "ObsGetLastRecording called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  std::string lastRecording = obs->getLastRecording();
  return Napi::String::New(info.Env(), lastRecording);
}

Napi::Value ObsInitPreview(const Napi::CallbackInfo& info) {
  blog(LOG_INFO, "ObsInitPreview called");

  if (!obs) {
    blog(LOG_ERROR, "ObsInitPreview called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  bool valid = info.Length() == 1 && info[0].IsBuffer();

  if (!valid) {
    Napi::TypeError::New(info.Env(), "Invalid arguments passed to ObsInitPreview").ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  Napi::Buffer<uint8_t> buffer = info[0].As<Napi::Buffer<uint8_t>>();

  if (buffer.Length() < sizeof(HWND)) {
    Napi::TypeError::New(info.Env(), "Buffer too small for HWND").ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  HWND hwnd = *reinterpret_cast<HWND*>(buffer.Data());
  obs->initPreview(hwnd);
  return info.Env().Undefined();
}

Napi::Value ObsShowPreview(const Napi::CallbackInfo& info) {
  blog(LOG_INFO, "ObsShowPreview called");

  if (!obs) {
    blog(LOG_ERROR, "ObsShowPreview called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  bool valid = info.Length() == 4 &&
    info[1].IsNumber() && // X
    info[2].IsNumber() && // Y
    info[3].IsNumber() && // Width
    info[4].IsNumber(); // Height

  if (!valid) {
    Napi::TypeError::New(info.Env(), "Invalid arguments passed to ObsShowPreview").ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  int x = info[0].As<Napi::Number>().Int32Value();
  int y = info[1].As<Napi::Number>().Int32Value();
  int width = info[2].As<Napi::Number>().Int32Value();
  int height = info[3].As<Napi::Number>().Int32Value();

  obs->showPreview(x, y, width, height);
  return info.Env().Undefined();
}

Napi::Value ObsHidePreview(const Napi::CallbackInfo& info) {
  if (!obs) {
    blog(LOG_ERROR, "ObsHidePreview called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  obs->hidePreview();
  return info.Env().Undefined();
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("ObsInit", Napi::Function::New(env, ObsInit));
  exports.Set("ObsShutdown", Napi::Function::New(env, ObsShutdown));

  exports.Set("ObsStartBuffer", Napi::Function::New(env, ObsStartBuffer));
  exports.Set("ObsStartRecording", Napi::Function::New(env, ObsStartRecording));
  exports.Set("ObsStopRecording", Napi::Function::New(env, ObsStopRecording));
  exports.Set("ObsGetLastRecording", Napi::Function::New(env, ObsGetLastRecording));

  exports.Set("ObsInitPreview", Napi::Function::New(env, ObsInitPreview));
  exports.Set("ObsShowPreview", Napi::Function::New(env, ObsShowPreview));
  exports.Set("ObsHidePreview", Napi::Function::New(env, ObsHidePreview));
  return exports;
}

NODE_API_MODULE(addon, Init)