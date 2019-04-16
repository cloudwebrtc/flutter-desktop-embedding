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
  memset(&configuration_.ice_servers, 0, sizeof(configuration_.ice_servers));
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

void FlutterWebRTCBase::ParseConstraints(
    Json::Value src, scoped_refptr<RTCMediaConstraints> mediaConstraints,
    ParseConstraintType type /*= kMandatory*/) {
  Json::Value::Members mem = src.getMemberNames();
  Json::Value params = src;
  for (auto iter = mem.begin(); iter != mem.end(); iter++) {
    std::string key(*iter);
    std::string value;
    if (params[*iter].type() == Json::objectValue ||
        params[*iter].type() == Json::arrayValue) {
    } else if (params[*iter].type() == Json::stringValue) {
      value = std::string(params[*iter].asString());
    } else if (params[*iter].type() == Json::realValue) {
      value = std::to_string(params[*iter].asDouble());
    } else if (params[*iter].type() == Json::uintValue) {
      value = std::to_string(params[*iter].asUInt());
    } else if (params[*iter].type() == Json::booleanValue) {
      value = params[*iter].asBool() ? RTCMediaConstraints::kValueTrue
                                     : RTCMediaConstraints::kValueFalse;
    } else {
      value = std::to_string(params[*iter].asInt());
    }
    if (type == kMandatory)
      mediaConstraints->AddMandatoryConstraint(key.c_str(), value.c_str());
    else
      mediaConstraints->AddOptionalConstraint(key.c_str(), value.c_str());
  }
}

scoped_refptr<RTCMediaConstraints> FlutterWebRTCBase::ParseMediaConstraints(
    const Json::Value& constraints) {
  scoped_refptr<RTCMediaConstraints> media_constraints =
      RTCMediaConstraints::Create();

  if (!constraints["mandatory"].isNull() &&
      constraints["mandatory"].isObject()) {
    ParseConstraints(constraints["mandatory"], media_constraints, kMandatory);
  } else {
    // Log.d(TAG, "mandatory constraints are not a map");
  }

  if (constraints["optional"] && constraints["optional"].isArray()) {
    Json::Value optional = constraints["optional"];
    for (int i = 0, size = optional.size(); i < size; i++) {
      if (optional[i].isObject()) {
        ParseConstraints(optional[i], media_constraints, kOptional);
      }
    }
  } else {
    // Log.d(TAG, "optional constraints are not an array");
  }

  return media_constraints;
}

bool FlutterWebRTCBase::CreateIceServers(const Json::Value &iceServersArray,
                                         IceServer *ice_servers) {
  int size = (iceServersArray.isNull()) ? 0 : iceServersArray.size();
  for (int i = 0; i < size; i++) {
    IceServer &ice_server = ice_servers[i];
    Json::Value iceServerMap = iceServersArray[i];
    boolean hasUsernameAndCredential = iceServerMap["username"].isString() &&
                                       iceServerMap["credential"].isString();
    if (iceServerMap["url"].isString()) {
      if (hasUsernameAndCredential) {
        std::string username = iceServerMap["username"].asString();
        std::string credential = iceServerMap["credential"].asString();
        std::string uri = iceServerMap["uri"].asString();
        strncpy(ice_server.username, username.c_str(), username.size());
        strncpy(ice_server.password, credential.c_str(), credential.size());
        strncpy(ice_server.uri, uri.c_str(), uri.size());
      } else {
        std::string uri = iceServerMap["uri"].asString();
        strncpy(ice_server.uri, uri.c_str(), uri.size());
      }
    } else if (!iceServerMap["urls"].isNull()) {
      switch (iceServerMap["urls"].type()) {
        case Json::stringValue:
          if (hasUsernameAndCredential) {
            std::string username = iceServerMap["username"].asString();
            std::string credential = iceServerMap["credential"].asString();
            std::string uri = iceServerMap["uris"].asString();
            strncpy(ice_server.username, username.c_str(), username.size());
            strncpy(ice_server.password, credential.c_str(), credential.size());
            strncpy(ice_server.uri, uri.c_str(), uri.size());
          } else {
            std::string uri = iceServerMap["uris"].asString();
            strncpy(ice_server.uri, uri.c_str(), uri.size());
          }
          break;
        case Json::arrayValue:
          Json::Value urls = iceServerMap["urls"];
          for (Json::ArrayIndex j = 0; j < urls.size(); j++) {
            std::string url = urls[j].asString();
            if (hasUsernameAndCredential) {
              std::string username = iceServerMap["username"].asString();
              std::string credential = iceServerMap["credential"].asString();
              strncpy(ice_server.username, username.c_str(), username.size());
              strncpy(ice_server.password, credential.c_str(),
                      credential.size());
              strncpy(ice_server.uri, url.c_str(), url.size());
            } else {
              strncpy(ice_server.uri, url.c_str(), url.size());
            }
          }
          break;
      }
    }
  }
  return size > 0;
}

bool FlutterWebRTCBase::ParseRTCConfiguration(const Json::Value &map,
                                              RTCConfiguration &conf) {
  if (!map["iceServers"].isNull() && map["iceServers"].isArray()) {
    Json::Value iceServersArray = map["iceServers"];
    CreateIceServers(iceServersArray, conf.ice_servers);
  }
  // iceTransportPolicy (public api)
  if (!map["iceTransportPolicy"].isNull() &&
      map["iceTransportPolicy"].isString()) {
    std::string v = map["iceTransportPolicy"].asString();
    if (v == "all")  // public
      conf.type = kAll;
    else if (v == "relay")
      conf.type = kRelay;
    else if (v == "nohost")
      conf.type = kNoHost;
    else if (v == "none")
      conf.type = kNone;
  }

  // bundlePolicy (public api)
  if (!map["bundlePolicy"].isNull() && map["bundlePolicy"].isString()) {
    std::string v = map["bundlePolicy"].asString();
    if (v == "balanced")  // public
      conf.bundle_policy = kBundlePolicyBalanced;
    else if (v == "max-compat")  // public
      conf.bundle_policy = kBundlePolicyMaxCompat;
    else if (v == "max-bundle")  // public
      conf.bundle_policy = kBundlePolicyMaxBundle;
  }

  // rtcpMuxPolicy (public api)
  if (!map["rtcpMuxPolicy"].isNull() && map["rtcpMuxPolicy"].isString()) {
    std::string v = map["rtcpMuxPolicy"].asString();
    if (v == "negotiate")  // public
      conf.rtcp_mux_policy = kRtcpMuxPolicyNegotiate;
    else if (v == "require")  // public
      conf.rtcp_mux_policy = kRtcpMuxPolicyRequire;
  }

  // FIXME: peerIdentity of type DOMString (public api)
  // FIXME: certificates of type sequence<RTCCertificate> (public api)

  // iceCandidatePoolSize of type unsigned short, defaulting to 0
  if (!map["iceCandidatePoolSize"].isNull() &&
      map["iceCandidatePoolSize"].isInt()) {
    int v = map["iceCandidatePoolSize"].asInt();
    conf.ice_candidate_pool_size = v;
  }

  return true;
}

};  // namespace flutter_webrtc_plugin
