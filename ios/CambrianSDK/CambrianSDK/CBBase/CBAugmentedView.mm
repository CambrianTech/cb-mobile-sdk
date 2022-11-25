//
//  CBAugmentedView.m
//  CambrianAR
//
//  Created by Joel Teply on 12/4/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import "CBAugmentedView.h"

#import "CBARInternal.h"
#import <Foundation/Foundation.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <CoreML/CoreML.h>
#import <CoreLocation/CoreLocation.h>

#import "ImageTranslation.h"
#import "CoreMLTools.h"

#include <utility/Diagnostics.h>

#include <cbar/CBAR_View.hpp>
#include <cbar/scene/scene.h>
#include <cbcommon/CB_Types.h>
#include <cbar/CBAR_GLOffscreenRenderer.hpp>
#include <vector>

#define TEST_IMAGE_URL_0 @"https://images.unsplash.com/photo-1531835551805-16d864c8d311?ixlib=rb-1.2.1&ixid=eyJhcHBfaWQiOjEyMDd9&w=1000&q=80"
#define TEST_IMAGE_URL_1 @"https://designingidea.com/wp-content/uploads/2016/07/kitchen-with-american-cherry-hardwood-flooring.jpg"
#define TEST_IMAGE_URL_2 @"https://static1.squarespace.com/static/5484733fe4b0c710260b03b2/t/5951602de110eb8a73f53b09/1498505377961/Massage+office+wall.jpg"
#define TEST_IMAGE_URL_3 @"https://allbusinesssystems.com/wp-content/uploads/2017/04/Signature-Cubicle-Furniture-OPS-2.jpg"
#define TEST_IMAGE_URL_4 @"https://i.pinimg.com/originals/60/22/f8/6022f8952fa666870e280711ac63e74b.jpg"
#define TEST_IMAGE_URL_5 @"https://zonaprinta.com/wp-content/uploads/2018/09/Commercial-Pp-Carpet-Tiles-MalaysiaModern-Carpet-Tiles-Malaysia.jpg"
#define TEST_IMAGE_URL_6 @"https://img.alicdn.com/i1/office-carpet-carpet-squares-engineering-living-room-carpet-thick-carpet-office-carpet-hotel-carpet-pvc-carpet/TB16hv9IVXXXXcqXVXXXXXXXXXX_!!0-item_pic.jpg"
#define TEST_IMAGE_URL_7 @"https://forbo.blob.core.windows.net/forboimages/6484/1485945430339.jpg"
#define TEST_IMAGE_URL_8 @"https://kickstumbler.com/wp-content/uploads/2016/12/Awesome-Office-Carpet-Tiles.jpg"
#define TEST_IMAGE_URL_9 @"https://www.burmatex.co.uk/wp-content/uploads/2016/06/185-grade-tufted-loop-pile-planks-abyss-zinc-smoke-silver-herringbone-grey-studio.jpg"

#define RUN_TESTS 0
#define COMBINE_DEBUG_IMAGES 1

using namespace cbar;

HitTestResult HitTestResultForCBHitTestResult(const CBHitTestResult &hitTest) {
    
    HitTestResult result;
    
    result.type = HitTestResultType(hitTest.type);
    result.distance = hitTest.distance;
    result.localTransform = cbMatToEigen(hitTest.localTransform);
    result.worldTransform = cbMatToEigen(hitTest.worldTransform);
    
//    if (hitTest.anchorIdentifier) {
//        result.anchorIdentifier = std::string((const char *)hitTest.anchorIdentifier);
//    }
//    result.isValid = hitTest.isValid;
    
    return result;
}

PlaneAnchor PlaneAnchorForCBPlaneAnchor(const CBPlaneAnchor &anchor) {
    PlaneAnchor result;
    
    if (anchor.anchorIdentifier) {
        result.anchorIdentifier = std::string((const char *)anchor.anchorIdentifier);
    }
    result.transform = cbMatToEigen(anchor.transform);
    result.alignment = PlaneAnchorAlignment(anchor.alignment);
    result.center = {anchor.center.x, anchor.center.y, anchor.center.z};
    result.extent = {anchor.extent.x, anchor.extent.y, anchor.extent.z};
    
    result.contour.resize(anchor.geometry.boundaryVertexCount);
    for (int i=0; i<anchor.geometry.boundaryVertexCount; i++) {
        int j=i*4;
        //convert to coordinate space
        result.contour[i] = Eigen::Vector3f(anchor.geometry.boundaryVertices[j],
                                            anchor.geometry.boundaryVertices[j+1],
                                            -anchor.geometry.boundaryVertices[j+2]);
    }
    
    return result;
}

UserAnchor UserAnchorForCBUserAnchor(const CBUserAnchor &anchor) {
    UserAnchor result;
    
    if (anchor.anchorIdentifier) {
        result.anchorIdentifier = std::string((const char *)anchor.anchorIdentifier);
    }
    result.transform = cbMatToEigen(anchor.transform);
    
    return result;
}

API_AVAILABLE(ios(12.0))
@interface CBAugmentedView() <CambrianARSessionListener, CLLocationManagerDelegate> {
    BOOL isRunning;
    
    BOOL m_licenseMessagedUser;
    int64_t m_frameCounter;
    
    int m_captureCallbackCount;
    
    CADisplayLink* m_displayLink;
    
    CBAR_GLOffscreenRenderer m_offscreenRenderer;
    GLuint m_offscreenTexture;
    
    CGColorSpaceRef m_colorSpaceRef;
    
    CBSessionState m_state;
    
    NSString *_unityScene;
    
    int frameIndex;

    std::vector<cv::Mat>_testImages;
    
    NSMutableArray *_scheduledCommands;
}

@property (nonatomic, assign) BOOL isHalted;
@property (nonatomic, assign) BOOL isInBackground;
@property (assign, nonatomic) BOOL hasPerformedLayout;
@property (nonatomic, assign) BOOL shuttingDown;
@property (nonatomic, assign) BOOL isPaused;
@property (nonatomic, assign) BOOL notifiedReady;
@property (nonatomic, assign) int frameRotation;
@property (nonatomic, assign) CBMatrix4x4 displayTransform;

