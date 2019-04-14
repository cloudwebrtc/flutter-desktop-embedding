#include "flutter_data_channel.h"

namespace flutter_webrtc_plugin {

FlutterRTCDataChannelObserver::FlutterRTCDataChannelObserver(
    scoped_refptr<RTCDataChannel> data_channel, BinaryMessenger *messenger,
    const std::string &name)
    : data_channel_(data_channel),
      event_channel_(new EventChannel<Json::Value>(
          messenger, name, &JsonMethodCodec::GetInstance(),
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

FlutterRTCDataChannelObserver::~FlutterRTCDataChannelObserver() {}

void FlutterDataChannel::CreateDataChannel(
    const std::string &label, const Json::Value &dataChannelDict,
    RTCPeerConnection *pc, std::unique_ptr<MethodResult<Json::Value>> result) {
  RTCDataChannelInit init;
  init.id = dataChannelDict["id"].asInt();
  init.ordered = dataChannelDict["ordered"].asBool();
  init.maxRetransmitTime = dataChannelDict["maxRetransmitTime"].asInt();
  init.maxRetransmits = dataChannelDict["maxRetransmits"].asInt();
  std::string protocol = {dataChannelDict["protocol"].isNull()
                              ? "sctp"
                              : dataChannelDict["protocol"].asCString()};

  strncpy(init.protocol, protocol.c_str(), protocol.size());

  init.negotiated = dataChannelDict["negotiated"].asBool();

  scoped_refptr<RTCDataChannel> data_channel =
      pc->CreateDataChannel(label.c_str(), &init);

  std::string event_channel = "cloudwebrtc.com/WebRTC/dataChannelEvent" +
                              std::to_string(data_channel->id());

  std::unique_ptr<FlutterRTCDataChannelObserver> observer(
      new FlutterRTCDataChannelObserver(data_channel, base_->messenger_,
                                        event_channel));

  base_->data_channel_observers_[data_channel->id()] = std::move(observer);

  Json::Value params;
  params["id"] = data_channel->id();
  params["label"] = data_channel->label();
  result->Success(&params);
}

void FlutterDataChannel::DataChannelSend(
    RTCDataChannel *data_channel, const std::string &type,
    const std::string &data,
    std::unique_ptr<MethodResult<Json::Value>> result) {
  data_channel->Send(data.data(), (int)data.size(), type == "binary");
  result->Success(nullptr);
}

void FlutterDataChannel::DataChannelClose(
    RTCDataChannel *data_channel,
    std::unique_ptr<MethodResult<Json::Value>> result) {
  int id = data_channel->id();
  data_channel->Close();
  auto it = base_->data_channel_observers_.find(id);
  if (it != base_->data_channel_observers_.end())
    base_->data_channel_observers_.erase(it);
  result->Success(nullptr);
}

RTCDataChannel *FlutterDataChannel::DataChannelFormId(int id) {
  auto it = base_->data_channel_observers_.find(id);

  if (it != base_->data_channel_observers_.end()) {
    FlutterRTCDataChannelObserver *observer = it->second.get();
    scoped_refptr<RTCDataChannel> data_channel = observer->data_channel();
    return data_channel.get();
  }
  return nullptr;
}

static const char *DataStateString(RTCDataChannelState state) {
  switch (state) {
    case RTCDataChannelConnecting:
      return "connecting";
    case RTCDataChannelOpen:
      return "open";
    case RTCDataChannelClosing:
      return "closing";
    case RTCDataChannelClosed:
      return "closed";
  }
  return "";
}

void FlutterRTCDataChannelObserver::OnStateChange(RTCDataChannelState state) {
  if (event_sink_ != nullptr) {
    Json::Value params;
    params["event"] = "dataChannelReceiveMessage";
    params["id"] = data_channel_->id();
    params["state"] = DataStateString(state);
    (*event_sink_)(&params);
  }
}

void FlutterRTCDataChannelObserver::OnMessage(const char *buffer, int length,
                                              bool binary) {
  if (event_sink_ != nullptr) {
    Json::Value params;
    params["event"] = "dataChannelReceiveMessage";
    params["id"] = data_channel_->id();
    params["type"] = binary ? "binary" : "text";
    params["data"] = std::string(buffer, length);
    (*event_sink_)(&params);
  }
}
};  // namespace flutter_webrtc_plugin
