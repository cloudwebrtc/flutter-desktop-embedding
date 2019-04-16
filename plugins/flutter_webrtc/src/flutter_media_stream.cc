#include "flutter_media_stream.h"

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720
#define DEFAULT_FPS 30

namespace flutter_webrtc_plugin {

void FlutterMediaStream::GetUserMedia(
    const Json::Value &constraints,
    std::unique_ptr<MethodResult<Json::Value>> result) {
  std::string uuid = base_->GenerateUUID();
  scoped_refptr<RTCMediaStream> stream =
      base_->factory_->CreateStream(uuid.c_str());

  
  Json::Value params;
  params["streamId"] = uuid;

  if (!constraints["audio"].isNull()) {
    switch (constraints["audio"].type()) {
      case Json::booleanValue:
        if (constraints["audio"].asBool()) {
          GetUserAudio(constraints, stream, params);
        }
        break;
      case Json::objectValue:
        GetUserAudio(constraints, stream, params);
        break;
      default:
        break;
    }
  }

  if (!constraints["video"].isNull()) {
    switch (constraints["video"].type()) {
      case Json::booleanValue:
        if (constraints["video"].asBool()) {
          GetUserVideo(constraints, stream, params);
        }
        break;
      case Json::objectValue:
        GetUserVideo(constraints, stream, params);
        break;
      default:
        break;
    }
  }
  base_->media_streams_[uuid] = stream;
  result->Success(&params);
}

void addDefaultAudioConstraints(
    scoped_refptr<RTCMediaConstraints> audioConstraints) {
  audioConstraints->AddOptionalConstraint("googNoiseSuppression", "true");
  audioConstraints->AddOptionalConstraint("googEchoCancellation", "true");
  audioConstraints->AddOptionalConstraint("echoCancellation", "true");
  audioConstraints->AddOptionalConstraint("googEchoCancellation2", "true");
  audioConstraints->AddOptionalConstraint("googDAEchoCancellation", "true");
}

void FlutterMediaStream::GetUserAudio(const Json::Value &constraints,
                                      scoped_refptr<RTCMediaStream> stream,
                                      Json::Value &params) {
  scoped_refptr<RTCMediaConstraints> audioConstraints;
  if (constraints["audio"].isBool()) {
    audioConstraints = RTCMediaConstraints::Create();
    addDefaultAudioConstraints(audioConstraints);
  } else {
    audioConstraints = base_->ParseMediaConstraints(constraints["audio"]);
  }

  // TODO: Select audio device by sourceId,

  scoped_refptr<RTCAudioSource> source =
      base_->factory_->CreateAudioSource("audioinput");
  std::string uuid = base_->GenerateUUID();
  scoped_refptr<RTCAudioTrack> track =
      base_->factory_->CreateAudioTrack(source, uuid.c_str());

  Json::Value info;
  info["id"] = track->id();
  info["label"] = track->id();
  info["kind"] = track->kind();
  info["enabled"] = track->enabled();
  params["audioTracks"].append(info);
  stream->AddTrack(track);
}

std::string getFacingMode(const Json::Value &mediaConstraints) {
  return mediaConstraints["facingMode"].isString()
             ? mediaConstraints["facingMode"].asString()
             : "";
}

std::string getSourceIdConstraint(const Json::Value &mediaConstraints) {
  if (!mediaConstraints["optional"].isNull() &&
      mediaConstraints["optional"].isArray()) {
    Json::Value optional = mediaConstraints["optional"];
    for (int i = 0, size = optional.size(); i < size; i++) {
      if (optional[i].isObject()) {
        Json::Value option = optional[i];
        if (!option["sourceId"].isNull() && option["sourceId"].isString()) {
          return option["sourceId"].asString();
        }
      }
    }
  }
  return "";
}

void FlutterMediaStream::GetUserVideo(const Json::Value &constraints,
                                      scoped_refptr<RTCMediaStream> stream,
                                      Json::Value &params) {
  Json::Value video_constraints;
  Json::Value video_mandatory;
  if (constraints["video"].isObject()) {
    Json::Value video_constraints = constraints["video"];
    if (!video_constraints["mandatory"].isNull() &&
        video_constraints["mandatory"].isObject()) {
      video_mandatory = video_constraints["mandatory"];
    }
  }

  std::string facing_mode = getFacingMode(video_constraints);
  boolean isFacing = facing_mode == "" || facing_mode != "environment";
  std::string sourceId = getSourceIdConstraint(video_constraints);
  /*
  int width = video_mandatory["minWidth"].isNumeric()
                  ? video_mandatory["minWidth"].asInt()
                  : DEFAULT_WIDTH;
  int height = video_mandatory["minHeight"].isNumeric()
                   ? video_mandatory["minHeight"].asInt()
                   : DEFAULT_HEIGHT;
  int fps = video_mandatory["minFrameRate"].isNumeric()
                ? video_mandatory["minFrameRate"].asInt()
                : DEFAULT_FPS;
 */
  scoped_refptr<RTCVideoCapturer> video_capturer;
  char strNameUTF8[128];
  char strGuidUTF8[128];
  int nb_video_devices = base_->video_device_->NumberOfDevices();

  for (int i = 0; i < nb_video_devices; i++) {
    base_->video_device_->GetDeviceName(i, strNameUTF8, 128, strGuidUTF8, 128);
    if (sourceId != "" && sourceId == strGuidUTF8) {
      video_capturer = base_->video_device_->Create(strNameUTF8, i);
      break;
	}
  }

  if (nb_video_devices == 0) return;

  if (!video_capturer.get()) {
    base_->video_device_->GetDeviceName(0, strNameUTF8, 128, strGuidUTF8, 128);
    video_capturer = base_->video_device_->Create(strNameUTF8, 0);
  }

  scoped_refptr<RTCVideoSource> source = base_->factory_->CreateVideoSource(
      video_capturer, "videoinput",
      base_->ParseMediaConstraints(video_constraints));

  std::string uuid = base_->GenerateUUID();
  scoped_refptr<RTCVideoTrack> track =
      base_->factory_->CreateVideoTrack(source, uuid.c_str());
  Json::Value info;
  info["id"] = track->id();
  info["label"] = track->id();
  info["kind"] = track->kind();
  info["enabled"] = track->enabled();
  params["videoTracks"].append(info);
  stream->AddTrack(track);
}

void FlutterMediaStream::GetSources(
    std::unique_ptr<MethodResult<Json::Value>> result) {
  Json::Value array;

  int nb_audio_devices = base_->audio_device_->RecordingDevices();
  char strNameUTF8[128];
  char strGuidUTF8[128];

  for (int i = 0; i < nb_audio_devices; i++) {
    base_->audio_device_->RecordingDeviceName(i, strNameUTF8, strGuidUTF8);
    Json::Value audio;
    audio["label"] = std::string(strNameUTF8);
    audio["deviceId"] = std::string(strGuidUTF8);
    audio["facing"] = "";
    audio["kind"] = "audioinput";
    array.append(audio);
  }

  nb_audio_devices = base_->audio_device_->PlayoutDevices();
  for (int i = 0; i < nb_audio_devices; i++) {
    base_->audio_device_->PlayoutDeviceName(i, strNameUTF8, strGuidUTF8);
    Json::Value audio;
    audio["label"] = std::string(strGuidUTF8);
    audio["deviceId"] = std::string(strNameUTF8);
    audio["facing"] = "";
    audio["kind"] = "audiooutput";
    array.append(audio);
  }

  int nb_video_devices = base_->video_device_->NumberOfDevices();
  for (int i = 0; i < nb_video_devices; i++) {
    base_->video_device_->GetDeviceName(i, strNameUTF8, 128, strGuidUTF8, 128);
    Json::Value video;
    video["label"] = std::string(strGuidUTF8);
    video["deviceId"] = std::string(strNameUTF8);
    video["facing"] = i == 1 ? "front" : "back";
    video["kind"] = "videoinput";
    array.append(video);
  }
  result->Success(&array);
}

void FlutterMediaStream::MediaStreamGetTracks(
    const std::string &stream_id,
    std::unique_ptr<MethodResult<Json::Value>> result) {
  scoped_refptr<RTCMediaStream> stream = base_->MediaStreamForId(stream_id);

  if (stream) {
    Json::Value params;
    Json::Value audioTracks;
    for (auto track : stream->GetAudioTracks()) {
      Json::Value info;
      info["id"] = track->id();
      info["label"] = track->id();
      info["kind"] = track->kind();
      info["enabled"] = track->enabled();
      info["remote"] = true;
      info["readyState"] = "live";
      audioTracks.append(info);
    }
    params["audioTracks"] = audioTracks;

    Json::Value videoTracks;
    for (auto track : stream->GetVideoTracks()) {
      Json::Value info;
      info["id"] = track->id();
      info["label"] = track->id();
      info["kind"] = track->kind();
      info["enabled"] = track->enabled();
      info["remote"] = true;
      info["readyState"] = "live";
      videoTracks.append(info);
    }
    params["videoTracks"] = videoTracks;

    result->Success(&params);
  } else {
    result->Error("MediaStreamGetTracksFailed",
                  "MediaStreamGetTracks() media stream is null !");
  }
}

void FlutterMediaStream::MediaStreamDispose(
    const std::string &stream_id,
    std::unique_ptr<MethodResult<Json::Value>> result) {}

void FlutterMediaStream::MediaStreamTrackSetEnable(
    const std::string &track_id,
    std::unique_ptr<MethodResult<Json::Value>> result) {}

void FlutterMediaStream::MediaStreamTrackSwitchCamera(
    const std::string &track_id,
    std::unique_ptr<MethodResult<Json::Value>> result) {}

void FlutterMediaStream::MediaStreamTrackDispose(
    const std::string &track_id,
    std::unique_ptr<MethodResult<Json::Value>> result) {}
};  // namespace flutter_webrtc_plugin
