#ifndef FLUTTER_WEBRTC_RTC_DATA_CHANNEL_HXX
#define FLUTTER_WEBRTC_RTC_DATA_CHANNEL_HXX

#include "flutter_webrtc_base.h"

namespace flutter_webrtc_plugin {

class FlutterRTCDataChannelObserver : public RTCDataChannelObserver {
 public:
  FlutterRTCDataChannelObserver(scoped_refptr<RTCDataChannel> data_channel,
                                BinaryMessenger *messenger,
                                const std::string &channel_name);
  virtual ~FlutterRTCDataChannelObserver();

  virtual void OnStateChange(RTCDataChannelState state) override;

  virtual void OnMessage(const char *buffer, int length, bool binary) override;

  scoped_refptr<RTCDataChannel> data_channel() { return data_channel_; }

 private:
  std::unique_ptr<EventChannel<Json::Value>> event_channel_;
  const EventSink<Json::Value> *event_sink_ = nullptr;
  scoped_refptr<RTCDataChannel> data_channel_;
};

class FlutterDataChannel {
 public:
  FlutterDataChannel(FlutterWebRTCBase *base) : base_(base) {}

  void CreateDataChannel(const std::string &label,
                         const Json::Value &dataChannelDict,
                         RTCPeerConnection *pc,
                         std::unique_ptr<MethodResult<Json::Value>>);

  void DataChannelSend(RTCDataChannel *data_channel, const std::string &type,
                       const std::string &data,
                       std::unique_ptr<MethodResult<Json::Value>>);

  void DataChannelClose(RTCDataChannel *data_channel,
                        std::unique_ptr<MethodResult<Json::Value>>);

  RTCDataChannel *DataChannelFormId(int id);

 private:
  FlutterWebRTCBase *base_;
};

};  // namespace flutter_webrtc_plugin

#endif  // !FLUTTER_WEBRTC_RTC_DATA_CHANNEL_HXX