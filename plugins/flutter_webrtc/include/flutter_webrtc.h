#ifndef PLUGINS_FLUTTER_WEBRTC_HXX
#define PLUGINS_FLUTTER_WEBRTC_HXX

#include <json/json.h>
#include "libwebrtc.h"

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>

#include "flutter_data_channel.h"
#include "flutter_media_stream.h"
#include "flutter_peerconnection.h"
#include "flutter_video_renderer.h"

namespace flutter_webrtc_plugin {
using namespace libwebrtc;

class FlutterWebRTCPlugin : public flutter::Plugin {
 public:
  virtual flutter::BinaryMessenger *messenger() = 0;

  virtual flutter::TextureRegistrar *textures() = 0;
};

class FlutterWebRTC : public FlutterWebRTCBase,
                      public FlutterVideoRendererManager,
                      public FlutterMediaStream,
                      public FlutterPeerConnection,
                      public FlutterDataChannel {
 public:
  FlutterWebRTC(FlutterWebRTCPlugin *plugin);
  virtual ~FlutterWebRTC();

  void HandleMethodCall(
      const flutter::MethodCall<Json::Value> &method_call,
      std::unique_ptr<flutter::MethodResult<Json::Value>>
          result);

 private:
  FlutterWebRTCPlugin *plugin_ = nullptr;
};

};  // namespace flutter_webrtc_plugin

#endif  // PLUGINS_FLUTTER_WEBRTC_HXX