@property (nonatomic, assign) dispatch_once_t prepareToken;
@property (assign, nonatomic) BOOL preparedViewport;
@property (nonatomic, assign) dispatch_once_t  displayToken;
@property (assign, nonatomic) std::shared_ptr<CBAR_CallbackI> callback;

@property (strong, nonatomic) MLModel *segmenterModel;
@property (strong, nonatomic) MLModel *normalsModel;
@property (strong, nonatomic) MLModel *elevationModel;
@property (strong, nonatomic) MLModel *shadowsModel;

@property (strong, nonatomic) CLLocationManager* locationManager;

@end

@interface UIView (mxcl)
- (UIViewController *)parentViewController;
@end

@implementation UIView (mxcl)
- (UIViewController *)parentViewController {
    UIResponder *responder = self;
    while ([responder isKindOfClass:[UIView class]])
        responder = [responder nextResponder];
    return (UIViewController *)responder;
}
@end

__weak static CBAugmentedView *m_instance;

struct CBARCallback : CBAR_CallbackI {
    
    CBARCallback(void *context) : CBAR_CallbackI(context) {
        
    }
    
    virtual void sceneImageLoaded(const cv::Mat &image);
    
    virtual void historyChanged(std::shared_ptr<cbpipe::UndoState> undo, bool forward);
    
    virtual void colorsCallback(std::vector<cbar::ColorInfo>&colors);
    
    virtual void exposureCallback(cv::Point2f exposurePoint);
    
    virtual void focusCallback(cv::Point2f focusPoint);
    
    virtual void drawingUpdated(const cv::Mat &mask, const cv::Mat &color);
    
    virtual void saveScreenshot(const std::string &path, bool isJPEG, int qualityLevel);
    
    virtual void orientationNormalFound(const Eigen::Vector3f &normal);
    
    virtual std::vector<cbpipe::HitTestResult> hitTestAtPoint(const cv::Point2f &point, HitTestResultType resultType);
    
    virtual void loadDeepModel(const CBP_DeepModelInfo &info);
    
    virtual cv::Mat predict(const CBP_DeepModelInfo &info, const std::map<std::string, cv::Mat> &images);
    
    virtual void vibrateDevice();
    
    virtual void uploadDiagnosticImageAtPath(const std::string &path);
};

@implementation CBAugmentedView

@dynamic isLiveMode;
@dynamic isAugmentedReality;

- (void)dealloc
{
    NSLog(@"Deallocating %@", NSStringFromClass([self class]));
    
    [self removeNotifications];
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        CBThread::listRunningCBThreads();
    });

}

+ (NSURL *) bundlePathForResource:(NSString *)path {
    NSBundle * bundle = [NSBundle bundleForClass:[self class]];
    NSURL *modelUrl = [bundle URLForResource:path withExtension:nil];
    return modelUrl;
}

+ (NSURL *) modelPathForName:(NSString *)name {
    NSString *fileName = [name stringByDeletingPathExtension];
    return [self bundlePathForResource:[fileName stringByAppendingString:@".mlmodelc"]];
}

- (void) initialize {
    CambrianUnityInterface.sharedInstance.session.delegate = self;
    
    _scheduledCommands = [NSMutableArray new];
    
    m_instance = self;
    m_state.isVideo = YES;
    
    if (!self.scene) [self generateScene];
    
    self.locationManager = [[CLLocationManager alloc] init];
    self.locationManager.delegate = self;
    
    m_colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    
    dispatch_queue_attr_t highPriorityAttr = dispatch_queue_attr_make_with_qos_class (DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INITIATED,-1);
    self.cb_queue = dispatch_queue_create ("com.cambrian.cb_queue", highPriorityAttr);
    
    dispatch_queue_attr_t mediumPriorityAttr = dispatch_queue_attr_make_with_qos_class (DISPATCH_QUEUE_CONCURRENT, QOS_CLASS_USER_INITIATED,-1);
    self.unity_queue = dispatch_queue_create ("com.cambrian.unity_queue", mediumPriorityAttr);
    
    dispatch_queue_attr_t uploadAttr = dispatch_queue_attr_make_with_qos_class (DISPATCH_QUEUE_CONCURRENT_WITH_AUTORELEASE_POOL, QOS_CLASS_BACKGROUND,-1);
    self.upload_queue = dispatch_queue_create ("com.cambrian.upload_queue", uploadAttr);
    
    UITapGestureRecognizer *tapped = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(tapped:)];
    tapped.cancelsTouchesInView = NO;
    [self addGestureRecognizer:tapped];
}

- (id) initWithCoder:(NSCoder *)aDecoder {
    if (self = [super initWithCoder:aDecoder]) {
        [self initialize];
    }
    
    return self;
}

- (id) initWithFrame:(CGRect)frame {
    if (self = [super initWithFrame:frame]) {
        [self initialize];
    }
    
    return self;
}

#if RUN_TESTS

- (void)loadTestImages {
    @synchronized(TEST_IMAGE_URL_0){
        if (!_testImages.size()) {
            [self loadTestImage:TEST_IMAGE_URL_0];
            [self loadTestImage:TEST_IMAGE_URL_1];
            [self loadTestImage:TEST_IMAGE_URL_2];
            [self loadTestImage:TEST_IMAGE_URL_3];
            [self loadTestImage:TEST_IMAGE_URL_4];
            [self loadTestImage:TEST_IMAGE_URL_5];
            [self loadTestImage:TEST_IMAGE_URL_6];
            [self loadTestImage:TEST_IMAGE_URL_7];
            [self loadTestImage:TEST_IMAGE_URL_8];
            [self loadTestImage:TEST_IMAGE_URL_9];
        }
    }
}

-(void)loadTestImage:(NSString *)path {
    NSData *data = [NSData dataWithContentsOfURL:[NSURL URLWithString:path]];
    if (!data) return;
    UIImage *uiImage = [UIImage imageWithData:data];
    if (!uiImage) return;
    cv::Mat image = [ImageTranslation CVMatForImage:uiImage];
    _testImages.push_back(image);
}

