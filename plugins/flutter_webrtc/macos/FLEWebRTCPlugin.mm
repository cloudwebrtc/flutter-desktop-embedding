#import "FLEWebRTCPlugin.h"
#import "FlutterWebRTCPlugin.h"

@implementation FLEWebRTCPlugin

+ (void)registerWithRegistrar:(nonnull id<FLEPluginRegistrar>)registrar{
    FlutterMethodChannel* channel = [FlutterMethodChannel
                                 methodChannelWithName:@"cloudwebrtc.com/WebRTC.Method"
                                 binaryMessenger:registrar.messenger
                                 codec:[FlutterJSONMethodCodec sharedInstance]];
    FlutterWebRTCPlugin* instance = [[FlutterWebRTCPlugin alloc] initWithPluginRegistrar:registrar channel:channel];
    [registrar addMethodCallDelegate:instance channel:channel];
}


@end
