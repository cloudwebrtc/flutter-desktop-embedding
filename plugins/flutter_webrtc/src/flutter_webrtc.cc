#include "flutter_webrtc.h"
#include "flutter_webrtc_plugin.h"

namespace flutter_webrtc_plugin {

FlutterWebRTC::FlutterWebRTC(FlutterWebRTCPlugin *plugin)
    : FlutterWebRTCBase::FlutterWebRTCBase(plugin->messenger(),
                                           plugin->textures()),
      FlutterVideoRendererManager::FlutterVideoRendererManager(this),
      FlutterMediaStream::FlutterMediaStream(this),
      FlutterPeerConnection::FlutterPeerConnection(this),
      FlutterDataChannel::FlutterDataChannel(this),
      plugin_(plugin) {}

FlutterWebRTC::~FlutterWebRTC() {}

void FlutterWebRTC::HandleMethodCall(
    const flutter::MethodCall<Json::Value> &method_call,
    std::unique_ptr<flutter::MethodResult<Json::Value>>
        result) {
  if (method_call.method_name().compare("createPeerConnection") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null arguments received");
      return;
    }
    const Json::Value &arguments = *method_call.arguments();
    CreateRTCPeerConnection(&arguments, std::move(result));
  } else if (method_call.method_name().compare("getUserMedia") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const Json::Value &constraints = *method_call.arguments();
    GetUserMedia(&constraints, std::move(result));
  } else if (method_call.method_name().compare("getDisplayMedia") == 0) {
    result->NotImplemented();
  } else if (method_call.method_name().compare("getSources") == 0) {
    GetSources(std::move(result));
  } else if (method_call.method_name().compare("mediaStreamGetTracks") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const Json::Value &params = *method_call.arguments();
    const std::string streamId = params["streamId"].asString();
    MediaStreamGetTracks(streamId, std::move(result));
  } else if (method_call.method_name().compare("createOffer") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const Json::Value &params = *method_call.arguments();
    const std::string peerConnectionId = params["peerConnectionId"].asString();
    const Json::Value constraints = params["constraints"];
    RTCPeerConnection *pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("createOfferFailed",
                    "createOffer() peerConnection is null");
      return;
    }
    CreateOffer(&constraints, pc, std::move(result));
  } else if (method_call.method_name().compare("createAnswer") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const Json::Value &params = *method_call.arguments();
    const std::string peerConnectionId = params["peerConnectionId"].asString();
    const Json::Value constraints = params["constraints"];
    RTCPeerConnection *pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("createAnswerFailed",
                    "createAnswer() peerConnection is null");
      return;
    }
    CreateAnswer(&constraints, pc, std::move(result));
  } else if (method_call.method_name().compare("addStream") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null arguments received");
      return;
    }
    const Json::Value &params = *method_call.arguments();
    const std::string streamId = params["streamId"].asString();
    const std::string peerConnectionId = params["peerConnectionId"].asString();

    scoped_refptr<RTCMediaStream> stream = MediaStreamForId(streamId);
    if (!stream) {
      result->Error("addStreamFailed", "addStream() stream not found!");
      return;
    }
    RTCPeerConnection *pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("addStreamFailed", "addStream() peerConnection is null");
      return;
    }
    pc->AddStream(stream);
    result->Success(nullptr);
  } else if (method_call.method_name().compare("removeStream") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null arguments received");
      return;
    }
    const Json::Value &params = *method_call.arguments();
    const std::string streamId = params["streamId"].asString();
    const std::string peerConnectionId = params["peerConnectionId"].asString();

    scoped_refptr<RTCMediaStream> stream = MediaStreamForId(streamId);
    if (!stream) {
      result->Error("removeStreamFailed", "removeStream() stream not found!");
      return;
    }
    RTCPeerConnection *pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("removeStreamFailed",
                    "removeStream() peerConnection is null");
      return;
    }
    pc->RemoveStream(stream);
    result->Success(nullptr);
  } else if (method_call.method_name().compare("setLocalDescription") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const Json::Value &params = *method_call.arguments();
    const std::string peerConnectionId = params["peerConnectionId"].asString();
    const Json::Value constraints = params["description"];
    RTCPeerConnection *pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("setLocalDescriptionFailed",
                    "setLocalDescription() peerConnection is null");
      return;
    }

    SdpParseError error;
    scoped_refptr<RTCSessionDescription> description =
        CreateRTCSessionDescription(constraints["type"].asCString(),
                                    constraints["sdp"].asCString(), &error);

    SetLocalDescription(description.get(), pc, std::move(result));
  } else if (method_call.method_name().compare("setRemoteDescription") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const Json::Value &params = *method_call.arguments();
    const std::string peerConnectionId = params["peerConnectionId"].asString();
    const Json::Value constraints = params["description"];
    RTCPeerConnection *pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("setRemoteDescriptionFailed",
                    "setRemoteDescription() peerConnection is null");
      return;
    }

    SdpParseError error;
    scoped_refptr<RTCSessionDescription> description =
        CreateRTCSessionDescription(constraints["type"].asCString(),
                                    constraints["sdp"].asCString(), &error);

    SetRemoteDescription(description.get(), pc, std::move(result));
  } else if (method_call.method_name().compare("addCandidate") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const Json::Value &params = *method_call.arguments();
    const std::string peerConnectionId = params["peerConnectionId"].asString();
    const Json::Value constraints = params["candidate"];
    RTCPeerConnection *pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("addCandidateFailed",
                    "addCandidate() peerConnection is null");
      return;
    }

    SdpParseError error;
    scoped_refptr<RTCIceCandidate> rtc_candidate = CreateRTCIceCandidate(
        constraints["candidate"].asCString(), constraints["sdpMid"].asCString(),
        constraints["sdpMLineIndex"].asInt(), &error);

    AddIceCandidate(rtc_candidate.get(), pc, std::move(result));
  } else if (method_call.method_name().compare("getStats") == 0) {
  } else if (method_call.method_name().compare("createDataChannel") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const Json::Value &params = *method_call.arguments();
    const std::string peerConnectionId = params["peerConnectionId"].asString();

    RTCPeerConnection *pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("createDataChannelFailed",
                    "createDataChannel() peerConnection is null");
      return;
    }

    const std::string label = params["label"].asString();
    const Json::Value dataChannelDict = params["dataChannelDict"];

    CreateDataChannel(label, dataChannelDict, pc, std::move(result));
  } else if (method_call.method_name().compare("dataChannelSend") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const Json::Value &params = *method_call.arguments();
    const std::string peerConnectionId = params["peerConnectionId"].asString();
    RTCPeerConnection *pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("dataChannelSendFailed",
                    "dataChannelSend() peerConnection is null");
      return;
    }

    int dataChannelId = params["dataChannelId"].asInt();
    const std::string type = params["type"].asString();
    const std::string data = params["data"].asString();
    RTCDataChannel *data_channel = DataChannelFormId(dataChannelId);
    if (data_channel == nullptr) {
      result->Error("dataChannelSendFailed",
                    "dataChannelSend() data_channel is null");
      return;
    }
    DataChannelSend(data_channel, type, data, std::move(result));
  } else if (method_call.method_name().compare("dataChannelClose") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const Json::Value &params = *method_call.arguments();
    const std::string peerConnectionId = params["peerConnectionId"].asString();
    RTCPeerConnection *pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("dataChannelCloseFailed",
                    "dataChannelClose() peerConnection is null");
      return;
    }

    int dataChannelId = params["dataChannelId"].asInt();
    RTCDataChannel *data_channel = DataChannelFormId(dataChannelId);
    if (data_channel == nullptr) {
      result->Error("dataChannelCloseFailed",
                    "dataChannelClose() data_channel is null");
      return;
    }
    DataChannelClose(data_channel, std::move(result));
  } else if (method_call.method_name().compare("streamDispose") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const Json::Value &params = *method_call.arguments();
    const std::string stream_id = params["streamId"].asString();
    MediaStreamDispose(stream_id, std::move(result));
  } else if (method_call.method_name().compare("mediaStreamTrackSetEnable") ==
             0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const Json::Value &params = *method_call.arguments();
    const std::string track_id = params["trackId"].asString();
    MediaStreamTrackSetEnable(track_id, std::move(result));
  } else if (method_call.method_name().compare("trackDispose") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const Json::Value &params = *method_call.arguments();
    const std::string track_id = params["trackId"].asString();
    MediaStreamTrackDispose(track_id, std::move(result));
  } else if (method_call.method_name().compare("peerConnectionClose") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const Json::Value &params = *method_call.arguments();
    const std::string peerConnectionId = params["peerConnectionId"].asString();
    RTCPeerConnection *pc = PeerConnectionForId(peerConnectionId);
    if (pc == nullptr) {
      result->Error("peerConnectionCloseFailed",
                    "peerConnectionClose() peerConnection is null");
      return;
    }
    RTCPeerConnectionClose(pc, peerConnectionId, std::move(result));
  } else if (method_call.method_name().compare("createVideoRenderer") == 0) {
    CreateVideoRendererTexture(std::move(result));
  } else if (method_call.method_name().compare("videoRendererDispose") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const Json::Value &params = *method_call.arguments();
    int64_t texture_id = params["textureId"].asInt64();
    VideoRendererDispose(texture_id, std::move(result));
  } else if (method_call.method_name().compare("videoRendererSetSrcObject") ==
             0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const Json::Value &params = *method_call.arguments();
    const std::string stream_id = params["streamId"].asString();
    int64_t texture_id = params["textureId"].asInt64();
    SetMediaStream(texture_id, stream_id);
    result->Success(nullptr);
  } else if (method_call.method_name().compare(
                 "mediaStreamTrackSwitchCamera") == 0) {
    if (!method_call.arguments()) {
      result->Error("Bad Arguments", "Null constraints arguments received");
      return;
    }
    const Json::Value &params = *method_call.arguments();
    const std::string track_id = params["trackId"].asString();
    MediaStreamTrackSwitchCamera(track_id, std::move(result));
  } else if (method_call.method_name().compare("setVolume") == 0) {
  } else {
    result->NotImplemented();
  }
}

};  // namespace flutter_webrtc_plugin