- (cv::Mat)getInputImage:(const cv::Mat&)src forInfo:(const CBP_DeepModelInfo &)info  {
    
    cv::Mat image;
    cv::resize(src, image, info.input.resolution);
    if (image.channels() == 4) {
        cv::cvtColor(image, image, CV_RGBA2RGB);
    }
    
    return image;
}

- (void)testSingleImage:(const CBP_DeepModelInfo &)info {
    
    for (int i=0; i<_testImages.size(); i++) {

        auto start = sys_usec_time();
        
        MLModel *model = [self modelForType:info.type];
        cv::Mat image = [CoreMLTools sizeImageForModel:model image:_testImages[i]];
        
        cv::Mat output = [CoreMLTools predictWithModel:model image:image];
        
        float elapsedTime = seconds_elapsed(start);
        
#if COMBINE_DEBUG_IMAGES
        if (model == self.segmenterModel) {
            std::vector<cv::Mat>planes;
            cv::split(output, planes);
            for (int j=0; j<planes.size(); j++) {
                cv::Mat img = image.clone();
                ImageProcessing::overlayMaskOntoRGB(planes[j], img);
                Diagnostics::SaveDiagnosticImage(info.output.flipBGR, img, "%s_%d_(%d)result.png", info.name.c_str(), i, j);
            }
        } else {
            if (output.channels() != image.channels()) {
                cv::cvtColor(output, output, CV_GRAY2RGB);
            }
            cv::hconcat(image, output, image);
            Diagnostics::SaveDiagnosticImage(info.output.flipBGR, image, "%s_%d_result.png", info.name.c_str(), i);
        }
        
        
#else
        Diagnostics::SaveDiagnosticImage(info.output.flipBGR, image, "%s_%d_src.png", info.name.c_str(), i);
        
        if (model == self.segmenterModel) {
            std::vector<cv::Mat>planes;
            cv::split(output, planes);
            for (int j=0; j<planes.size(); j++) {
                Diagnostics::SaveDiagnosticImage(false, planes[j], "%s_%d_(%d)result.png", info.name.c_str(), i, j);
            }
        } else {
            Diagnostics::SaveDiagnosticImage(info.output.flipBGR, output, "%s_%d_result.png", info.name.c_str(), i);
        }
#endif
        
        NSLog(@"%s test completed in %f seconds", info.name.c_str(), elapsedTime);
    }
}

#endif

+ (CBAugmentedView *) getInstance {
    return m_instance;
}

- (void) runCommand:(dispatch_block_t)command {
    if (isRunning && self.notifiedReady) {
        dispatch_async(self.cb_queue, command);
    } else {
        [_scheduledCommands addObject:command];
    }
}

- (void)testSegmentation:(const CBP_DeepModelInfo &)info {
    
    for (int i=0; i<_testImages.size(); i++) {
        
        MLModel *model = [self modelForType:info.type];
        cv::Mat image = [CoreMLTools sizeImageForModel:model image:_testImages[i]];
        
        std::map<std::string, cv::Mat> inputImages;
        inputImages["Placeholder__0"] = image;
        inputImages["Placeholder_1__0"] = cv::Mat::zeros(256, 256, CV_8UC1);
        
        //run once, rotate result, input again
        cv::Mat lastMask = [CoreMLTools predictWithModel:model images:inputImages];
        cv::Mat trans = cv::getRotationMatrix2D( cv::Point(lastMask.cols/2, lastMask.rows/2), -5.0, 1.0);
        cv::warpAffine(lastMask, lastMask, trans, lastMask.size());
        inputImages["Placeholder_1__0"] = lastMask;
        
        //run again with last mask output as input
        auto start = sys_usec_time();
        cv::Mat output = [CoreMLTools predictWithModel:model images:inputImages];
        float elapsedTime = seconds_elapsed(start);
        
#if COMBINE_DEBUG_IMAGES
        std::vector<cv::Mat>planes;
        cv::split(output, planes);
        for (int j=0; j<planes.size(); j++) {
            cv::Mat img = image.clone();
            ImageProcessing::overlayMaskOntoRGB(planes[j], img);
            Diagnostics::SaveDiagnosticImage(info.output.flipBGR, img, "%s_%d_(%d)result.png", info.name.c_str(), i, j);
        }
#else
        Diagnostics::SaveDiagnosticImage(info.output.flipBGR, image, "%s_%d_src.png", info.name.c_str(), i);
        
        std::vector<cv::Mat>planes;
        cv::split(output, planes);
        for (int j=0; j<planes.size(); j++) {
            Diagnostics::SaveDiagnosticImage(false, planes[j], "%s_%d_(%d)result.png", info.name.c_str(), i, j);
        }
#endif
        
        NSLog(@"%s test completed in %f seconds", info.name.c_str(), elapsedTime);
    }
}

- (void) setScene:(CBAugmentedScene *)scene {
    
    cbar::CBAR_VideoFramePtr frame = scene.coreScene->getVideoFrame();

    if (!frame.empty()) {
        [self stopRunning];
        self.prepareToken = 0;
        self.displayToken = 0;
        
        [self prepareViewportForImageSize:CGSizeMake(frame->frameSize().width, frame->frameSize().height)
                                 rotation:0
                          displayRotation:180];
        _scene = scene;
        [self startRunning:NO unityScene:_unityScene];
    } else {
        _scene = scene;
    }
}

- (void)startRunning {
    [self startRunning:YES unityScene:_unityScene];
}

- (void)startRunning:(BOOL)isVideo unityScene:(NSString * _Nullable)unityScene {
    _unityScene = unityScene;
    
    [self addNotifications];
    
    if (!CambrianUnityInterface.sharedInstance.view.superview) {
        [CambrianUnityInterface.sharedInstance startUnityInFrame:self.frame withScene:unityScene];
        [self addSubview:CambrianUnityInterface.sharedInstance.view];
    }
    
    if (self.isAugmentedReality) {
        __weak typeof(self) weakSelf = self;
        [self dispatch_cb_get_result:^{
            __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
            strongSelf.coreView->startRunning(isVideo);
        }];
    }
    
    m_state.isVideo = isVideo;
    m_state.worldTransform = eigenToCBMat(self.scene.coreScene->getWorldTransform());
    
    [CambrianUnityInterface.sharedInstance.session startRunning:m_state];
    
    if (isRunning) {
        [CambrianUnityInterface.sharedInstance refreshUnityView];
    }
    
    [self.locationManager startUpdatingHeading];
    
    isRunning = YES;
}

