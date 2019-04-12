#import "FlutterWebRTCPlugin.h"

@interface FlutterRTCVideoRenderer : NSObject <FLETexture, RTCVideoRenderer, FlutterStreamHandler>

/**
 * The {@link RTCVideoTrack}, if any, which this instance renders.
 */
@property (nonatomic, strong) RTCVideoTrack *videoTrack;
@property (nonatomic) int64_t textureId;
@property (nonatomic, weak) id<FLETextureRegistrar> registry;
@property (nonatomic, strong) FlutterEventSink eventSink;

- (instancetype)initWithSize:(CGSize)renderSize;

- (void)dispose;

@end


@interface FlutterWebRTCPlugin (FlutterVideoRendererManager)

- (FlutterRTCVideoRenderer *)createWithSize:(CGSize)size
             withTextureRegistry:(id<FLETextureRegistrar>)registry
                       messenger:(NSObject<FlutterBinaryMessenger>*)messenger;

-(void)setStreamId:(NSString*)streamId view:(FlutterRTCVideoRenderer*)view;

@end
