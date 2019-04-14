#ifndef FLUTTER_WEBRTC_BASE_HXX
#define FLUTTER_WEBRTC_BASE_HXX

#include <flutter/event_channel.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar.h>
#include <flutter/texture_registrar.h>
#include <flutter/json_message_codec.h >
#include <flutter/json_method_codec.h >

#include "libwebrtc.h"
#include "rtc_audio_device.h"
#include "rtc_media_stream.h"
#include "rtc_media_track.h"
#include "rtc_peerconnection.h"
#include "rtc_peerconnection_factory.h"
#include "rtc_video_device.h"

#include "json/json.h"
#include "uuidxx.h"

#include <list>
#include <map>
#include <memory>

namespace flutter_webrtc_plugin {

using namespace libwebrtc;
using namespace flutter;

class FlutterVideoRenderer;
class FlutterRTCDataChannelObserver;
class FlutterPeerConnectionObserver;

class FlutterWebRTCBase {
 public:
  friend class FlutterMediaStream;
  friend class FlutterPeerConnection;
  friend class FlutterVideoRendererManager;
  friend class FlutterDataChannel;
  friend class FlutterPeerConnectionObserver;

 public:
  FlutterWebRTCBase(BinaryMessenger *messenger, TextureRegistrar *textures);
  ~FlutterWebRTCBase();

  std::string GenerateUUID();

  RTCPeerConnection *PeerConnectionForId(const std::string &id);

  scoped_refptr<RTCMediaStream> MediaStreamForId(const std::string &id);

  bool ParseConstraints(const Json::Value &constraints,
                        RTCConfiguration *configuration);

 protected:
  scoped_refptr<RTCPeerConnectionFactory> factory_;
  scoped_refptr<RTCAudioDevice> audio_device_;
  scoped_refptr<RTCVideoDevice> video_device_;
  RTCConfiguration configuration_;

  std::map<std::string, scoped_refptr<RTCPeerConnection>> peerconnections_;
  std::map<std::string, scoped_refptr<RTCMediaStream>> media_streams_;
  std::map<std::string, scoped_refptr<RTCMediaTrack>> media_tracks_;
  std::map<std::string, scoped_refptr<RTCDataChannel>> data_channels_;
  std::map<int64_t, std::shared_ptr<FlutterVideoRenderer>> renders_;
  std::map<int, std::unique_ptr<FlutterRTCDataChannelObserver>>
      data_channel_observers_;
  std::map<std::string, std::unique_ptr<FlutterPeerConnectionObserver>>
      peerconnection_observers_;

 protected:
  BinaryMessenger *messenger_;
  TextureRegistrar *textures_;
};

};  // namespace flutter_webrtc_plugin

#endif  // !FLUTTER_WEBRTC_BASE_HXX