- (void)runScheduledCommands {
    @synchronized(_scheduledCommands) {
        for (id command in _scheduledCommands) {
            dispatch_sync(self.cb_queue, command);
        }
        [_scheduledCommands removeAllObjects];
    }
}

- (void)stopRunning {
    self.shuttingDown = true;
    self.notifiedReady = false;
    [CambrianUnityInterface.sharedInstance.session stopRunning:m_state];
    [CambrianUnityInterface.sharedInstance stopUnity];
    [CambrianUnityInterface.sharedInstance.view removeFromSuperview];
    self.shuttingDown = false;
}

- (void)captureToStill {
    __weak typeof(self) weakSelf = self;
    [self dispatch_cb_get_result:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        strongSelf.coreView->captureCurrentState();
    }];
    
    [CambrianUnityInterface.sharedInstance.session captureToStill];
    m_state.isVideo = false;
}

- (BOOL)isLiveMode {
    return m_state.isVideo;
}

- (BOOL)isAugmentedReality {
    if (_unityScene == nil || _unityScene.length == 0) {
        return true;
    }
    return false;
}

- (void)sendUnityCommand:(NSString *)commandName json:(NSString *)jsonString {
    
    [self runCommand:^{
        CBCommandData command;
        command.command = (void *)[commandName UTF8String];
        command.json = (void *)[jsonString UTF8String];
        [CambrianUnityInterface.sharedInstance.session sendUnityCommand:command];
    }];
}

#pragma mark Notifications

- (void) addNotifications {
    [self removeNotifications];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillResignActive)
                                                 name:UIApplicationWillResignActiveNotification
                                               object:nil ];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationDidEnterBackground)
                                                 name:UIApplicationDidEnterBackgroundNotification
                                               object:nil ];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillEnterForeground)
                                                 name:UIApplicationWillEnterForegroundNotification
                                               object:nil ];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationDidBecomeActive)
                                                 name:UIApplicationDidBecomeActiveNotification
                                               object:nil ];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationDidReceiveMemoryWarning)
                                                 name:UIApplicationDidReceiveMemoryWarningNotification
                                               object:nil ];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillTerminate)
                                                 name:UIApplicationWillTerminateNotification
                                               object:nil ];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(orientationChanged)
                                                 name:UIDeviceOrientationDidChangeNotification
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(transitioningViewControllers:)
                                                 name:@"UINavigationControllerDidShowViewControllerNotification"
                                               object:nil];
}

- (void) removeNotifications {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void) applicationDidEnterBackground {
    self.isInBackground = YES;
    [CambrianUnityInterface.sharedInstance applicationDidEnterBackground:[UIApplication sharedApplication]];
}

- (void) applicationWillResignActive {
    self.isInBackground = YES;
    [CambrianUnityInterface.sharedInstance applicationWillResignActive:[UIApplication sharedApplication]];
    
//    UINavigationController *nav = (UINavigationController *) [UIApplication sharedApplication].keyWindow.rootViewController;
//    if (nav && [nav respondsToSelector:@selector(popViewControllerAnimated:)]) {
//        [self stopRunning];
//        self.isHalted = YES;
//        [nav popViewControllerAnimated:NO];
//    } else {
//        exit(0);
//    }
    
    [self stopRunning];
    self.isHalted = YES;
    
    exit(0);
}

- (void) applicationWillEnterForeground {
    //occasionally fails
}

- (void) applicationDidBecomeActive {
    if (self.isInBackground) {
        [self startRunning];
        self.isInBackground = NO;
    }
    [CambrianUnityInterface.sharedInstance applicationDidBecomeActive:[UIApplication sharedApplication]];
}

- (void) applicationDidReceiveMemoryWarning {
    self.isInBackground = NO;
    [CambrianUnityInterface.sharedInstance applicationDidReceiveMemoryWarning:[UIApplication sharedApplication]];
}

- (void) applicationWillTerminate {
    self.isInBackground = NO;
    [CambrianUnityInterface.sharedInstance applicationWillTerminate:[UIApplication sharedApplication]];
}

- (void)transitioningViewControllers:(NSNotification *) notification {
    
    UIViewController *currentVC = [self topViewController];
    
    if (currentVC != self.parentViewController) {
        [self stopRunning];
        self.isHalted = YES;
    } else if (self.isHalted && currentVC == self.parentViewController) {
        self.isHalted = NO;
        [self startRunning:m_state.isVideo unityScene:_unityScene];
    }
}

- (UIViewController*)topViewController {
    return [self topViewControllerWithRootViewController:[UIApplication sharedApplication].keyWindow.rootViewController];
}

- (UIViewController*)topViewControllerWithRootViewController:(UIViewController*)rootViewController {
    if ([rootViewController isKindOfClass:[UITabBarController class]]) {
        UITabBarController* tabBarController = (UITabBarController*)rootViewController;
        return [self topViewControllerWithRootViewController:tabBarController.selectedViewController];
    } else if ([rootViewController isKindOfClass:[UINavigationController class]]) {
        UINavigationController* navigationController = (UINavigationController*)rootViewController;
        return [self topViewControllerWithRootViewController:navigationController.visibleViewController];
    } else if (rootViewController.presentedViewController) {
        UIViewController* presentedViewController = rootViewController.presentedViewController;
        return [self topViewControllerWithRootViewController:presentedViewController];
    } else {
        return rootViewController;
    }
}

#pragma mark Events

- (TouchStep) gestureStateToStep:(UIGestureRecognizerState)state {
    switch (state) {
        case UIGestureRecognizerStateBegan:
            return TouchStepBegan;
        case UIGestureRecognizerStateChanged:
            return TouchStepMoved;
        case UIGestureRecognizerStateEnded:
            return TouchStepEnded;
        default:
            break;
    }
    
    return TouchStepCancelled;
}

