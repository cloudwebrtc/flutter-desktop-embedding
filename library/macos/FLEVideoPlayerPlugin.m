//
//  FLEVideoPlayerPlugin.m
//  FlutterEmbedderMac
//
//  Created by Chinmay Garde on 11/30/18.
//  Copyright © 2018 Google LLC. All rights reserved.
//

#define COREVIDEO_SILENCE_GL_DEPRECATION 1

#import "FLEVideoPlayerPlugin.h"
#import "FLEViewController.h"
#import "FLEViewController+Internal.h"
#import <FlutterEmbedder/FlutterEmbedder.h>
#import <OpenGL/gl.h>

@import Foundation;
@import AVFoundation;
@import CoreVideo;

@interface FLEVideoPlayer : NSObject <FLEPlugin>

-(instancetype) initWithURL:(nonnull NSURL* )url controller:(FLEViewController *)controller;

@property (nonatomic, readonly) NSInteger identifier;
@property (nonatomic) BOOL looping;
@property (nonatomic) BOOL volume;

-(void) play;

-(void) pause;

- (int64_t)position;

- (int64_t)duration;

-(BOOL) populateTextureWidth:(size_t) width height:(size_t) height texture:(FlutterOpenGLTexture*) texture;

@end

@interface FLEVideoPlayerPlugin() <FLETextureDelegate>

@end

@implementation FLEVideoPlayer {
  AVPlayerItem *_playerItem;
  AVPlayer *_player;
  AVPlayerItemVideoOutput *_videoOutput;
  CVDisplayLinkRef _displayLink;
  NSUInteger _frameCount;
  CVOpenGLTextureCacheRef _textureCache;
}

@synthesize controller = _controller;
@synthesize channel = _channel;
@synthesize looping = _looping;
@synthesize volume = _volume;

-(instancetype) initWithURL:(nonnull NSURL* )url controller:(FLEViewController *)controller {
  self = [super init];

  if (self) {
    _controller = controller;
    _channel = [NSString stringWithFormat:@"flutter.io/videoPlayer/videoEvents%zu",
                self.textureIdentifier];
    BOOL result = [_controller addPlugin:self];
    NSAssert(result, @"Must be able to add the view plugin channel.");
    result = [_controller registerTexture:self.textureIdentifier];
    NSAssert(result, @"Must be able to register the texture.");

    [self initializePlayerWithURL:url];
  }

  return self;
}

-(void) initializePlayerWithURL:(nonnull NSURL* )url {
  _playerItem = [AVPlayerItem playerItemWithURL:url];
  NSAssert(_playerItem != NULL, @"");
  [_playerItem addObserver:self forKeyPath:@"status" options:NSKeyValueObservingOptionNew | NSKeyValueObservingOptionInitial context:(__bridge void * _Nullable)(self)];
  NSDictionary* pixBuffAttributes = @{
                                      (id)kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_32BGRA),
                                      (id)kCVPixelBufferIOSurfacePropertiesKey : @{}
                                      };
  _videoOutput = [[AVPlayerItemVideoOutput alloc] initWithPixelBufferAttributes:pixBuffAttributes];
  [_playerItem addOutput:_videoOutput];
  _player = [AVPlayer playerWithPlayerItem:_playerItem];
  _player.actionAtItemEnd = AVPlayerActionAtItemEndNone;
}

int64_t FLTCMTimeToMillis(CMTime time) { return time.value * 1000 / time.timescale; }

static void OnGLTextureRelease(CVPixelBufferRef pixelBuffer) {
  CVPixelBufferRelease(pixelBuffer);
}

-(BOOL) populateTextureWidth:(size_t) width height:(size_t) height texture:(FlutterOpenGLTexture*) texture {

  // Get the video frame.
  CVPixelBufferRef videoFramePixelBuffer = NULL;
  CMTime outputItemTime = [_videoOutput itemTimeForHostTime:CACurrentMediaTime()];
  if ([_videoOutput hasNewPixelBufferForItemTime:outputItemTime]) {
    videoFramePixelBuffer = [_videoOutput copyPixelBufferForItemTime:outputItemTime itemTimeForDisplay:NULL];
  } else {
    return NO;
  }

  if (videoFramePixelBuffer == NULL) {
    return NO;
  }

  // Create the texture cache if necessary.
  if (_textureCache == NULL) {
    CGLContextObj context = [NSOpenGLContext currentContext].CGLContextObj;
    CGLPixelFormatObj format = CGLGetPixelFormat(context);
    if (CVOpenGLTextureCacheCreate(kCFAllocatorDefault, NULL, context, format, NULL, &_textureCache) != kCVReturnSuccess) {
      NSLog(@"Could not create texture cache.");
      CVPixelBufferRelease(videoFramePixelBuffer);
      return NO;
    }
  }

  CVOpenGLTextureRef openGLTexture = NULL;

  if (CVOpenGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault, _textureCache, videoFramePixelBuffer, NULL, &openGLTexture) != kCVReturnSuccess) {
    CVPixelBufferRelease(videoFramePixelBuffer);
    return NO;
  }

  texture->target = CVOpenGLTextureGetTarget(openGLTexture);
  texture->name = CVOpenGLTextureGetName(openGLTexture);
  texture->format = GL_RGBA8;
  texture->destruction_callback = (VoidCallback)&OnGLTextureRelease;
  texture->user_data = openGLTexture;

  CVPixelBufferRelease(videoFramePixelBuffer);
  return YES;
}

- (int64_t)position {
  return FLTCMTimeToMillis([_player currentTime]);
}

- (int64_t)duration {
  return FLTCMTimeToMillis([[_player currentItem] duration]);
}

-(void) play {

}

-(void) pause {

}

