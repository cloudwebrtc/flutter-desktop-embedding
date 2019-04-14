#ifndef PLUGINS_FLUTTER_WEBRTC_INCLUDE_FLUTTER_WEBRTC_PLUGIN_H_
#define PLUGINS_FLUTTER_WEBRTC_INCLUDE_FLUTTER_WEBRTC_PLUGIN_H_

#include <flutter_plugin_registrar.h>

#ifdef FLUTTER_WEBRTC_PLUGIN_IMPL

// Add visibiilty/export annotations when building the library.
#ifdef _WIN32
#define FLUTTER_WEBRTC_API __declspec(dllexport)
#else
#define FLUTTER_WEBRTC_API __attribute__((visibility("default")))
#endif

#else

// Add import annotations when consuming the library.
#ifdef _WIN32
#define FLUTTER_WEBRTC_API __declspec(dllimport)
#else
#define FLUTTER_WEBRTC_API
#endif

#endif

#if defined(__cplusplus)
extern "C" {
#endif

FLUTTER_WEBRTC_API void FlutterWebRTCRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar);

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // PLUGINS_FLUTTER_WEBRTC_INCLUDE_FLUTTER_WEBRTC_PLUGIN_H_