- (TouchStep) phaseToStep:(CBTouchPhase)phase {
    switch (phase) {
        case CBTouchPhaseBegan:
            return TouchStepBegan;
        case CBTouchPhaseStationary:
        case CBTouchPhaseMoved:
            return TouchStepMoved;
        case CBTouchPhaseEnded:
            return TouchStepEnded;
        case CBTouchPhaseCanceled:
            return TouchStepCancelled;
        default:
            break;
    }
    
    return TouchStepCancelled;
}

- (void) touch:(NSSet *)touches step:(TouchStep)step  {
    UITouch *touch = [touches anyObject];
    if (touches.count > 1) return;
    
    if (int(step) < 0) {
        return;
    }
    
    CGPoint touchPoint = [touch locationInView:CambrianUnityInterface.sharedInstance.view];
    
    cv::Point2f normalizedPoint = cv::Point2f((touchPoint.x) / self.frame.size.width,
                                              (touchPoint.y) / self.frame.size.height);
    
    __weak typeof(self) weakSelf = self;
    
    [self dispatch_cb_get_result:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        strongSelf.coreView->touchedAt(normalizedPoint, step);
    }];
}

- (void)tapped:(UITapGestureRecognizer *)t {
    
    CGPoint touchPoint = [t locationInView:self];
    
    cv::Point2f normalizedPoint = cv::Point2f((touchPoint.x) / self.frame.size.width,
                                              (touchPoint.y) / self.frame.size.height);
    
    if ([self.rootDelegate respondsToSelector:@selector(assetTapped:)]) {
        if (CBAugmentedAsset *asset = [self assetAtPoint:touchPoint]) {
            [self.rootDelegate assetTapped:asset];
            return;
        }
    }
    
    __weak typeof(self) weakSelf = self;
    [self dispatch_cb_get_result:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        strongSelf.coreView->tappedAt(normalizedPoint);
    }];
}

- (void)touchEvent:(CBTouchPhase)phase position:(CBPoint)touchPoint {
    cv::Point2f normalizedPoint = cv::Point2f(touchPoint.x, touchPoint.y);
    
    TouchStep step = [self phaseToStep:phase];
    
    //CBLog("STEP: %s", [self touchStepToString:step]);
    
    __weak typeof(self) weakSelf = self;
    [self dispatch_cb_get_result:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        strongSelf.coreView->touchedAt(cv::Point2f(normalizedPoint.x, normalizedPoint.y), step);
    }];
}

- (CBToolMode) toolMode {
    __block CBToolMode toolMode;
    
    __weak typeof(self) weakSelf = self;
    [self dispatch_cb_get_result:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        toolMode = (CBToolMode)strongSelf.coreView->getToolMode();
    }];
    
    return toolMode;
}

- (void) setToolMode:(CBToolMode)toolMode {
    __weak typeof(self) weakSelf = self;
    
    [self dispatch_cb:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        strongSelf.coreView->setToolMode((ToolMode) toolMode);
    }];
}

- (CBAugmentedAsset *)assetAtPoint:(CGPoint)point {
    return nil;
}

- (void) orientationChanged {
    
}

- (void) generateScene {
    self.scene = [CBAugmentedScene new];
}

- (std::shared_ptr<CBAR_View>) coreView {
    if (!_coreView) {
        if (!m_instance) {
            m_instance = self;
        }
        if (!_callback) {
            self.callback = std::shared_ptr<CBARCallback>(new CBARCallback((__bridge void *) self));
        }
        _coreView = [self generateCoreView:self.callback];
    }
    return _coreView;
}

- (std::shared_ptr<CBAR_View>) generateCoreView:(std::shared_ptr<CBAR_CallbackI>)callback; {
    auto coreView = std::shared_ptr<CBAR_View>(new CBAR_View(callback));
    coreView->setToolMode(ToolModeFindColor);
    return coreView;
}

- (void) dispatch_cb:(dispatch_block_t) block {
    if (!m_instance || !m_instance.cb_queue) return;
    
    __weak typeof(self) weakSelf = self;
    dispatch_async(self.cb_queue, ^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        if (block) block();
    });
}

- (void) dispatch_cb_get_result:(dispatch_block_t) block {
    
    __weak typeof(self) weakSelf = self;
    dispatch_sync(self.cb_queue, ^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        if (block) block();
    });
}

inline void getMatrixFromCGAffineTransform(CBMatrix4x4& outMatrix, CGAffineTransform displayTransform)
{
    outMatrix.column0.x = displayTransform.a;
    outMatrix.column0.y = displayTransform.c;
    outMatrix.column0.z = displayTransform.tx;
    
    outMatrix.column1.x = displayTransform.b;
    outMatrix.column1.y = displayTransform.d;
    outMatrix.column1.z = displayTransform.ty;
    
    outMatrix.column2.z = 1.0f;
    outMatrix.column3.w = 1.0f;
}

- (void)setCGTransform:(CGAffineTransform)imageTransform {
    //UIInterfaceOrientation orientation = [[UIApplication sharedApplication] statusBarOrientation];
    
    CBMatrix4x4 trans;
    memset(&trans, 0, sizeof(CBMatrix4x4));
    
    getMatrixFromCGAffineTransform(trans, imageTransform);
    
    self.displayTransform = trans;
}

- (void)setDisplayTransform:(CBMatrix4x4)displayTransform {
    _displayTransform = displayTransform;
    Eigen::Matrix4f trans = cbMatToEigen(_displayTransform);
    self.scene.coreScene->setDisplayTransform(trans);
}

