#include "flutter_webrtc_base.h"
#include "flutter_data_channel.h"
#include "flutter_peerconnection.h"

namespace flutter_webrtc_plugin {

FlutterWebRTCBase::FlutterWebRTCBase(BinaryMessenger *messenger,
                                     TextureRegistrar *textures)
    : messenger_(messenger), textures_(textures) {
  LibWebRTC::Initialize();
  factory_ = LibWebRTC::CreateRTCPeerConnectionFactory();
  audio_device_ = factory_->GetAudioDevice();
  video_device_ = factory_->GetVideoDevice();
}

FlutterWebRTCBase::~FlutterWebRTCBase() { LibWebRTC::Terminate(); }

std::string FlutterWebRTCBase::GenerateUUID() {
  return uuidxx::uuid::Generate().ToString(false);
}

RTCPeerConnection *FlutterWebRTCBase::PeerConnectionForId(
    const std::string &id) {
  auto it = peerconnections_.find(id);

  if (it != peerconnections_.end()) return (*it).second.get();

  return nullptr;
}

scoped_refptr<RTCMediaStream> FlutterWebRTCBase::MediaStreamForId(
    const std::string &id) {
  auto it = media_streams_.find(id);

  if (it != media_streams_.end()) return (*it).second;

  return nullptr;
}

bool FlutterWebRTCBase::ParseConstraints(const Json::Value &constraints,
                                         RTCConfiguration *configuration) {
  memset(&configuration->ice_servers, 0, sizeof(configuration->ice_servers));
  return false;
}

};  // namespace flutter_webrtc_plugin
