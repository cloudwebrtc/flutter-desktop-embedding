#include "flutter_media_stream.h"

namespace flutter_webrtc_plugin {

void FlutterMediaStream::GetUserMedia(
    const Json::Value *constraints,
    std::unique_ptr<MethodResult<Json::Value>> result) {
  std::string uuid = base_->GenerateUUID();
  scoped_refptr<RTCMediaStream> stream =
      base_->factory_->CreateStream(uuid.c_str());

  scoped_refptr<RTCAudioTrack> audio_track = GetUserAudio(constraints);
  scoped_refptr<RTCVideoTrack> video_track = GetUserVideo(constraints);

  stream->AddTrack(audio_track);
  stream->AddTrack(video_track);

  base_->media_streams_[uuid] = stream;

  Json::Value params;
  params["streamId"] = uuid;

  {
    Json::Value track;
    track["id"] = audio_track->id();
    track["label"] = audio_track->id();
    track["kind"] = audio_track->kind();
    track["enabled"] = audio_track->enabled();
    params["audioTracks"].append(track);
  }

  {
    Json::Value track;
    track["id"] = video_track->id();
    track["label"] = video_track->id();
    track["kind"] = video_track->kind();
    track["enabled"] = video_track->enabled();
    params["videoTracks"].append(track);
  }

  result->Success(&params);
}

scoped_refptr<RTCAudioTrack> FlutterMediaStream::GetUserAudio(
    const Json::Value &constraints) {
  scoped_refptr<RTCAudioSource> source =
      base_->factory_->CreateAudioSource("audioinput");
  std::string uuid = base_->GenerateUUID();
  scoped_refptr<RTCAudioTrack> track =
      base_->factory_->CreateAudioTrack(source, uuid.c_str());
  return track;
}

scoped_refptr<RTCVideoTrack> FlutterMediaStream::GetUserVideo(
    const Json::Value &constraints) {
  scoped_refptr<RTCVideoSource> source =
      base_->factory_->CreateVideoSource(nullptr, "videoinput");
  std::string uuid = base_->GenerateUUID();
  scoped_refptr<RTCVideoTrack> track =
      base_->factory_->CreateVideoTrack(source, uuid.c_str());
  return track;
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
    audio["label"] = std::string(strNameUTF8);
    audio["deviceId"] = std::string(strGuidUTF8);
    audio["facing"] = "";
    audio["kind"] = "audiooutput";
    array.append(audio);
  }

  int nb_video_devices = base_->video_device_->NumberOfDevices();
  for (int i = 0; i < nb_video_devices; i++) {
    base_->video_device_->GetDeviceName(i, strNameUTF8, 128, strGuidUTF8, 128);
    Json::Value video;
    video["label"] = std::string(strNameUTF8);
    video["deviceId"] = std::string(strGuidUTF8);
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