- (void)prepareViewportForImageSize:(CGSize)imageSize
                           rotation:(int)rotation
                    displayRotation:(int)displayRotation {
    
    __weak typeof(self) weakSelf = self;
    
    float displayDensity = [[UIScreen mainScreen] scale];
    
    cv::Point2f pointOffset;
    
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone) {
        //based on rotation?
        pointOffset.y = -20.0f / [UIScreen mainScreen].bounds.size.height;
    }
    
    dispatch_once(&_prepareToken, ^{
        dispatch_async(dispatch_get_main_queue(), ^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf || !strongSelf.coreView) return;
            
            strongSelf.frameRotation = rotation;

            [strongSelf setContentScaleFactor: displayDensity];
            CGSize outputSize = CGSizeMake(displayDensity * strongSelf.frame.size.width, displayDensity * strongSelf.frame.size.height);

            CGAffineTransform affine = [ImageTranslation imageTransformForRotation:displayRotation
                                              imageSize:imageSize viewportSize:outputSize
                                              cropImage:strongSelf.isLiveMode];
            
            if (!strongSelf.isLiveMode) {
                [strongSelf setCGTransform:affine];
            }
            
            if (strongSelf.coreView) {
                [strongSelf dispatch_cb_get_result:^{
                    __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
                    strongSelf.coreView->setScene(strongSelf.scene.coreScene);
                    strongSelf.coreView->set2DOffset(pointOffset);
                }];
            }

            float fov = 1.11568153;
            Eigen::Matrix3f intrinsics = Eigen::Matrix3f::Identity();

            bool isRotatedDiagonally = rotation/90 % 2;
            if (!isRotatedDiagonally) {
                fov = fov * float(imageSize.height) / float(imageSize.width);
            }
            
            strongSelf.coreView->prepareViewport((__bridge void *)CambrianUnityInterface.sharedInstance.view.layer,
                                                 nil,
                                                 int(imageSize.width), int(imageSize.height),
                                                 outputSize.width,
                                                 outputSize.height,
                                                 fov, rotation, intrinsics);
            
            strongSelf.preparedViewport = YES;
        });
    });
}

- (void) layoutSubviews {
    [super layoutSubviews];
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self
                                             selector:@selector(didFinishAutoLayout)
                                               object:nil];
    [self performSelector:@selector(didFinishAutoLayout) withObject:nil
               afterDelay:0.1];
}

- (void) didFinishAutoLayout {
    [self boundsRefreshed];
    self.hasPerformedLayout = YES;    
}

- (void)setFrame:(CGRect)frame {
    [super setFrame:frame];
    
    [NSObject cancelPreviousPerformRequestsWithTarget:self
                                             selector:@selector(didFinishAutoLayout)
                                               object:nil];
    [self performSelector:@selector(didFinishAutoLayout) withObject:nil
               afterDelay:0.1];
}

- (void) boundsRefreshed {
    
    if (!self.hasPerformedLayout) return;
    
    _prepareToken = 0;
    _displayToken = 0;
    
    self.clipsToBounds = YES;
    CambrianUnityInterface.sharedInstance.view.bounds = CGRectMake(0, 0, self.bounds.size.width, self.bounds.size.height);
    CambrianUnityInterface.sharedInstance.view.frame = CambrianUnityInterface.sharedInstance.view.bounds;
}

- (float) segmentationAverageSeconds {
    return 0.2; //remove this method and calls to it
}

- (BOOL) needsFrame {
    return (frameIndex ++) % 5 == 0;
}

- (void) sendFrame:(CBCameraFrame)arFrame {
    if (!self.isAugmentedReality || self.shuttingDown || self.isPaused || self.isInBackground || !self.hasPerformedLayout) {
        return;
    }
    
    if (!self.preparedViewport) {
        
        int rotation = [ImageTranslation degreeRotationForDeviceOrientation:UIDeviceOrientation(arFrame.videoParams.screenOrientation)];
        [self prepareViewportForImageSize:CGSizeMake(arFrame.videoParams.yWidth, arFrame.videoParams.yHeight) rotation:rotation displayRotation:rotation];
        
        return;
    }
    
    if (!arFrame.videoParams.pYPixelBytes) return;
    
    cbar::RawFrame frame;
    
    frame.frameRotation = self.frameRotation;
    frame.dataLength = arFrame.videoParams.yWidth * arFrame.videoParams.yHeight * 3 / 2;
    
    frame.frameType = frame_type_420YpCbCr8;
    
    frame.data = (uint8_t *) arFrame.videoParams.pYPixelBytes;
    frame.chromaData = (uint8_t *) arFrame.videoParams.pUVPixelBytes;
    
    frame.width = arFrame.videoParams.yWidth;
    frame.height = arFrame.videoParams.yHeight;
    frame.stride = arFrame.videoParams.yStride;
    
    Eigen::Matrix4f cameraPos = cbMatToEigen(arFrame.cameraTransform);
    self.scene.coreScene->setCameraTransform(cameraPos);
    
    Eigen::Matrix4f projection = cbMatToEigen(arFrame.projectionMatrix);
    self.scene.coreScene->setCameraProjection(projection);
    
    __weak typeof(self) weakSelf = self;
    
    if (frame.data) {
        self.coreView->addFrame(frame);
    }
    
    dispatch_once(&_displayToken, ^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        strongSelf->_displayTransform = arFrame.displayTransform;
        Eigen::Matrix4f transform = cbMatToEigen(strongSelf->_displayTransform);
        strongSelf.scene.coreScene->setDisplayTransform(transform);
    });
}

#pragma mark CLLocation Callbacks

- (void)locationManager:(CLLocationManager*)manager didUpdateHeading:(CLHeading*)newHeading
{
    Eigen::Vector3f heading = {newHeading.x, newHeading.z, newHeading.y};
    heading.normalize();

    self.coreView->updateHeading(heading);
}

- (BOOL)locationManagerShouldDisplayHeadingCalibration:(CLLocationManager*)manager
{
    return NO;
}

- (void)locationManager:(CLLocationManager*)manager didFailWithError:(NSError*)error;
{
    NSLog(@"locationManager ERROR!");
    //gLocationServiceStatus.locationStatus = kLocationServiceFailed;
    //gLocationServiceStatus.headingStatus = kLocationServiceFailed;
}

#pragma mark Cambrian Callbacks

- (void) notifyReady:(BOOL)isReady {
    self.notifiedReady = isReady;
    if (isReady) {
        [self runScheduledCommands];
        CBLog("Notify Ready!");
    } else {
        CBLog("No longer Ready!");
    }
}

- (void) screenshotSaved {
    self.coreView->screenshotSaved();
}

- (void)cameraPositionUpdated:(const CBMatrix4x4 &)modelView {
    //Eigen::Matrix4f view = cbMatToEigen(modelView);
    //self.scene.coreScene->setWorldTransform(view);
}

- (void) cameraProjectionUpdated:(const CBMatrix4x4 &)projection {
    
}

