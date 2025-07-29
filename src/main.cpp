#include <napi.h>
#include <windows.h>
#include <obs.h>
#include "obs_interface.h"
#include "utils.h"

ObsInterface* obs = nullptr;

Napi::Value ObsInit(const Napi::CallbackInfo& info) {
  bool valid = info.Length() == 5 &&
   info[0].IsString() &&   // Plugin path
   info[1].IsString() &&   // Log path
   info[2].IsString() &&   // Data path
   info[3].IsString() &&   // Recording path
   info[4].IsFunction();   // JavaScript callback

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

Napi::Value ObsSetRecordingDir(const Napi::CallbackInfo& info) {
  if (!obs) {
    blog(LOG_ERROR, "ObsSetRecordingDir called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  bool valid = info.Length() == 1 && info[0].IsString();

  if (!valid) {
    Napi::TypeError::New(info.Env(), "Invalid arguments passed to ObsSetRecordingDir").ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  std::string recordingPath = info[0].As<Napi::String>().Utf8Value();
  obs->setRecordingDir(recordingPath);
  return info.Env().Undefined();
}

Napi::Value ObsSetBuffering(const Napi::CallbackInfo& info) {
  blog(LOG_INFO, "ObsSetBuffering called");

  if (!obs) {
    blog(LOG_ERROR, "ObsSetBuffering called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  bool valid = info.Length() == 1 && info[0].IsBoolean();

  if (!valid) {
    Napi::TypeError::New(info.Env(), "Invalid arguments passed to ObsSetBuffering").ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  bool buffering = info[0].As<Napi::Boolean>().Value();
  bool success = obs->setBuffering(buffering);

  if (!success) {
    Napi::Error::New(info.Env(), "Failed to set buffering mode, is recording active?").ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

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
    info[0].IsNumber() && // X
    info[1].IsNumber() && // Y
    info[2].IsNumber() && // Width
    info[3].IsNumber(); // Height

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

Napi::Value ObsGetPreviewScaleFactor(const Napi::CallbackInfo& info) {
  if (!obs) {
    blog(LOG_ERROR, "ObsGetPreviewScaleFactor called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  float scaleFactor = obs->getPreviewScaleFactor();
  return Napi::Number::New(info.Env(), scaleFactor);
}

Napi::Value ObsCreateSource(const Napi::CallbackInfo& info) {
  if (!obs) {
    blog(LOG_ERROR, "ObsCreateSource called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  bool valid = info.Length() == 2 &&
   info[0].IsString() && // Source name
   info[1].IsString();   // Source type

  if (!valid) {
    Napi::TypeError::New(info.Env(), "Invalid arguments passed to ObsCreateSource").ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  std::string name = info[0].As<Napi::String>().Utf8Value();
  std::string type = info[1].As<Napi::String>().Utf8Value();

  obs->createSource(name, type);
  return info.Env().Undefined();
}

Napi::Value ObsDeleteSource(const Napi::CallbackInfo& info) {
  if (!obs) {
    blog(LOG_ERROR, "ObsDeleteSource called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  bool valid = info.Length() == 1 && info[0].IsString();

  if (!valid) {
    Napi::TypeError::New(info.Env(), "Invalid arguments passed to ObsDeleteSource").ThrowAsJavaScriptException();
    return info.Env().Undefined();  
  }

  std::string name = info[0].As<Napi::String>().Utf8Value();
  obs->deleteSource(name);
  return info.Env().Undefined();
}

Napi::Value ObsGetSourceSettings(const Napi::CallbackInfo& info) {
  if (!obs) {
    blog(LOG_ERROR, "ObsGetSourceSettings called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  bool valid = info.Length() == 1 && info[0].IsString();

  if (!valid) {
    Napi::TypeError::New(info.Env(), "Invalid arguments passed to ObsGetSourceSettings").ThrowAsJavaScriptException();
    return info.Env().Undefined();  
  }

  std::string name = info[0].As<Napi::String>().Utf8Value();

  obs_data_t* settings = obs->getSourceSettings(name);
  Napi::Object result = data_to_napi(info.Env(), settings);
  obs_data_release(settings);

  return result;
}

Napi::Value ObsSetSourceSettings(const Napi::CallbackInfo& info) {
  if (!obs) {
    blog(LOG_ERROR, "ObsSetSourceSettings called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  bool valid = info.Length() == 2 && info[0].IsString() && info[1].IsObject();

  if (!valid) {
    Napi::TypeError::New(info.Env(), "Invalid arguments passed to ObsSetSourceSettings").ThrowAsJavaScriptException();
    return info.Env().Undefined();  
  }

  std::string name = info[0].As<Napi::String>().Utf8Value();

  Napi::Object obj = info[1].As<Napi::Object>();
  obs_data_t* settings = napi_to_data(obj);
  obs->setSourceSettings(name, settings);
  obs_data_release(settings);

  return info.Env().Undefined();
}

Napi::Value ObsGetSourceProperties(const Napi::CallbackInfo& info) {
  if (!obs) {
    blog(LOG_ERROR, "ObsGetSourceProperties called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  bool valid = info.Length() == 1 && info[0].IsString();

  if (!valid) {
    Napi::TypeError::New(info.Env(), "Invalid arguments passed to ObsGetSourceProperties").ThrowAsJavaScriptException();
    return info.Env().Undefined();  
  }

  std::string name = info[0].As<Napi::String>().Utf8Value();
  obs_properties_t* properties = obs->getSourceProperties(name);
  Napi::Object result = properties_to_napi(info.Env(), properties);
  obs_properties_destroy(properties);

  return result;
}

Napi::Value ObsCreateScene(const Napi::CallbackInfo& info) {
  if (!obs) {
    blog(LOG_ERROR, "ObsCreateScene called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  bool valid = info.Length() == 2 &&
   info[0].IsString() && // Scene name
   info[1].IsString();   // Source type

  if (!valid) {
    Napi::TypeError::New(info.Env(), "Invalid arguments passed to ObsCreateScene").ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  std::string name = info[0].As<Napi::String>().Utf8Value();
  std::string type = info[1].As<Napi::String>().Utf8Value();

  obs->createSource(name, type);
  return info.Env().Undefined();
}

Napi::Value ObsAddSourceToScene(const Napi::CallbackInfo& info) {
  if (!obs) {
    blog(LOG_ERROR, "ObsAddSourceToScene called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  bool valid = info.Length() == 1 && info[0].IsString();

  if (!valid) {
    Napi::TypeError::New(info.Env(), "Invalid arguments passed to ObsAddSourceToScene").ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  std::string name = info[0].As<Napi::String>().Utf8Value();

  obs->addSourceToScene(name);
  return info.Env().Undefined();
}

Napi::Value ObsRemoveSourceFromScene(const Napi::CallbackInfo& info) {
  if (!obs) {
    blog(LOG_ERROR, "ObsRemoveSourceFromScene called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  bool valid = info.Length() == 1 && info[0].IsString();

  if (!valid) {
    Napi::TypeError::New(info.Env(), "Invalid arguments passed to ObsRemoveSourceFromScene").ThrowAsJavaScriptException();
    return info.Env().Undefined();  
  }

  std::string name = info[0].As<Napi::String>().Utf8Value();
  obs->removeSourceFromScene(name);
  return info.Env().Undefined();
}

Napi::Value ObsGetSourcePos(const Napi::CallbackInfo& info) {
  if (!obs) {
    blog(LOG_ERROR, "ObsGetSourcePos called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  bool valid = info.Length() == 1 && info[0].IsString();

  if (!valid) {
    Napi::TypeError::New(info.Env(), "Invalid arguments passed to ObsGetSourcePos").ThrowAsJavaScriptException();
    return info.Env().Undefined();  
  }

  std::string name = info[0].As<Napi::String>().Utf8Value();

  vec2 pos; vec2 size; vec2 scale;
  obs->getSourcePos(name, &pos, &size, &scale);

  Napi::Object result = Napi::Object::New(info.Env());
  result.Set("x", Napi::Number::New(info.Env(), pos.x));
  result.Set("y", Napi::Number::New(info.Env(), pos.y));
  result.Set("width", Napi::Number::New(info.Env(), size.x));
  result.Set("height", Napi::Number::New(info.Env(), size.y));
  result.Set("scaleX", Napi::Number::New(info.Env(), scale.x));
  result.Set("scaleY", Napi::Number::New(info.Env(), scale.y));
  return result;
}

Napi::Value ObsSetSourcePos(const Napi::CallbackInfo& info) {
  if (!obs) {
    blog(LOG_ERROR, "ObsSetSourcePos called but obs is not initialized");
    throw std::runtime_error("Obs not initialized");
  }

  bool valid = info.Length() == 2 &&
    info[0].IsString() && // Source name
    info[1].IsObject();   // Position definition.

  if (!valid) {
    Napi::TypeError::New(info.Env(), "Invalid arguments passed to ObsSetSourcePos").ThrowAsJavaScriptException();
    return info.Env().Undefined();  
  }

  std::string name = info[0].As<Napi::String>().Utf8Value();

  Napi::Object position = info[1].As<Napi::Object>();
  float x = position.Get("x").As<Napi::Number>().FloatValue();
  float y = position.Get("y").As<Napi::Number>().FloatValue();
  vec2 pos = { x, y };

  float scaleX = position.Get("scaleX").As<Napi::Number>().FloatValue();
  float scaleY = position.Get("scaleY").As<Napi::Number>().FloatValue();
  vec2 scale = { scaleX, scaleY };

  obs->setSourcePos(name, &pos, &scale);
  return info.Env().Undefined();
}

Napi::Value ObsSetDrawSourceOutline(const Napi::CallbackInfo& info) {
  bool valid =  info.Length() == 1 && info[0].IsBoolean();
    if (!valid) {
    Napi::TypeError::New(info.Env(), "Invalid arguments passed to ObsSetDrawSourceOutline").ThrowAsJavaScriptException();
    return info.Env().Undefined();  
  }
  bool enabled = info[0].As<Napi::Boolean>();
  blog(LOG_DEBUG, "ObsSetDrawSourceOutline set to %d", enabled);
  obs->setDrawSourceOutline(enabled);
}

Napi::Value ObsGetDrawSourceOutlineEnabled(const Napi::CallbackInfo& info) {
  return Napi::Boolean::New(info.Env(), obs->getDrawSourceOutlineEnabled());
}


Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("Init", Napi::Function::New(env, ObsInit));
  exports.Set("Shutdown", Napi::Function::New(env, ObsShutdown));
  exports.Set("SetRecordingDir", Napi::Function::New(env, ObsSetRecordingDir));

  exports.Set("SetBuffering", Napi::Function::New(env, ObsSetBuffering));
  exports.Set("StartBuffer", Napi::Function::New(env, ObsStartBuffer));
  exports.Set("StartRecording", Napi::Function::New(env, ObsStartRecording));
  exports.Set("StopRecording", Napi::Function::New(env, ObsStopRecording));
  exports.Set("GetLastRecording", Napi::Function::New(env, ObsGetLastRecording));

  exports.Set("CreateSource", Napi::Function::New(env, ObsCreateSource));
  exports.Set("DeleteSource", Napi::Function::New(env, ObsDeleteSource));
  exports.Set("GetSourceSettings", Napi::Function::New(env, ObsGetSourceSettings));
  exports.Set("SetSourceSettings", Napi::Function::New(env, ObsSetSourceSettings));
  exports.Set("GetSourceProperties", Napi::Function::New(env, ObsGetSourceProperties));

  exports.Set("AddSourceToScene", Napi::Function::New(env, ObsAddSourceToScene));
  exports.Set("RemoveSourceFromScene", Napi::Function::New(env, ObsRemoveSourceFromScene));
  exports.Set("GetSourcePos", Napi::Function::New(env, ObsGetSourcePos));
  exports.Set("SetSourcePos", Napi::Function::New(env, ObsSetSourcePos));

  exports.Set("InitPreview", Napi::Function::New(env, ObsInitPreview));
  exports.Set("ShowPreview", Napi::Function::New(env, ObsShowPreview));
  exports.Set("HidePreview", Napi::Function::New(env, ObsHidePreview));
  exports.Set("GetPreviewScaleFactor", Napi::Function::New(env, ObsGetPreviewScaleFactor));
  exports.Set("GetDrawSourceOutlineEnabled", Napi::Function::New(env, ObsGetDrawSourceOutlineEnabled));
  exports.Set("SetDrawSourceOutline", Napi::Function::New(env, ObsSetDrawSourceOutline));

  return exports;
}

NODE_API_MODULE(addon, Init)