- (void)observeValueForKeyPath:(NSString*)path
                      ofObject:(id)object
                        change:(NSDictionary*)change
                       context:(void*)context {
  if ([path isEqualToString:@"status"]) {
    if (_playerItem == nil) {
      return;
    }
    if (_playerItem.status == AVPlayerItemStatusReadyToPlay) {
      [_player play];
      [self startDisplayLink];

      CGSize size = _player.currentItem.presentationSize;
      [_controller dispatchMessage:@[@{
                                     @"event" : @"initialized",
                                     @"duration" : @(self.duration),
                                     @"width" : @(size.width),
                                     @"height" : @(size.height),
                                     }] onChannel:_channel];
    } else {
      NSLog(@"Could not initialize video player for playback.");
    }
  }
}

-(void) dealloc {
  CVOpenGLBufferRelease(_textureCache);
  [self stopDisplayLink];
  [_playerItem removeObserver:self forKeyPath:@"status"];
  [_controller unregisterTexture:self.textureIdentifier];
}

-(void) notifyFrameAvailable {
  if (!_playerItem || _playerItem.status != AVPlayerItemStatusReadyToPlay) {
    return;
  }
  if (![_controller markTextureFrameAvailable:self.textureIdentifier]) {
    NSLog(@"Could not mark texture frame avaialable.");
  }
}

static CVReturn OnDisplayLink(CVDisplayLinkRef CV_NONNULL displayLink,
                              const CVTimeStamp * CV_NONNULL inNow,
                              const CVTimeStamp * CV_NONNULL inOutputTime,
                              CVOptionFlags flagsIn,
                              CVOptionFlags * CV_NONNULL flagsOut,
                              void * CV_NULLABLE displayLinkContext) {
  __weak FLEVideoPlayer* video_player = (__bridge FLEVideoPlayer *)(displayLinkContext);
  dispatch_async(dispatch_get_main_queue(), ^{
    [video_player notifyFrameAvailable];
  });
  return kCVReturnSuccess;
}

-(void) startDisplayLink {
  if (_displayLink != NULL) {
    return;
  }

  if (CVDisplayLinkCreateWithCGDisplay(CGMainDisplayID(), &_displayLink) != kCVReturnSuccess) {
    _displayLink = NULL;
    return;
  }

  CVDisplayLinkSetOutputCallback(_displayLink, OnDisplayLink, (__bridge void *)self);
  CVDisplayLinkStart(_displayLink);
}

-(void) stopDisplayLink {
  if (_displayLink == NULL) {
    return;
  }

  CVDisplayLinkStop(_displayLink);
  CVDisplayLinkRelease(_displayLink);

}

-(NSInteger) textureIdentifier {
  return (NSInteger)(self);
}

- (void)handleMethodCall:(nonnull FLEMethodCall *)call result:(nonnull FLEMethodResult)result {
  if ([call.methodName isEqualToString:@"listen"]) {
    result(nil);
  } else {
    NSAssert(NO, @"Unhandled method.");
    result(nil);
  }
}

@end

@implementation FLEVideoPlayerPlugin {
  NSMutableDictionary<NSNumber *, FLEVideoPlayer*> *_videoPlayers;
}

@synthesize controller=_controller;

-(instancetype) init {
  self = [super init];

  if (self) {
    _videoPlayers = [[NSMutableDictionary alloc] init];
  }

  return self;
}

-(void) dealloc {

}

-(NSString *) channel {
  return @"flutter.io/videoPlayer";
}


-(BOOL) populateTextureWithIdentifier:(int64_t) texture_identifier width:(size_t) width height:(size_t) height texture:(FlutterOpenGLTexture*) texture {
  return [_videoPlayers[@(texture_identifier)] populateTextureWidth:width height:height texture:texture];
}

- (void)handleMethodCall:(nonnull FLEMethodCall *)call result:(nonnull FLEMethodResult)result {
  if ([@"init" isEqualToString:call.methodName]) {
    [_videoPlayers removeAllObjects];
    NSAssert(_controller, @"");
    _controller.textureDelegate = self;
    result(nil);
  } else if ([@"create" isEqualToString:call.methodName]) {
    NSURL *url = [NSURL URLWithString:((NSDictionary*)call.arguments)[@"uri"]];
    FLEVideoPlayer *player = [[FLEVideoPlayer alloc] initWithURL:url controller:self.controller];
    _videoPlayers[@(player.textureIdentifier)] = player;
    result(@{@"textureId" : @(player.textureIdentifier)});
  } else {
    NSDictionary* argsMap = (NSDictionary*)call.arguments;
    int64_t textureId = ((NSNumber*)argsMap[@"textureId"]).unsignedIntegerValue;
    FLEVideoPlayer* player = _videoPlayers[@(textureId)];
    if ([@"dispose" isEqualToString:call.methodName]) {
      [player pause];
      [_videoPlayers removeObjectForKey:@(player.textureIdentifier)];
      result(nil);
    } else if ([@"setLooping" isEqualToString:call.methodName]) {
      player.looping = [argsMap[@"looping"] boolValue];
      result(nil);
    } else if ([@"setVolume" isEqualToString:call.methodName]) {
      player.volume = [argsMap[@"volume"] doubleValue];
      result(nil);
    } else if ([@"play" isEqualToString:call.methodName]) {
      [player play];
      result(nil);
    } else if ([@"position" isEqualToString:call.methodName]) {
      result(@([player position]));
    } else if ([@"seekTo" isEqualToString:call.methodName]) {
//      NSAssert(NO, @"");
      result(nil);
    } else if ([@"pause" isEqualToString:call.methodName]) {
      [player pause];
      result(nil);
    } else {
      NSAssert(NO, @"");
    }
  }
}

@end