- (void)assetTransformUpdated:(const CBMatrix4x4 &)position assetID:(NSString *)assetID {
    if (auto asset = self.scene.coreScene->getSelectedAsset()) {
        Eigen::Matrix4f transform = cbMatToEigen(position);
        asset->setWorldTransform(transform);
    }
}

- (void)arFrameCallback:(CBCameraFrame)arFrame {
    
    [self sendFrame:arFrame];
}

- (void)pointCloudDataUpdated:(CBPointCloudData)data {
    std::map<uint64_t, Eigen::Vector3f> eigenPoints;
    
    for (int i=0; i<data.points.size(); i++) {
        uint64_t identifier = data.identifiers[i];
        const auto &point = data.points[i];
        eigenPoints[identifier] = Eigen::Vector3f(point.x, point.y, point.z);
    }
    
    self.coreView->setPointCloudData(eigenPoints);
}

- (void)stillRender {
    self.coreView->stillRender();
}

- (void) historyChanged:(std::shared_ptr<cbpipe::UndoState>)undo forward:(BOOL)forward {
    
}

- (void) sceneImageLoaded:(const cv::Mat &)image {
    [CambrianUnityInterface.sharedInstance.session setMainCameraImage:image.data width:image.cols height:image.rows];
}

- (void) drawingUpdated:(const cv::Mat &)mask color:(const cv::Mat &)color {
    //cv::Mat mask = cv::Mat::zeros(_mask.rows, _mask.cols, CV_8UC4);
    //mask.setTo(cv::Scalar::all(255));
    [CambrianUnityInterface.sharedInstance.session drawingUpdated:mask.data width:mask.cols height:mask.rows color:(float *)color.data];
}

- (void) saveScreenshot:(const std::string &)path isJPEG:(BOOL)isJPEG qualityLevel:(int)quality {
    CBScreenshotData params;
    params.path = (void *)path.c_str();
    params.isJPEG = isJPEG;
    params.qualityLevel = quality;
    [CambrianUnityInterface.sharedInstance.session saveScreenshot:params];
}

- (SCNVector3) getGroundPosition3D:(CGPoint)touchPoint {
    return SCNVector3Zero;
}

- (void) orientationNormalFound:(const Eigen::Vector3f &)normal {
    
    CBVector3 cbNormal = {normal.x(), normal.y(), normal.z()};

    [CambrianUnityInterface.sharedInstance.session orientationNormalFound:cbNormal];
}

- (void) planeAnchorAdded:(CBPlaneAnchor)anchordata {
    auto anchor = PlaneAnchorForCBPlaneAnchor(anchordata);
    self.coreView->planeAnchorAdded(anchor);
}

- (void) planeAnchorUpdated:(CBPlaneAnchor)anchordata {
    auto anchor = PlaneAnchorForCBPlaneAnchor(anchordata);
    self.coreView->planeAnchorUpdated(anchor);
}

- (void) planeAnchorRemoved:(CBPlaneAnchor)anchordata {
    auto anchor = PlaneAnchorForCBPlaneAnchor(anchordata);
    self.coreView->planeAnchorRemoved(anchor);
}

- (void) userAnchorAdded:(CBUserAnchor)anchordata {
    auto anchor = UserAnchorForCBUserAnchor(anchordata);
    self.coreView->userAnchorAdded(anchor);
}

- (void) userAnchorUpdated:(CBUserAnchor)anchordata {
    auto anchor = UserAnchorForCBUserAnchor(anchordata);
    self.coreView->userAnchorUpdated(anchor);
}

- (void) userAnchorRemoved:(CBUserAnchor)anchordata {
    auto anchor = UserAnchorForCBUserAnchor(anchordata);
    self.coreView->userAnchorRemoved(anchor);
}

- (void)clearAll {
    if (!self.shuttingDown && _coreView) {
        self.coreView->clearAll();
    }
}

- (CBMatrix4x4) get2DAffineTransform:(const CBVector3*)before after:(const CBVector3*)after {
    
    cv::Point2f src[4];
    //std::cout << std::endl << "CV Points: ";
    for (int i=0; i<4; i++) {
        const CBVector3 &point = before[i];
        src[i] = cv::Point2f(point.x, point.y);
        //std::cout << src[i];
    }
    cv::Point2f dest[4];
    for (int i=0; i<4; i++) {
        const CBVector3 &point = after[i];
        dest[i] = cv::Point2f(point.x, point.y);
        //std::cout << dest[i];
    }
    
    //std::cout << std::endl;
    
    CBMatrix4x4 cbresult;
    
    cbresult.column0.x = 1.0f;
    cbresult.column1.y = 1.0f;
    cbresult.column2.z = 1.0f;
    cbresult.column3.w = 1.0f;
    
    
#if 0
    cv::Mat result = cv::getPerspectiveTransform(src, dest);
    
    cbresult.column0.x = result.at<double>(0,0);
    cbresult.column0.y = -result.at<double>(1,0);
    //cbresult.column0.z = result.at<double>(2,0);
    
    cbresult.column1.x = -result.at<double>(0,1);
    cbresult.column1.y = result.at<double>(1,1);
    //cbresult.column1.z = result.at<double>(2,1);
    
    //result.column2.x = perspective.at<double>(0,2);
    //result.column2.y = perspective.at<double>(1,2);
    //result.column2.z = perspective.at<double>(2,2);
    
    cbresult.column3.x = result.at<double>(0,2);
    cbresult.column3.y = result.at<double>(1,2);
    //cbresult.column3.z = result.at<double>(2,2);
#else
    cv::Mat result = cv::getAffineTransform(src, dest);
    
    //cbresult.column0.x = result.at<double>(0,0);
    //cbresult.column0.y = -result.at<double>(1,0);
    
    //cbresult.column1.x = -result.at<double>(0,1);
    //cbresult.column1.y = result.at<double>(1,1);
    
    cbresult.column3.x = result.at<double>(0,2);
    cbresult.column3.y = result.at<double>(1,2);
#endif

    return cbresult;
}

