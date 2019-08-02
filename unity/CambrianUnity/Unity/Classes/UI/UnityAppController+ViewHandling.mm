#include "UnityAppController+ViewHandling.h"
#include "UnityAppController+Rendering.h"

#include "UI/OrientationSupport.h"
#include "UI/UnityView.h"
#include "UI/UnityViewControllerBase.h"
#include "Unity/DisplayManager.h"


// TEMP: ?
#include "UI/ActivityIndicator.h"

extern bool _skipPresent;
extern bool _unityAppReady;


@implementation UnityAppController (ViewHandling)

#if UNITY_SUPPORT_ROTATION
// special case for when we DO know the app orientation, but dont get it through normal mechanism (UIViewController orientation handling)
// how can this happen:
// 1. On startup: ios is not sending "change orientation" notifications on startup (but rather we "start" in correct one already)
// 2. When using presentation controller it can override orientation constraints, so on dismissing we need to tweak app orientation;
//      pretty much like startup situation UIViewController would have correct orientation, and app will be out-of-sync
- (void)updateAppOrientation:(UIInterfaceOrientation)orientation
{
    _curOrientation = orientation;
    [_unityView willRotateToOrientation: orientation fromOrientation: (UIInterfaceOrientation)UIInterfaceOrientationUnknown];
    [_unityView didRotate];
}

#endif

- (UnityView*)createUnityView
{
    return [[UnityView alloc] initFromMainScreen];
}

- (UIViewController*)createUnityViewControllerDefault
{
    UnityDefaultViewController* ret = [[UnityDefaultViewController alloc] init];
#if PLATFORM_TVOS
    // This enables game controller use in on-screen keyboard
    ret.controllerUserInteractionEnabled = YES;
#endif
    return ret;
}

#if UNITY_SUPPORT_ROTATION
- (UIViewController*)createUnityViewControllerForOrientation:(UIInterfaceOrientation)orient
{
    switch (orient)
    {
        case UIInterfaceOrientationPortrait:            return [[UnityPortraitOnlyViewController alloc] init];
        case UIInterfaceOrientationPortraitUpsideDown:  return [[UnityPortraitUpsideDownOnlyViewController alloc] init];
        case UIInterfaceOrientationLandscapeLeft:       return [[UnityLandscapeLeftOnlyViewController alloc] init];
        case UIInterfaceOrientationLandscapeRight:      return [[UnityLandscapeRightOnlyViewController alloc] init];
            
        default:                                        NSAssert(false, @"bad UIInterfaceOrientation provided");
    }
    return nil;
}

#endif

- (UIView*)createSnapshotView
{
    return nil;
}

- (void)createUI
{
    NSAssert(_unityView != nil, @"_unityView should be inited at this point");
    NSAssert(_window != nil, @"_window should be inited at this point");

    [UIView setAnimationsEnabled: NO];
    
    [self startUnity];
}

- (void)forceAutorotatingControllerToRefreshEnabledOrientationsIfNeeded
{
    
}

- (UIViewController*)createRootViewController
{
    return [GetAppController() rootViewController];
}

- (void)showGameUI
{
    HideActivityIndicator();

    // make sure that we start up with correctly created/inited rendering surface
    // NB: recreateRenderingSurface won't go into rendering because _unityAppReady is false
    [_unityView recreateRenderingSurface];

    // why we set level ready only now:
    // surface recreate will try to repaint if this var is set (poking unity to do it)
    // but this frame now is actually the first one we want to process/draw
    // so all the recreateSurface before now (triggered by reorientation) should simply change extents

    _unityAppReady = true;

    // why we skip present:
    // this will be the first frame to draw, so Start methods will be called
    // and we want to properly handle resolution request in Start (which might trigger surface recreate)
    // NB: we want to draw right after showing window, to avoid black frame creeping in

    _skipPresent = true;

    if (!UnityIsPaused())
        UnityRepaint();

    _skipPresent = false;
    [self repaint];

    [UIView setAnimationsEnabled: YES];
}

- (UIViewController*)topMostController
{
    UIViewController *topController = self.window.rootViewController;
    while (topController.presentedViewController)
        topController = topController.presentedViewController;
    return topController;
}

- (void)willStartWithViewController:(UIViewController*)controller
{
    _unityView.contentScaleFactor   = UnityScreenScaleFactor([UIScreen mainScreen]);
    _unityView.autoresizingMask     = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
}

- (void)willTransitionToViewController:(UIViewController*)toController fromViewController:(UIViewController*)fromController
{
}

#if UNITY_SUPPORT_ROTATION
- (void)interfaceWillChangeOrientationTo:(UIInterfaceOrientation)toInterfaceOrientation
{
    UIInterfaceOrientation fromInterfaceOrientation = _curOrientation;
    
    _curOrientation = toInterfaceOrientation;
    [_unityView willRotateToOrientation: toInterfaceOrientation fromOrientation: fromInterfaceOrientation];
}

- (void)interfaceDidChangeOrientationFrom:(UIInterfaceOrientation)fromInterfaceOrientation
{
    [_unityView didRotate];
}

#endif
@end

extern "C" void UnityNotifyHideHomeButtonChange()
{
    //[GetAppController() notifyHideHomeButtonChange];
}

extern "C" void UnityNotifyDeferSystemGesturesChange()
{
    //[GetAppController() notifyDeferSystemGesturesChange];
}

