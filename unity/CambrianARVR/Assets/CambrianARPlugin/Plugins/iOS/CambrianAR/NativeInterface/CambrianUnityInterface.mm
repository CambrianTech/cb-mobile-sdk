//
//  CambrianUnity.m
//  CambrianUnity
//
//  Created by Joel Teply on 5/18/18.
//

#import "CambrianUnityInterface.h"
#import "UnityAppController+Rendering.h"
#import "UnityAppController+ViewHandling.h"
#include "UI/UnityView.h"
#include "Unity/DisplayManager.h"

#include "os/TimeZoneInfo.h"
//#include "RegisterMonoModules.h"
//#include "RegisterFeatures.h"

#include <csignal>

void UnityInitTrampoline();

static CambrianUnityInterface *m_instance  = nil;
static bool m_hasInitialized = false;

@implementation CambrianUnityInterface

- (instancetype)init {
    if (self = [super init])
    {
        self.session = [[CambrianARSession alloc] init];
    }
    return self;
}

+ (CambrianUnityInterface *)sharedInstance {
    if (!m_instance) {
        m_instance = [CambrianUnityInterface new];
    }
    return m_instance;
}

- (void)applicationWillResignActive:(UIApplication*)application {
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [CambrianUnityInterface.sharedInstance destroyDisplayLink];
    });
}

- (void)startUnityInFrame:(CGRect)frame withScene:(NSString *)scene {
    
    [self.session setSceneName:scene];
    
    if (m_hasInitialized) {
        //resume session
        [self restartUnity];
    } else {
        m_hasInitialized = true;
        
        // send notfications
        UnityInitTrampoline();
        
        NSString *app_path = [[NSBundle mainBundle] resourcePath];
        char *argv[1] = {(char *) [app_path UTF8String]};
        UnityInitRuntime(1, argv);
        
        ///seems unnecessary:
        //RegisterMonoModules();
        //NSLog(@"-> registered mono modules %p\n", &constsection);
        //RegisterFeatures();
        
        std::signal(SIGPIPE, SIG_IGN);
        
        UnityInitApplicationNoGraphics([[[NSBundle mainBundle] bundlePath] UTF8String]);
        
        [self selectRenderingAPI];
        [UnityRenderingView InitializeForAPI: self.renderingAPI];
        
        _window         = [[[UIApplication sharedApplication] delegate] window];
        _unityView      = [[UnityView alloc] initWithFrame:CGRectMake(0, 0, frame.size.width, frame.size.height)];
        
        [DisplayManager Initialize];
        _mainDisplay    = [DisplayManager Instance].mainDisplay;
        [_mainDisplay createWithWindow: _window andView: _unityView];
        
        [self createUI];
    }
    
#if !PLATFORM_TVOS && DISABLE_TOUCH_DELAYS
    for (UIGestureRecognizer *g in _window.gestureRecognizers)
    {
        g.delaysTouchesBegan = false;
    }
#endif
    
}

- (void)stopUnity {
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [self destroyDisplayLink];
        [super applicationWillResignActive:[UIApplication sharedApplication]];
    });
}

- (void)restartUnity
{
    _unityView.hidden = true;
    [_unityView.layer setNeedsDisplay];
    
    [self createDisplayLink];
    [super applicationDidBecomeActive:[UIApplication sharedApplication]];
    
    //come back anyway
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        CambrianUnityInterface.sharedInstance.view.hidden = false;
    });
}

extern "C" void cambrian_firstFrameReceived() {
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        CambrianUnityInterface.sharedInstance.view.hidden = false;
    });
}

#define UIViewParentController(__view) ({ \
UIResponder *__responder = __view; \
while ([__responder isKindOfClass:[UIView class]]) \
__responder = [__responder nextResponder]; \
(UIViewController *)__responder; \
})

- (UIViewController *)rootViewController {
    return UIViewParentController(self.unityView);
}

- (UIView *)rootView {
    return self.unityView;
}

- (void)refreshUnityView
{
    [self showGameUI];
}

- (UIView *)view {
    return _unityView;
}

@end

namespace il2cpp
{
    namespace os
    {
        bool TimeZoneInfo::UsePalForTimeZoneInfo() {
            return false;
        }

        void* TimeZoneInfo::GetTimeZoneIDs() {
            return nullptr;
        }

        bool TimeZoneInfo::GetLocalTimeZoneData(void** nativeRawData, char** nativeID, int* size) {
            return false;
        }

        bool TimeZoneInfo::GetTimeZoneDataForID(char* id, void** nativeRawData, int* size) {
            return false;
        }
    }
}