- (std::vector<HitTestResult>) hitTestAtPoint:(const cv::Point2f &)point resultType:(HitTestResultType)resultType {
    std::vector<CBHitTestResult> cbResults = [CambrianUnityInterface.sharedInstance.session hitTestAtPoint:{point.x, point.y}
                                                                                  resultType:CBHitTestResultType(resultType)];
    
    std::vector<HitTestResult>results;
    
    for (const auto &cbResult : cbResults) {
        results.push_back(HitTestResultForCBHitTestResult(cbResult));
    }
    
    return results;
}

- (MLModel *) loadDeepModelNamed:(NSString *)modelName {
    NSError *error;
    NSURL *modelUrl = [[self class] modelPathForName:modelName];
    MLModelConfiguration *config = [[MLModelConfiguration alloc] init];
    config.computeUnits = MLComputeUnitsCPUAndGPU;
    MLModel *model = [MLModel modelWithContentsOfURL:modelUrl configuration:config error:&error];
    NSAssert(model, @"Failed to load model %s", [modelName UTF8String]);
    return model;
}

- (void) loadDeepModel:(const CBP_DeepModelInfo &)info {
    
#if RUN_TESTS
    bool runTestImages = true;
    [self loadTestImages];
#endif
    
    switch (info.type) {
        case CBAR_PredictionType_Semantic: {
            self.segmenterModel = [self loadDeepModelNamed:@"CBSegmentationModel"];
#if RUN_TESTS
            runTestImages = false;
            [self testSegmentation:info];
#endif
            break;
        }
            
        case CBAR_PredictionType_Normals: {
            self.normalsModel = [self loadDeepModelNamed:@"CBNormalsModel"];
            break;
        }
            
        case CBAR_PredictionType_Elevation: {
            self.elevationModel = [self loadDeepModelNamed:@"CBElevationModel"];
            break;
        }

        case CBAR_PredictionType_Shadows: {
            self.shadowsModel = [self loadDeepModelNamed:@"CBShadowsModel"];
            break;
        }

        default:
            break;
    }
    
#if RUN_TESTS
    if (runTestImages) {
        [self testSingleImage:info];
    }
#endif
    
}

- (MLModel *)modelForType:(const CBAR_PredictionType &)type {
    switch (type) {
        case CBAR_PredictionType_Semantic:
            return self.segmenterModel;
        case CBAR_PredictionType_Elevation:
            return self.elevationModel;
        case CBAR_PredictionType_Normals:
            return self.normalsModel;
        case CBAR_PredictionType_Shadows:
            return self.shadowsModel;
        default:
            break;
    }
}

- (void)vibratePhone
{
    if([[UIDevice currentDevice].model isEqualToString:@"iPhone"])
    {
        AudioServicesPlaySystemSound (1352); //works ALWAYS as of this post
    }
    else
    {
        // Not an iPhone, so doesn't have vibrate
        // play the less annoying tick noise or one of your own
        AudioServicesPlayAlertSound (1105);
    }
}

- (void) uploadDiagnosticImageAtPath:(const std::string&)path {
    if (!self.rootDelegate) return;
    
    NSString *nsPath = [NSString stringWithUTF8String:path.c_str()];
    dispatch_async(m_instance.upload_queue, ^{
        [self.rootDelegate uploadDiagnosticImageAtPath:nsPath];
    });
}

@end

void CBARCallback::drawingUpdated(const cv::Mat &image, const cv::Mat &color) {
    if (!m_instance) return;
    
    @autoreleasepool {
        dispatch_async(m_instance.unity_queue, ^{
            [m_instance drawingUpdated:image color:color];
        });
    }
}

void CBARCallback::sceneImageLoaded(const cv::Mat &image) {
    if (!m_instance) return;
    
    @autoreleasepool {
        dispatch_async(m_instance.unity_queue, ^{
            [m_instance sceneImageLoaded:image];
        });
    }
}

void CBARCallback::saveScreenshot(const std::string &path, bool isJPEG, int qualityLevel) {
    if (!m_instance) return;
    
    @autoreleasepool {
        dispatch_async(m_instance.unity_queue, ^{
            [m_instance saveScreenshot:path isJPEG:isJPEG qualityLevel:qualityLevel];
        });
    }
}

void CBARCallback::historyChanged(std::shared_ptr<cbpipe::UndoState> undo, bool forward) {
    if (!m_instance) return;
    
    @autoreleasepool {
        dispatch_async(m_instance.unity_queue, ^{
            [m_instance historyChanged:undo forward:forward];
        });
    }
}

void CBARCallback::colorsCallback(std::vector<cbar::ColorInfo>&colors) {
    if (!m_instance) return;
    
    @autoreleasepool {
        //[m_instance colorsCallback: colors];
    }
}

void CBARCallback::exposureCallback(cv::Point2f exposurePoint) {
    if (!m_instance) return;
    
    @autoreleasepool {
        //[m_instance exposureCallback:exposurePoint];
    }
}

void CBARCallback::focusCallback(cv::Point2f focusPoint) {
    if (!m_instance) return;
    
    @autoreleasepool {
        //[m_instance focusCallback:focusPoint];
    }
}

void CBARCallback::orientationNormalFound(const Eigen::Vector3f &normal) {
    if (!m_instance) return;
    
    @autoreleasepool {
        dispatch_async(m_instance.unity_queue, ^{
            [m_instance orientationNormalFound:normal];
        });
    }
}

std::vector<HitTestResult> CBARCallback::hitTestAtPoint(const cv::Point2f &point, HitTestResultType resultType) {
    
    @autoreleasepool {
        return [m_instance hitTestAtPoint:point resultType:resultType];
    }
}

void CBARCallback::loadDeepModel(const CBP_DeepModelInfo &model) {
    if (!m_instance) return;
    
    @autoreleasepool {
        return [m_instance loadDeepModel:model];
    }
}

cv::Mat CBARCallback::predict(const CBP_DeepModelInfo &model, const std::map<std::string, cv::Mat> &images) {
    if (!m_instance) return cv::Mat();
    
    @autoreleasepool {
        return [CoreMLTools predictWithModel:[m_instance modelForType:model.type] images:images];
    }
}

void CBARCallback::vibrateDevice() {
    [m_instance vibratePhone];
}

void CBARCallback::uploadDiagnosticImageAtPath(const std::string &path) {
    @autoreleasepool {
        [m_instance uploadDiagnosticImageAtPath:path];
    }
}
