#include "flutter_video_renderer.h"

namespace flutter_webrtc_plugin {
FlutterVideoRenderer::FlutterVideoRenderer(TextureRegistrar *registrar,
                                           BinaryMessenger *messenger)
    : registrar_(registrar) {
  texture_id_ = registrar_->RegisterTexture(this);
  std::string event_channel =
      "cloudwebrtc.com/WebRTC/Texture" + std::to_string(texture_id_);
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
  if (dest_frame_size_.width != width || dest_frame_size_.height != height) {
    size_t buffer_size = (width * height) * (32 >> 3);
    frame_buffer_.reset(new uint8_t[buffer_size]);
    dest_frame_size_ = {width, height};
  }
  frame_->ConvertToARGB(RTCVideoFrame::Type::kABGR, frame_buffer_.get(), 0,
                        (int)width, (int)height);
  return frame_buffer_;
}

void FlutterVideoRenderer::OnFrame(scoped_refptr<RTCVideoFrame> frame) {

  if (!first_frame_rendered && event_sink_) {
      Json::Value params;
      params["event"] = "didFirstFrameRendered";
      (*event_sink_)(&params);
      first_frame_rendered = true;
  }
 
  if (frame_size_.width != frame->width() ||
      frame_size_.height != frame->height()) {
    if (event_sink_) {
      Json::Value params;
      params["event"] = "didTextureChangeVideoSize";
      params["id"] = texture_id_;
      params["width"] = 0.0 + frame_size_.width;
      params["height"] = 0.0 + frame_size_.height;
      (*event_sink_)(&params);
    }
    frame_size_ = {(size_t)frame->width(), (size_t)frame->height()};
  }

  frame_ = frame;
  registrar_->MarkTextureFrameAvailable(texture_id_);
}

void FlutterVideoRenderer::SetVideoTrack(RTCVideoTrack *track) {
  if (track_ != track) {
    if (track_) track_->RemoveRenderer(this);
    track_ = track;
    frame_size_ = {0, 0};
    dest_frame_size_ = {0, 0};
    first_frame_rendered = false;
    if (track_) track_->AddRenderer(this);
  }
}

FlutterVideoRendererManager::FlutterVideoRendererManager(
    FlutterWebRTCBase *base)
    : base_(base) {}

void FlutterVideoRendererManager::CreateVideoRendererTexture(
    std::unique_ptr<MethodResult<Json::Value>> result) {
  std::unique_ptr<FlutterVideoRenderer> texture(
      new FlutterVideoRenderer(base_->textures_, base_->messenger_));
  int64_t texture_id = texture->texture_id();
  renderers_[texture_id] = std::move(texture);
  Json::Value params;
  params["textureId"] = texture_id;
  result->Success(&params);
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