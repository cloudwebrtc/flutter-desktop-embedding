#ifndef FLUTTER_WEBRTC_RTC_PEER_CONNECTION_HXX
#define FLUTTER_WEBRTC_RTC_PEER_CONNECTION_HXX

#include "flutter_webrtc_base.h"

namespace flutter_webrtc_plugin {

using namespace flutter;

class FlutterPeerConnectionObserver : public RTCPeerConnectionObserver {
 public:
  FlutterPeerConnectionObserver(FlutterWebRTCBase *base,
                                scoped_refptr<RTCPeerConnection> peerconnection,
                                BinaryMessenger *messenger,
                                const std::string &channel_name);

  virtual void onSignalingState(RTCSignalingState state) override;
  virtual void onIceGatheringState(RTCIceGatheringState state) override;
  virtual void onIceConnectionState(RTCIceConnectionState state) override;
  virtual void onIceCandidate(RTCIceCandidate *candidate) override;
  virtual void onAddStream(RTCMediaStream *stream) override;
  virtual void onRemoveStream(RTCMediaStream *stream) override;
  virtual void onAddTrack(RTCMediaStream *stream,
                          RTCMediaTrack *track) override;
  virtual void onRemoveTrack(RTCMediaStream *stream,
                             RTCMediaTrack *track) override;
  virtual void onDataChannel(RTCDataChannel *data_channel) override;
  virtual void onRenegotiationNeeded() override;

 private:
  std::unique_ptr<EventChannel<Json::Value>> event_channel_;
  const EventSink<Json::Value> *event_sink_ = nullptr;
  scoped_refptr<RTCPeerConnection> peerconnection_;
  FlutterWebRTCBase *base_;
};

class FlutterPeerConnection {
 public:
  FlutterPeerConnection(FlutterWebRTCBase *base) : base_(base) {}

  void CreateRTCPeerConnection(
      const Json::Value *constraints,
      std::unique_ptr<MethodResult<Json::Value>> result);

  void RTCPeerConnectionClose(
      RTCPeerConnection *pc, const std::string &uuid,
      std::unique_ptr<MethodResult<Json::Value>> result);

  void CreateOffer(const Json::Value *constraints, RTCPeerConnection *pc,
                   std::unique_ptr<MethodResult<Json::Value>> result);

  void CreateAnswer(const Json::Value *constraints, RTCPeerConnection *pc,
                    std::unique_ptr<MethodResult<Json::Value>> result);

  void SetLocalDescription(RTCSessionDescription *sdp, RTCPeerConnection *pc,
                           std::unique_ptr<MethodResult<Json::Value>> result);

  void SetRemoteDescription(RTCSessionDescription *sdp, RTCPeerConnection *pc,
                            std::unique_ptr<MethodResult<Json::Value>> result);

  void AddIceCandidate(RTCIceCandidate *candidate, RTCPeerConnection *pc,
                       std::unique_ptr<MethodResult<Json::Value>> result);

  void GetStats(const std::string &track_id, RTCPeerConnection *pc,
                std::unique_ptr<MethodResult<Json::Value>> result);

 private:
  // bool ParseRTCConfiguration(const Json::Value &map);

 private:
  FlutterWebRTCBase *base_;
};
};  // namespace flutter_webrtc_plugin

#endif  // !FLUTTER_WEBRTC_RTC_PEER_CONNECTION_HXX