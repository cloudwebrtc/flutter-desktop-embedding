#include "flutter_peerconnection.h"
#include "flutter_data_channel.h"

namespace flutter_webrtc_plugin {

void FlutterPeerConnection::CreateRTCPeerConnection(
    const Json::Value *constraints,
    std::unique_ptr<MethodResult<Json::Value>> result) {
  std::cout << " constraints = " << *constraints << std::endl;
  base_->ParseConstraints(*constraints, &base_->configuration_);

  std::string uuid = base_->GenerateUUID();
  scoped_refptr<RTCPeerConnection> pc =
      base_->factory_->Create(base_->configuration_);
  base_->peerconnections_[uuid] = pc;

  std::string event_channel =
      "cloudwebrtc.com/WebRTC/peerConnectoinEvent" + uuid;

  std::unique_ptr<FlutterPeerConnectionObserver> observer(
      new FlutterPeerConnectionObserver(base_, pc, base_->messenger_,
                                        event_channel));

  base_->peerconnection_observers_[event_channel] = std::move(observer);

  Json::Value res;
  res["peerConnectionId"] = uuid;
  result->Success(&res);
}

void FlutterPeerConnection::RTCPeerConnectionClose(
    RTCPeerConnection *pc, const std::string &uuid,
    std::unique_ptr<MethodResult<Json::Value>> result) {
  pc->Close();
  auto it = base_->peerconnection_observers_.find(uuid);
  if (it != base_->peerconnection_observers_.end())
    base_->peerconnection_observers_.erase(it);

  result->Success(nullptr);
}

void FlutterPeerConnection::CreateOffer(
    const Json::Value *constraints, RTCPeerConnection *pc,
    std::unique_ptr<MethodResult<Json::Value>> result) {
  // TODO: Convert constraints to RTCOfferAnswerOptions
  std::cout << " constraints = " << *constraints << std::endl;
  std::shared_ptr<MethodResult<Json::Value>> result_ptr(result.release());
  pc->CreateOffer(
      [result_ptr](const char *sdp, const char *type) {
        Json::Value res_params;
        res_params["sdp"] = sdp;
        res_params["type"] = type;
        result_ptr->Success(&res_params);
      },
      [result_ptr](const char *error) {
        result_ptr->Error("createOfferFailed", error);
      });
}

void FlutterPeerConnection::CreateAnswer(
    const Json::Value *constraints, RTCPeerConnection *pc,
    std::unique_ptr<MethodResult<Json::Value>> result) {
  // TODO: Convert constraints to RTCOfferAnswerOptions
  std::shared_ptr<MethodResult<Json::Value>> result_ptr(result.release());
  pc->CreateAnswer(
      [result_ptr](const char *sdp, const char *type) {
        Json::Value res_params;
        res_params["sdp"] = sdp;
        res_params["type"] = type;
        result_ptr->Success(&res_params);
      },
      [result_ptr](const std::string &error) {
        result_ptr->Error("createAnswerFailed", error);
      });
}

void FlutterPeerConnection::SetLocalDescription(
    RTCSessionDescription *sdp, RTCPeerConnection *pc,
    std::unique_ptr<MethodResult<Json::Value>> result) {
  std::shared_ptr<MethodResult<Json::Value>> result_ptr(result.release());
  pc->SetLocalDescription(
      sdp->sdp(), sdp->type(),
      [result_ptr]() { result_ptr->Success(nullptr); },
      [result_ptr](const char *error) {
        result_ptr->Error("setLocalDescriptionFailed", error);
      });
}

void FlutterPeerConnection::SetRemoteDescription(
    RTCSessionDescription *sdp, RTCPeerConnection *pc,
    std::unique_ptr<MethodResult<Json::Value>> result) {
  std::shared_ptr<MethodResult<Json::Value>> result_ptr(result.release());
  pc->SetRemoteDescription(
      sdp->sdp(), sdp->type(),
      [result_ptr]() { result_ptr->Success(nullptr); },
      [result_ptr](const char *error) {
        result_ptr->Error("setRemoteDescriptionFailed", error);
      });
}

void FlutterPeerConnection::AddIceCandidate(
    RTCIceCandidate *candidate, RTCPeerConnection *pc,
    std::unique_ptr<MethodResult<Json::Value>> result) {
  pc->AddCandidate(candidate->sdp_mid(), candidate->sdp_mline_index(),
                   candidate->candidate());
  result->Success(nullptr);
}

void FlutterPeerConnection::GetStats(
    const std::string &track_id, RTCPeerConnection *pc,
    std::unique_ptr<MethodResult<Json::Value>> result) {}

FlutterPeerConnectionObserver::FlutterPeerConnectionObserver(
    FlutterWebRTCBase *base, scoped_refptr<RTCPeerConnection> peerconnection,
    BinaryMessenger *messenger, const std::string &channel_name)
    : base_(base),
      peerconnection_(peerconnection),
      event_channel_(new EventChannel<Json::Value>(
          messenger, channel_name, &JsonMethodCodec::GetInstance(),
          &JsonMessageCodec::GetInstance())) {
  StreamHandler<Json::Value> stream_handler = {
      [&](const Json::Value *arguments,
          const EventSink<Json::Value> *events) -> MethodResult<Json::Value> * {
        event_sink_ = events;
        return nullptr;
      },
      [&](const Json::Value *arguments) -> MethodResult<Json::Value> * {
        event_sink_ = nullptr;
        return nullptr;
      }};
  event_channel_->SetStreamHandler(stream_handler);
}
/*
bool FlutterPeerConnection::ParseRTCConfiguration(const Json::Value &map) {

  ConstraintsArray iceServersArray = null;
  if (map != null) {
    iceServersArray = map.getArray("iceServers");
  }
  List<PeerConnection.IceServer> iceServers = createIceServers(iceServersArray);
  PeerConnection.RTCConfiguration conf =
      new PeerConnection.RTCConfiguration(iceServers);
  if (map == null) {
    return conf;
  }

  // iceTransportPolicy (public api)
  if (map.hasKey("iceTransportPolicy") &&
      map.getType("iceTransportPolicy") == ObjectType.String) {
    final String v = map.getString("iceTransportPolicy");
    if (v != null) {
      switch (v) {
        case "all":  // public
          conf.iceTransportsType = PeerConnection.IceTransportsType.ALL;
          break;
        case "relay":  // public
          conf.iceTransportsType = PeerConnection.IceTransportsType.RELAY;
          break;
        case "nohost":
          conf.iceTransportsType = PeerConnection.IceTransportsType.NOHOST;
          break;
        case "none":
          conf.iceTransportsType = PeerConnection.IceTransportsType.NONE;
          break;
      }
    }
  }

  // bundlePolicy (public api)
  if (map.hasKey("bundlePolicy") &&
      map.getType("bundlePolicy") == ObjectType.String) {
    final String v = map.getString("bundlePolicy");
    if (v != null) {
      switch (v) {
        case "balanced":  // public
          conf.bundlePolicy = PeerConnection.BundlePolicy.BALANCED;
          break;
        case "max-compat":  // public
          conf.bundlePolicy = PeerConnection.BundlePolicy.MAXCOMPAT;
          break;
        case "max-bundle":  // public
          conf.bundlePolicy = PeerConnection.BundlePolicy.MAXBUNDLE;
          break;
      }
    }
  }

  // rtcpMuxPolicy (public api)
  if (map.hasKey("rtcpMuxPolicy") &&
      map.getType("rtcpMuxPolicy") == ObjectType.String) {
    final String v = map.getString("rtcpMuxPolicy");
    if (v != null) {
      switch (v) {
        case "negotiate":  // public
          conf.rtcpMuxPolicy = PeerConnection.RtcpMuxPolicy.NEGOTIATE;
          break;
        case "require":  // public
          conf.rtcpMuxPolicy = PeerConnection.RtcpMuxPolicy.REQUIRE;
          break;
      }
    }
  }

  // FIXME: peerIdentity of type DOMString (public api)
  // FIXME: certificates of type sequence<RTCCertificate> (public api)

  // iceCandidatePoolSize of type unsigned short, defaulting to 0
  if (map.hasKey("iceCandidatePoolSize") &&
      map.getType("iceCandidatePoolSize") == ObjectType.Number) {
    final int v = map.getInt("iceCandidatePoolSize");
    if (v > 0) {
      conf.iceCandidatePoolSize = v;
    }
  }
  return conf;
}*/

static const char *iceConnectionStateString(RTCIceConnectionState state) {
  switch (state) {
    case RTCIceConnectionStateNew:
      return "new";
    case RTCIceConnectionStateChecking:
      return "checking";
    case RTCIceConnectionStateConnected:
      return "connected";
    case RTCIceConnectionStateCompleted:
      return "completed";
    case RTCIceConnectionStateFailed:
      return "failed";
    case RTCIceConnectionStateDisconnected:
      return "disconnected";
    case RTCIceConnectionStateClosed:
      return "closed";
    case RTCIceConnectionStateCount:
      return "count";
  }
  return "";
}

static const char *signalingStateString(RTCSignalingState state) {
  switch (state) {
    case RTCSignalingStateStable:
      return "stable";
    case RTCSignalingStateHaveLocalOffer:
      return "have-local-offer";
    case RTCSignalingStateHaveLocalPrAnswer:
      return "have-local-pranswer";
    case RTCSignalingStateHaveRemoteOffer:
      return "have-remote-offer";
    case RTCSignalingStateHaveRemotePrAnswer:
      return "have-remote-pranswer";
    case RTCSignalingStateClosed:
      return "closed";
  }
  return "";
}
void FlutterPeerConnectionObserver::onSignalingState(RTCSignalingState state) {
  if (event_sink_ != nullptr) {
    Json::Value params;
    params["event"] = "iceConnectionState";
    params["state"] = signalingStateString(state);
    (*event_sink_)(&params);
  }
}

static const char *iceGatheringStateString(RTCIceGatheringState state) {
  switch (state) {
    case RTCIceGatheringStateNew:
      return "new";
    case RTCIceGatheringStateGathering:
      return "gathering";
    case RTCIceGatheringStateComplete:
      return "complete";
  }
  return "";
}

void FlutterPeerConnectionObserver::onIceGatheringState(
    RTCIceGatheringState state) {
  if (event_sink_ != nullptr) {
    Json::Value params;
    params["event"] = "iceGatheringState";
    params["state"] = iceGatheringStateString(state);
    (*event_sink_)(&params);
  }
}

void FlutterPeerConnectionObserver::onIceConnectionState(
    RTCIceConnectionState state) {
  if (event_sink_ != nullptr) {
    Json::Value params;
    params["event"] = "signalingState";
    params["state"] = iceConnectionStateString(state);
    (*event_sink_)(&params);
  }
}

void FlutterPeerConnectionObserver::onIceCandidate(RTCIceCandidate *candidate) {
  if (event_sink_ != nullptr) {
    Json::Value params;
    params["event"] = "onCandidate";
    Json::Value cand;
    cand["candidate"] = candidate->candidate();
    cand["sdpMLineIndex"] = candidate->sdp_mline_index();
    cand["sdpMid"] = candidate->sdp_mid();
    params["candidate"] = cand;
    (*event_sink_)(&params);
  }
}

void FlutterPeerConnectionObserver::onAddStream(RTCMediaStream *stream) {
  if (event_sink_ != nullptr) {
    Json::Value params;
    params["event"] = "onAddStream";
    params["streamId"] = stream->label();

    Json::Value audioTracks;
    for (auto track : stream->GetAudioTracks()) {
      Json::Value audioTrack;

      audioTrack["id"] = track->id();
      audioTrack["label"] = track->id();
      audioTrack["kind"] = track->kind();
      audioTrack["enabled"] = track->enabled();
      audioTrack["remote"] = true;
      audioTrack["readyState"] = "live";

      audioTracks.append(audioTrack);
    }
    params["audioTracks"] = audioTracks;

    Json::Value videoTracks;
    for (auto track : stream->GetVideoTracks()) {
      Json::Value videoTrack;

      videoTrack["id"] = track->id();
      videoTrack["label"] = track->id();
      videoTrack["kind"] = track->kind();
      videoTrack["enabled"] = track->enabled();
      videoTrack["remote"] = true;
      videoTrack["readyState"] = "live";

      videoTracks.append(videoTrack);
    }
    params["videoTracks"] = videoTracks;
    (*event_sink_)(&params);
  }
}

void FlutterPeerConnectionObserver::onRemoveStream(RTCMediaStream *stream) {
  if (event_sink_ != nullptr) {
    Json::Value params;
    params["event"] = "onRemoveStream";
    params["streamId"] = stream->label();
    (*event_sink_)(&params);
  }
}

void FlutterPeerConnectionObserver::onAddTrack(RTCMediaStream *stream,
                                               RTCMediaTrack *track) {
  if (event_sink_ != nullptr) {
    Json::Value params;
    params["event"] = "onAddTrack";
    params["streamId"] = stream->label();
    params["trackId"] = track->id();

    Json::Value audioTrack;
    audioTrack["id"] = track->id();
    audioTrack["label"] = track->id();
    audioTrack["kind"] = track->kind();
    audioTrack["enabled"] = track->enabled();
    audioTrack["remote"] = true;
    audioTrack["readyState"] = "live";
    params["track"] = audioTrack;

    (*event_sink_)(&params);
  }
}

void FlutterPeerConnectionObserver::onRemoveTrack(RTCMediaStream *stream,
                                                  RTCMediaTrack *track) {
  if (event_sink_ != nullptr) {
    Json::Value params;
    params["event"] = "onRemoveTrack";
    params["streamId"] = stream->label();
    params["trackId"] = track->id();

    Json::Value videoTrack;
    videoTrack["id"] = track->id();
    videoTrack["label"] = track->id();
    videoTrack["kind"] = track->kind();
    videoTrack["enabled"] = track->enabled();
    videoTrack["remote"] = true;
    videoTrack["readyState"] = "live";
    params["track"] = videoTrack;

    (*event_sink_)(&params);
  }
}

void FlutterPeerConnectionObserver::onDataChannel(
    RTCDataChannel *data_channel) {
  std::string event_channel = "cloudwebrtc.com/WebRTC/dataChannelEvent" +
                              std::to_string(data_channel->id());

  std::unique_ptr<FlutterRTCDataChannelObserver> observer(
      new FlutterRTCDataChannelObserver(data_channel, base_->messenger_,
                                        event_channel));

  base_->data_channel_observers_[data_channel->id()] = std::move(observer);
  if (event_sink_) {
    Json::Value params;
    params["event"] = "didOpenDataChannel";
    params["id"] = data_channel->id();
    params["label"] = data_channel->label();
    (*event_sink_)(&params);
  }
}

void FlutterPeerConnectionObserver::onRenegotiationNeeded() {
  if (event_sink_ != nullptr) {
    Json::Value params;
    params["event"] = "onRenegotiationNeeded";
    (*event_sink_)(&params);
  }
}

};  // namespace flutter_webrtc_plugin