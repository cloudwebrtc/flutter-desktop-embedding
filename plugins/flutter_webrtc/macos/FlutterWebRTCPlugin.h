
#import <FlutterMacOS/FlutterMacOS.h>
#import "FlutterWebRTC.h"

#import <objc/runtime.h>

@class FlutterRTCVideoRenderer;

@interface FlutterWebRTCPlugin : NSObject<FLEPlugin, RTCPeerConnectionDelegate>

+ (void)registerWithRegistrar:(nonnull id<FLEPluginRegistrar>)registrar;

@property (nonatomic, strong) RTCPeerConnectionFactory *peerConnectionFactory;
@property (nonatomic, strong) NSMutableDictionary<NSString *, RTCPeerConnection *> *peerConnections;
@property (nonatomic, strong) NSMutableDictionary<NSString *, RTCMediaStream *> *localStreams;
@property (nonatomic, strong) NSMutableDictionary<NSString *, RTCMediaStreamTrack *> *localTracks;
@property (nonatomic, strong) NSMutableDictionary<NSNumber *, FlutterRTCVideoRenderer *> *renders;
@property (nonatomic, strong) NSObject<FlutterBinaryMessenger>* messenger;
@property (nonatomic, strong) RTCCameraVideoCapturer *videoCapturer;
@property (nonatomic) BOOL _usingFrontCamera;
@property (nonatomic) int _targetWidth;
@property (nonatomic) int _targetHeight;
@property (nonatomic) int _targetFps;

- (instancetype)initWithPluginRegistrar:(id<FLEPluginRegistrar>)registrar
                                channel:(FlutterMethodChannel *)channel;

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult) result;

- (RTCMediaStream*)streamForId:(NSString*)streamId;

@end
