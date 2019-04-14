#ifndef FLUTTER_WEBRTC_RTC_VIDEO_RENDERER_HXX
#define FLUTTER_WEBRTC_RTC_VIDEO_RENDERER_HXX

#include "flutter_webrtc_base.h"

#include "rtc_video_frame.h"
#include "rtc_video_renderer.h"

namespace flutter_webrtc_plugin {

using namespace libwebrtc;
using namespace flutter;

class FlutterVideoRenderer : public Texture,
                             public RTCVideoRenderer<RTCVideoFrame> {
 public:
  FlutterVideoRenderer(TextureRegistrar *registrar, BinaryMessenger *messenger,
                       int64_t texture_id);

  virtual std::shared_ptr<uint8_t> CopyTextureBuffer(size_t width,
                                                     size_t height) override;

  virtual void OnFrame(RTCVideoFrame &frame) override;

  void SetVideoTrack(RTCVideoTrack *track);

 private:
  struct FrameSize {
    int width;
    int height;
  };
  FrameSize size_ = {0, 0};
  TextureRegistrar *registrar_ = nullptr;
  std::unique_ptr<EventChannel<Json::Value>> event_channel_;
  const EventSink<Json::Value> *event_sink_ = nullptr;
  int64_t texture_id_ = -1;
  RTCVideoTrack *track_ = nullptr;
  std::shared_ptr<uint8_t> frame_buffer_;
};

class FlutterVideoRendererManager {
 public:
  FlutterVideoRendererManager(FlutterWebRTCBase *base);

  void CreateVideoRendererTexture(
      std::unique_ptr<MethodResult<Json::Value>> result);

  void SetMediaStream(int64_t texture_id, const std::string &stream_id);

  void VideoRendererDispose(int64_t texture_id,
                            std::unique_ptr<MethodResult<Json::Value>> result);

 private:
  FlutterWebRTCBase *base_;
  int64_t texture_counter_ = -1;
  std::map<int64_t, std::unique_ptr<FlutterVideoRenderer>> renderers_;
};

};  // namespace flutter_webrtc_plugin

#endif  // !FLUTTER_WEBRTC_RTC_VIDEO_RENDERER_HXX