#ifndef FLUTTER_WEBRTC_RTC_GET_USERMEDIA_HXX
#define FLUTTER_WEBRTC_RTC_GET_USERMEDIA_HXX

#include "flutter_webrtc_base.h"

namespace flutter_webrtc_plugin {

using namespace flutter;

class FlutterMediaStream {
 public:
  FlutterMediaStream(FlutterWebRTCBase *base) : base_(base) {}

  void GetUserMedia(const Json::Value& constraints,
                    std::unique_ptr<MethodResult<Json::Value>> result);

  void GetUserAudio(const Json::Value &constraints,
                    scoped_refptr<RTCMediaStream> stream, Json::Value& params);

  void GetUserVideo(const Json::Value &constraints,
                    scoped_refptr<RTCMediaStream> stream, Json::Value &params);

  void GetSources(std::unique_ptr<MethodResult<Json::Value>> result);

  void MediaStreamGetTracks(const std::string &stream_id,
                            std::unique_ptr<MethodResult<Json::Value>> result);

  void MediaStreamDispose(const std::string &stream_id,
                          std::unique_ptr<MethodResult<Json::Value>> result);

  void MediaStreamTrackSetEnable(
      const std::string &track_id,
      std::unique_ptr<MethodResult<Json::Value>> result);

  void MediaStreamTrackSwitchCamera(
      const std::string &track_id,
      std::unique_ptr<MethodResult<Json::Value>> result);

  void MediaStreamTrackDispose(
      const std::string &track_id,
      std::unique_ptr<MethodResult<Json::Value>> result);

 private:
  FlutterWebRTCBase *base_;
};

};  // namespace flutter_webrtc_plugin

#endif  // !FLUTTER_WEBRTC_RTC_GET_USERMEDIA_HXX
