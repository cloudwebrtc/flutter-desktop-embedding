#ifndef LIB_WEBRTC_RTC_TYPES_HXX
#define LIB_WEBRTC_RTC_TYPES_HXX

#ifdef LIB_WEBRTC_API_EXPORTS
#define LIB_WEBRTC_API __declspec(dllexport)
#elif defined(LIB_WEBRTC_API_DLL)
#define LIB_WEBRTC_API __declspec(dllimport)
#elif !defined(WIN32)
#define LIB_WEBRTC_API __attribute__((visibility("default")))
#else
#define LIB_WEBRTC_API
#endif

#include "base/refcount.h"
#include "base/scoped_ref_ptr.h"

#include <stdint.h>
#include <string>
#include <vector>

namespace libwebrtc {

enum MediaSecurityType { kSRTP_None = 0, kSDES_SRTP, kDTLS_SRTP };

enum { kShortStringLength = 16, kMaxStringLength = 256, kMaxIceServerSize = 8 };

struct IceServer {
  char uri[kMaxStringLength];
  char username[kMaxStringLength];
  char password[kMaxStringLength];
};

struct RTCConfiguration {
  MediaSecurityType srtp_type = kDTLS_SRTP;
  IceServer ice_servers[kMaxIceServerSize];
  bool offer_to_receive_audio = true;
  bool offer_to_receive_video = true;
  bool use_rtp_mux = true;
  uint32_t local_audio_bandwidth = 128;
  uint32_t local_video_bandwidth = 512;
};

struct SdpParseError {
 public:
  // The sdp line that causes the error.
  char line[kMaxStringLength];
  // Explains the error.
  char description[kMaxStringLength];
};

};  // namespace libwebrtc

#endif //LIB_WEBRTC_RTC_TYPES_HXX
