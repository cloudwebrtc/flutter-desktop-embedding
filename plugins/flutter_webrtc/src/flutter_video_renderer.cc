#include "flutter_video_renderer.h"

namespace flutter_webrtc_plugin {
FlutterVideoRenderer::FlutterVideoRenderer(TextureRegistrar *registrar,
                                           BinaryMessenger *messenger,
                                           int64_t texture_id)
    : registrar_(registrar), texture_id_(texture_id) {
  std::string event_channel =
      "cloudwebrtc.com/WebRTC/Texture" + std::to_string(texture_id);

  event_channel_.reset(new EventChannel<Json::Value>(
      messenger, event_channel, &JsonMethodCodec::GetInstance(),
      &JsonMessageCodec::GetInstance()));

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
std::shared_ptr<uint8_t> FlutterVideoRenderer::CopyTextureBuffer(
    size_t width, size_t height) {
  return frame_buffer_;
}

void FlutterVideoRenderer::OnFrame(RTCVideoFrame &frame) {
  if (size_.width != frame.width() || size_.height != frame.height()) {
    size_t buffer_size = frame.width() * frame.height() * (32 >> 3);
    frame_buffer_.reset(new uint8_t[buffer_size]);
    size_ = {frame.width(), frame.height()};
  }
  frame.ConvertToARGB(RTCVideoFrame::Type::kRGBA, frame_buffer_.get(), 0);
  registrar_->MarkTextureFrameAvailable(texture_id_);
}

void FlutterVideoRenderer::SetVideoTrack(RTCVideoTrack *track) {
  if (track_ != track) {
    if (track_) track_->RemoveRenderer(this);
    track_ = track;
    if (track_) track_->AddRenderer(this);
  }
}

FlutterVideoRendererManager::FlutterVideoRendererManager(
    FlutterWebRTCBase *base)
    : base_(base) {}

void FlutterVideoRendererManager::CreateVideoRendererTexture(
    std::unique_ptr<MethodResult<Json::Value>> result) {
  int64_t texture_id = ++texture_counter_;
  std::unique_ptr<FlutterVideoRenderer> texture(new FlutterVideoRenderer(
      base_->textures_, base_->messenger_, texture_id));
  base_->textures_->RegisterTexture(texture.get());
  renderers_[texture_id] = std::move(texture);
}

void FlutterVideoRendererManager::SetMediaStream(int64_t texture_id,
                                                 const std::string &stream_id) {
  scoped_refptr<RTCMediaStream> stream = base_->MediaStreamForId(stream_id);

  auto it = renderers_.find(texture_id);
  if (it != renderers_.end()) {
    FlutterVideoRenderer *renderer = it->second.get();
    if (stream.get()) {
      VideoTrackVector tracks = stream->GetVideoTracks();
      if (tracks.size() > 0) {
        renderer->SetVideoTrack(tracks[0].get());
      }
    } else {
      renderer->SetVideoTrack(nullptr);
    }
  }
}

void FlutterVideoRendererManager::VideoRendererDispose(
    int64_t texture_id, std::unique_ptr<MethodResult<Json::Value>> result) {
  auto it = renderers_.find(texture_id);
  if (it != renderers_.end()) {
    base_->textures_->UnregisterTexture(texture_id);
    renderers_.erase(it);
    result->Success(nullptr);
    return;
  }
  result->Error("VideoRendererDisposeFailed",
                "VideoRendererDispose() texture not found!");
}

};  // namespace flutter_webrtc_plugin