#pragma once

#import <QuartzCore/CADisplayLink.h>
#import <ARKit/ARKit.h>
#import <CoreVideo/CoreVideo.h>
#include "stdlib.h"
#include "RenderPluginDelegate.h"

@class UnityView;
@class UnityViewControllerBase;
@class DisplayConnection;

@interface UnityAppController : NSObject<UIApplicationDelegate>
{
    UnityView*          _unityView;
    CADisplayLink*      _displayLink;

    UIWindow*           _window;
    UIView*             _snapshotView;

    DisplayConnection*  _mainDisplay;

    id<RenderPluginDelegate>    _renderDelegate;
    
#if UNITY_SUPPORT_ROTATION
    UIInterfaceOrientation  _curOrientation;
#endif
}

+ (UnityAppController *)sharedInstance;

// override it to add your render plugin delegate
- (void)shouldAttachRenderDelegate;

// this one is called at first applicationDidBecomeActive
// NB: it will be started with delay 0, so it will run on next run loop iteration
// this is done to make sure that activity indicator animation starts before blocking loading
- (void)startUnity;

- (void)initUnity:(CGRect)frame;

- (void)refreshUnityView;

// this is a part of UIApplicationDelegate protocol starting with ios5
// setter will be generated empty
@property (retain, nonatomic) UIWindow* window;

@property (readonly, copy, nonatomic) UnityView*            unityView;
@property (readonly, copy, nonatomic) CADisplayLink*        unityDisplayLink;

@property (readonly, nonatomic) UIView*                     rootView;
@property (readonly, nonatomic) UIViewController*           rootViewController;
@property (readonly, copy, nonatomic) DisplayConnection*    mainDisplay;

#if UNITY_SUPPORT_ROTATION
@property (readonly, nonatomic) UIInterfaceOrientation      interfaceOrientation;
#endif

@property (nonatomic, retain) id                            renderDelegate;
@property (nonatomic, copy)                                 void(^quitHandler)();

@end

// Put this into mm file with your subclass implementation
// pass subclass name to define

#define IMPL_APP_CONTROLLER_SUBCLASS(ClassName) \
@interface ClassName(OverrideAppDelegate)       \
{                                               \
}                                               \
+(void)load;                                    \
@end                                            \
@implementation ClassName(OverrideAppDelegate)  \
+(void)load                                     \
{                                               \
    extern const char* AppControllerClassName;  \
    AppControllerClassName = #ClassName;        \
}                                               \
@end                                            \

inline UnityAppController*  GetAppController()
{
    return UnityAppController.sharedInstance;
}

#define APP_CONTROLLER_RENDER_PLUGIN_METHOD(method)                         \
do {                                                                        \
    id<RenderPluginDelegate> delegate = GetAppController().renderDelegate;  \
    if([delegate respondsToSelector:@selector(method)])                     \
        [delegate method];                                                  \
} while(0)

#define APP_CONTROLLER_RENDER_PLUGIN_METHOD_ARG(method, arg)                \
do {                                                                        \
    id<RenderPluginDelegate> delegate = GetAppController().renderDelegate;  \
    if([delegate respondsToSelector:@selector(method:)])                    \
        [delegate method:arg];                                              \
} while(0)


// these are simple wrappers about ios api, added for convenience
void AppController_SendNotification(NSString* name);
void AppController_SendNotificationWithArg(NSString* name, id arg);

void AppController_SendUnityViewControllerNotification(NSString* name);

// in the case when apple adds new api that has easy fallback path for old ios
// we will add new api methods at runtime on older ios, so we can switch to new api universally
// in that case we still need actual declaration: we do it here as most convenient place

#if (PLATFORM_IOS && !UNITY_HAS_IOSSDK_10_0) || (PLATFORM_TVOS && !UNITY_HAS_TVOSSDK_10_0)
@interface CADisplayLink ()
@property(nonatomic) NSInteger preferredFramesPerSecond;
@end
#endif

#if (PLATFORM_IOS && !UNITY_HAS_IOSSDK_10_3) || (PLATFORM_TVOS && !UNITY_HAS_TVOSSDK_10_2)
// The maximumFramesPerSecond API is available in the SDKs since 10.3.
// However, tvOS SDK has it already in 10.2, but disabled.
@interface UIScreen ()
@property (readonly) NSInteger maximumFramesPerSecond;
@end
#endif
