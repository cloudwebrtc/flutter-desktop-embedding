#include "flutter_webrtc_plugin.h"

#include <flutter/json_method_codec.h>
#include "flutter_webrtc.h"

const char *kChannelName = "cloudwebrtc.com/WebRTC.Method";

namespace flutter_webrtc_plugin {

// A webrtc plugin for windows/linux.
class FlutterWebRTCPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrar *registrar) {
    auto channel = std::make_unique<flutter::MethodChannel<Json::Value>>(
        registrar->messenger(), kChannelName,
        &flutter::JsonMethodCodec::GetInstance());

    auto *channel_pointer = channel.get();

    // Uses new instead of make_unique due to private constructor.
    std::unique_ptr<FlutterWebRTCPlugin> plugin(
        new FlutterWebRTCPlugin(registrar, std::move(channel)));

    channel_pointer->SetMethodCallHandler(
        [plugin_pointer = plugin.get()](const auto &call, auto result) {
          plugin_pointer->HandleMethodCall(call, std::move(result));
        });

    registrar->AddPlugin(std::move(plugin));
  }

  virtual ~FlutterWebRTCPlugin() {}

  flutter::BinaryMessenger *messenger() { return messenger_; }

  flutter::TextureRegistrar *textures() { return textures_; }

 private:
  // Creates a plugin that communicates on the given channel.
  FlutterWebRTCPlugin(
      flutter::PluginRegistrar *registrar,
      std::unique_ptr<flutter::MethodChannel<Json::Value>> channel)
      : channel_(std::move(channel)),
        messenger_(registrar->messenger()),
        textures_(registrar->textures()) {
    webrtc_ = std::make_unique<FlutterWebRTC>(this);
  }

  // Called when a method is called on |channel_|;
  void HandleMethodCall(
      const flutter::MethodCall<Json::Value> &method_call,
      std::unique_ptr<flutter::MethodResult<Json::Value>> result) {
    // handle method call and forward to webrtc native sdk.
    webrtc_->HandleMethodCall(method_call, std::move(result));
  }

 private:
  std::unique_ptr<flutter::MethodChannel<Json::Value>> channel_;
  std::unique_ptr<FlutterWebRTC> webrtc_;
  flutter::BinaryMessenger *messenger_;
  flutter::TextureRegistrar *textures_;
};

}  // namespace flutter_webrtc_plugin

void FlutterWebRTCRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  static auto *plugin_registrar = new flutter::PluginRegistrar(registrar);
  flutter_webrtc_plugin::FlutterWebRTCPlugin::RegisterWithRegistrar(
      plugin_registrar);
}
