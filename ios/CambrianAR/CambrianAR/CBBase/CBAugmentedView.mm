//
//  CBAugmentedView.m
//  CambrianAR
//
//  Created by Joel Teply on 11/17/15.
//  Copyright © 2015 Joel Teply. All rights reserved.
//

#import "CBAugmentedView.h"
#import "CBARInternal.h"

#include <cbar/CBAR_View.hpp>
#include <cbar/CBAR_GLOffscreenRenderer.hpp>

#include <imaging/ImageProcessing.h>
#import "ImageTranslation.h"

#import "CBVideoCamera.h"

#import <CoreMotion/CoreMotion.h>
#import <GLKit/GLKit.h>
#import "CBLicensing.h"
#import "CBTexture.h"

#include <imaging/Drawing.h>
#include <imaging/Geometry.h>
#include <imaging/Coloring.h>
#include <imaging/Imaging.h>
#include <imaging/Accelerated.h>
#include <imaging/ImageDefines.h>
#include <cbar/pipeline/CBP_Types.hpp>

#import <AssetsLibrary/AssetsLibrary.h>
#import <AVFoundation/AVFoundation.h>
#import <OpenGLES/ES2/gl.h>
#import <Endian.h>
#import <OpenGLES/ES2/glext.h>
#import <ARKit/ARKit.h>

#if COREML_ENABLED
#import <CoreML/CoreML.h>
#import "SurfaceMatcher.h"
#endif

#define SEND_GL_FRAMES 1
#define ACCELEROMETER_HZ 60.0f

#define DIRECT_GPU_IMAGE 1

#define USE_HDR_MAP 1

#if DEBUG
#   define DEBUG_SCENEKIT 0
#else
#   define DEBUG_SCENEKIT 0
#endif

static CBMutex m_singleInstanceMutex;

struct CBARCallback : CBAR_CallbackI {
    
    CBARCallback(void *context) : CBAR_CallbackI(context) {
        
    }
    
    virtual void historyChanged(std::shared_ptr<cbpipe::UndoState> undo, bool forward);
    
    virtual void colorsCallback(std::vector<cbar::ColorInfo>&colors);
    
    virtual void exposureCallback(cv::Point2f exposurePoint);
    
    virtual void focusCallback(cv::Point2f focusPoint);
    
#if COREML_ENABLED
    cv::Mat m_floodImage;
    CIImage *m_image;
    int64_t m_totalTime = 0;
    CGAffineTransform m_ciTransform;
    
    API_AVAILABLE(ios(11.0)) SurfaceMatcher *m_matcher = nil;
    API_AVAILABLE(ios(11.0)) SurfaceMatcherInput *m_features = nil;
    
    virtual bool prepareForSegmentation(PaintPointType type, cv::Ptr<cbpipe::texture_grid> grid);
    
    virtual bool isMatch(PaintPointType type, cv::Ptr<cbpipe::texture_grid> grid, cbpipe::texture_sample &elementA, cbpipe::texture_sample &elementB);
    void ensureData(cbpipe::texture_sample &element, const cv::Mat &rgbImage);
    
    virtual void segmentationCompleted(PaintPointType type, cv::Ptr<cbpipe::texture_grid> grid, cv::Mat &smallResult);
#endif
    
};


__weak static CBAugmentedView *m_instance;

@interface CBTexture ()

- (cbpipe::CBP_Texture) getCBPTexture;

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

@interface CBAugmentedView() <UIGestureRecognizerDelegate, CBVideoCameraDelegate, ARSCNViewDelegate, SCNPhysicsContactDelegate>
{
    BOOL m_licenseMessagedUser;
    
    CADisplayLink* m_displayLink;
    
    CVOpenGLESTextureCacheRef m_textureCache;
    CVOpenGLESTextureCacheRef m_renderTextureCache;
    
    CBAR_GLOffscreenRenderer m_offscreenRenderer;
    GLuint m_offscreenTexture;
    
    CGColorSpaceRef m_colorSpaceRef;
}

@property (strong, nonatomic) UIImage *blankImage;
@property (assign, nonatomic) BOOL hasPerformedLayout;


@property (nonatomic, assign) CGPoint startLocation;
@property (nonatomic, assign) cv::Vec3f cameraLocation;
@property (atomic, assign) int busyAdjusting;
@property (nonatomic, assign) BOOL isActive;

@property (nonatomic, strong) CBVideoCamera *videoCamera;
@property (nonatomic, strong) EAGLContext *eaglContext;

@property (nonatomic, readonly) BOOL hasPaintColor;

@property (nonatomic, assign) BOOL startingUp;
@property (nonatomic, assign) BOOL shuttingDown;

@property (nonatomic, assign) BOOL debugMode;
@property (nonatomic, assign) BOOL isCapturingState;
@property (nonatomic, assign) BOOL overlayBusy;
@property (nonatomic, assign) BOOL isPaused;
@property (nonatomic, assign) BOOL wasLive;
@property (nonatomic, readonly) BOOL coreIsUnavailable;
@property (nonatomic, assign) BOOL preparedViewport;
@property (nonatomic, assign) dispatch_once_t prepareToken;
@property (nonatomic, assign) BOOL isHalted;
@property (nonatomic, assign) BOOL isInBackground;

@property (strong, nonatomic) CMMotionManager *motionManager;
@property (nonatomic, strong) NSOperationQueue *motion_queue;

@property (assign, nonatomic) std::shared_ptr<CBAR_CallbackI> callback;

@property (nonatomic, strong) CIImage *coreImage;
@property (nonatomic, assign) int coreImageRotation;
@property (nonatomic, assign) int frameRotation;

@property (nonatomic, assign) CGSize viewSize;
@property (nonatomic, assign) UIInterfaceOrientation deviceOrientation;

@end

@implementation CBAugmentedView

@dynamic coreIsUnavailable;
@dynamic hasPaintColor;

@dynamic toolMode;

@dynamic undoSize;
@dynamic maxUndoSize;

@dynamic redoSize;
@dynamic maxRedoSize;

@dynamic isLive;

- (void)dealloc
{
    self.shuttingDown = YES;
    
    [self destroy];
    
    NSLog(@"Deallocating %@", NSStringFromClass([self class]));
}

+ (Class)layerClass
{
#ifdef HAS_METAL_SDK
    Class metalClass = NSClassFromString(@"CAMetalLayer");    //is metal runtime sdk available
    if ( metalClass != nil)
    {
        m_device = MTLCreateSystemDefaultDevice(); // is metal supported on this device (is there a better way to do this - without creating device ?)
        if (m_device)
        {
            [m_device retain];
            return metalClass;
        }
    }
#endif
    
    return [CAEAGLLayer class];
}

+ (CBAugmentedView *) getInstance {
    return m_instance;
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

- (void) destroy {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    m_instance = NULL;
    
    if (_coreView) {
        _coreView.reset();
    }
    
    self.callback = NULL;
    self.eaglContext = nil;
    
    if (m_colorSpaceRef) CGColorSpaceRelease(m_colorSpaceRef);
    
    m_singleInstanceMutex.unlock();
}

- (void) destroyGL {
    m_offscreenRenderer.destroy();
    
    if (m_textureCache) {
        CVOpenGLESTextureCacheFlush(m_textureCache, 0);
        CFRelease(m_textureCache);
        m_textureCache = NULL;
    }
    
    if (m_renderTextureCache) {
        CVOpenGLESTextureCacheFlush(m_renderTextureCache, 0);
        CFRelease(m_renderTextureCache);
        m_renderTextureCache = NULL;
    }
    
    self.preparedViewport = NO;
}

- (void) dispatch_cb:(dispatch_block_t) block {
    if (!m_instance || !m_instance.cb_queue) return;
    
    __weak typeof(self) weakSelf = self;
    dispatch_async(self.cb_queue, ^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        [EAGLContext setCurrentContext:strongSelf.eaglContext];
        if (block) block();
    });
}

- (void) dispatch_cb_get_result:(dispatch_block_t) block {
    
     __weak typeof(self) weakSelf = self;
    dispatch_sync(self.cb_queue, ^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        [EAGLContext setCurrentContext:strongSelf.eaglContext];
        if (block) block();
    });
}

- (UIViewController*)topViewController {
    return [self topViewControllerWithRootViewController:[UIApplication sharedApplication].keyWindow.rootViewController];
}

- (BOOL)isModal:(UIViewController*)viewController {
    if([viewController presentingViewController])
        return YES;
    if([[[viewController navigationController] presentingViewController] presentedViewController] == [viewController navigationController])
        return YES;
    if([[[viewController tabBarController] presentingViewController] isKindOfClass:[UITabBarController class]])
        return YES;
    
    return NO;
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

- (void)start
{
    __weak typeof(self) weakSelf = self;
    [self dispatch_cb_get_result:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        strongSelf.coreView->startRunning(strongSelf.isLive);
    }];
    
    [self resume];
}

- (void)stop
{
    if (nil != m_displayLink)
    {
        [m_displayLink invalidate];
        m_displayLink = nil;
    }
}

- (void)resume {
    if (nil == m_displayLink)
    {
        UIScreen *screen =  [UIScreen mainScreen];
        
        m_displayLink = [screen displayLinkWithTarget:self selector:@selector(renderFrame)];
        [m_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        m_displayLink.paused = false;
    }
}

- (void) addNotifications {
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillBecomeInactive)
                                                 name:UIApplicationWillResignActiveNotification
                                               object:nil ];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillBecomeActive)
                                                 name:UIApplicationDidBecomeActiveNotification
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

- (void)transitioningViewControllers:(NSNotification *) notification {

    UIViewController *currentVC = [self topViewController];
    
    if (currentVC != self.parentViewController) {
        [self stop];
        self.isHalted = YES;
    } else if (self.isHalted && currentVC == self.parentViewController) {
        self.isHalted = NO;
        [self resume];
    }
}

- (void) addGestureRecognizers {
    
    UIRotationGestureRecognizer *rotateGesture = [[UIRotationGestureRecognizer alloc]
                                                  initWithTarget:self action:@selector(handleRotate:)];
    rotateGesture.delegate = self;
    rotateGesture.cancelsTouchesInView = true;
    [self addGestureRecognizer:rotateGesture];
    
    UIPinchGestureRecognizer *zoomGesture = [[UIPinchGestureRecognizer alloc]
                                             initWithTarget:self action:@selector(handlePinch:)];
    zoomGesture.delegate = self;
    zoomGesture.cancelsTouchesInView = true;
    [self addGestureRecognizer:zoomGesture];
    
    UILongPressGestureRecognizer *longPressGestureRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(handleLongPress:)];
    longPressGestureRecognizer.minimumPressDuration = 1.0;
    [self addGestureRecognizer:longPressGestureRecognizer];
}

- (void) handleInternalError:(int)error {
    
}

- (void) generateScene {
    self.scene = [CBAugmentedScene new];
}

- (void) initialize {
    
    if (m_instance) {
        [m_instance destroy];
    }
    m_singleInstanceMutex.lock();
    
    m_instance = self;
    
    m_colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    
    dispatch_queue_attr_t highPriorityAttr = dispatch_queue_attr_make_with_qos_class (DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INITIATED,-1);
    self.cb_queue = dispatch_queue_create ("com.cambrian.cb_queue",highPriorityAttr);
    self.motion_queue = [[NSOperationQueue alloc] init];
    
    [self addGestureRecognizer:[[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(tapped:)]];
    
    self.deviceOrientation = [[UIApplication sharedApplication] statusBarOrientation];
    
    self.frameRotation = 0;
    
    switch (self.deviceOrientation) {
        case UIDeviceOrientationLandscapeRight:
            self.frameRotation = 180;
            self.coreImageRotation = 1;
            break;
        case UIDeviceOrientationPortrait:
            self.frameRotation = 90;
            self.coreImageRotation = 6;
            break;
        case UIDeviceOrientationPortraitUpsideDown:
            self.frameRotation = 270;
            self.coreImageRotation = 8;
            break;
        case UIDeviceOrientationLandscapeLeft:
        default:
            self.frameRotation = 0;
            self.coreImageRotation = 3;
            break;
    }
    
    self.videoCamera = [[CBVideoCamera alloc] init:self];
    
    if (!self.scene) [self generateScene];

    self.userInteractionEnabled = YES;
    
    [self addNotifications];
    [self addGestureRecognizers];
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

- (void)setupPhysics:(SCNScene *)scene {
    
    // For our physics interactions, we place a large node a couple of meters below the world
    // origin, after an explosion, if the geometry we added has fallen onto this surface which
    // is place way below all of the surfaces we would have detected via ARKit then we consider
    // this geometry to have fallen out of the world and remove it
    float planeDepth = -1.5;
    
    SCNBox *bottomPlane = [SCNBox boxWithWidth:1000 height:0.5 length:10000 chamferRadius:0];
    SCNMaterial *bottomMaterial = [SCNMaterial new];
    
    // Make it transparent so you can't see it
    bottomMaterial.diffuse.contents = [UIColor colorWithWhite:0.5 alpha:0.0];
    bottomPlane.materials = @[bottomMaterial];
    self.groundPlane = [SCNNode nodeWithGeometry:bottomPlane];
    
    
    // Place it way below the world origin to catch all falling cubes
    self.groundPlane.position = SCNVector3Make(0, planeDepth, 0);
    self.groundPlane.physicsBody = [SCNPhysicsBody
                              bodyWithType:SCNPhysicsBodyTypeKinematic
                              shape: nil];
    
    self.groundPlane.physicsBody.categoryBitMask = CollisionCategoryBottom;
    self.groundPlane.physicsBody.contactTestBitMask = CollisionCategoryCube;
    
    [scene.rootNode addChildNode:self.groundPlane];
    scene.physicsWorld.contactDelegate = self;
}


#pragma mark - SCNPhysicsContactDelegate

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
    
//    BOOL wasRunning = NO;
//    if (self.videoCamera.isRunning && !self.isPaused) {
//        [self pause:YES];
//        wasRunning = YES;
//    }
//
//    if (wasRunning) {
//        [self pause:NO];
//    }
}

- (void) updateDisplay {
    [self pause:!self.isPaused];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    [self touch:touches step:TouchStepBegan];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    [self touch:touches step:TouchStepMoved];
}

- (void) touchesEnded: (NSSet *) touches withEvent: (UIEvent *) event
{
    [self touch:touches step:TouchStepEnded];
}

- (void) touchesCancelled: (NSSet *) touches withEvent: (UIEvent *) event
{
    [self touch:touches step:TouchStepCancelled];
}

- (void) touch:(NSSet *)touches step:(TouchStep)step  {
    UITouch *touch = [touches anyObject];
    if (touches.count > 1) return;
    
    if (int(step) < 0) {
        return;
    }
    
    CGPoint touchPoint = [touch locationInView:self];
        
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
    
    if (@available(iOS 11.0, *)) {

        NSArray<ARHitTestResult *> *result = [self.arScene hitTest:touchPoint types:ARHitTestResultTypeFeaturePoint];
        
        // If there are multiple hits, just pick the closest plane
        ARHitTestResult * hitResult = [result firstObject];
        
        if (hitResult) {
            self.groundPlane.position = SCNVector3Make(self.groundPlane.position.x,
                                                       self.groundPlaneHeight,
                                                       self.groundPlane.position.z);
            [self.scene.selectedAsset touchedGroundPlaneAt:hitResult scene:self.arScene];
        }
        
    }
    
    __weak typeof(self) weakSelf = self;
    [self dispatch_cb_get_result:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        strongSelf.coreView->tappedAt(normalizedPoint);
    }];
}

- (void)handleRotate:(UIRotationGestureRecognizer *)recognizer {
    __weak typeof(self) weakSelf = self;
    
    auto step = [self gestureStateToStep:recognizer.state];

    float rotation = recognizer.rotation;
    
    CGPoint touchPoint = [recognizer locationInView:self];
    
    cv::Point2f normalizedPoint = cv::Point2f((touchPoint.x) / self.frame.size.width,
                                              (touchPoint.y) / self.frame.size.height);
    
    [self dispatch_cb:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        strongSelf.coreView->rotateGesture(rotation, normalizedPoint, step);
    }];
}

- (void)handlePinch:(UIPinchGestureRecognizer *)recognizer {
    
}

- (CBAugmentedModel *)assetForNode:(SCNNode *)node {
    
    for (NSString *assetID in self.scene.assets) {
        CBAugmentedAsset *asset = self.scene.assets[assetID];
        if ([asset isKindOfClass:[CBAugmentedModel class]]) {
            CBAugmentedModel *amodel = (CBAugmentedModel *) asset;
            if (amodel.modelNode == node) {
                return amodel;
            }
        }
    }
    return nil;
}

- (CBAugmentedAsset *) assetAtPoint:(CGPoint)touchPoint {
    if (@available(iOS 11.0, *)) {

        NSDictionary *options = @{SCNHitTestBoundingBoxOnlyKey : @YES};
        
        NSArray<SCNHitTestResult *> *results = [self.arScene hitTest:touchPoint options:options];
        
        for (NSString *assetID in self.scene.assets) {
            CBAugmentedAsset *asset = self.scene.assets[assetID];
            
            if ([asset isKindOfClass:[CBAugmentedModel class]]) {
                CBAugmentedModel *model = (CBAugmentedModel *)asset;
                if ([model isMyNode:results.lastObject.node]) {
                    return model;
                }
            }
        }
    }
    return nil;
}

- (void)handleLongPress:(UILongPressGestureRecognizer *)recognizer {
    if (recognizer.state != UIGestureRecognizerStateBegan) return;
    
    if (![self.rootDelegate respondsToSelector:@selector(assetLongPressed:)]) {
        return;
    }
    
    CGPoint holdPoint = [recognizer locationInView:self];
    
    if (CBAugmentedAsset *asset = [self assetAtPoint:holdPoint]) {
        [self.rootDelegate assetLongPressed:asset];
    }
}

- (void) applicationWillBecomeInactive {
    self.isInBackground = YES;
    
    if (self.isLive) {
        [self destroyGL];
    }
    
    self.wasLive = self.isLive;
    [self pause:YES];
    [self stop];
}

- (void) applicationWillBecomeActive {
    self.isInBackground = NO;
    
    if (self.wasLive) {
        [self pause:NO];
    } else {
        [self resume];
    }
}

- (void) orientationChanged {

}

- (void) captureCurrentState {
    __weak typeof(self) weakSelf = self;
    [self dispatch_cb_get_result:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        if (strongSelf.coreView) {
            strongSelf.coreView->captureCurrentState();
        }
    }];
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        [strongSelf pause:YES notifyCore:NO];
    });
}

- (void) pause:(BOOL)pause {
    [self pause:pause notifyCore:YES];
}

- (void) pause:(BOOL)pause notifyCore:(BOOL)notifyCore {
    if (self.shuttingDown ||
        self.startingUp ||
        self.isPaused == pause) return;
    
    self.isPaused = pause;
    
    if (pause && self.isLive) {
        [self startStopAccelerometer:NO];
        [self.videoCamera pause];
        self.wasLive = YES;
        _prepareToken = 0;
    } else if (self.wasLive) {
        [self startStopAccelerometer:YES];
        [self.videoCamera resume];
        [self start];
    }
    
    if (notifyCore) {
        __weak typeof(self) weakSelf = self;
        [self dispatch_cb_get_result:^{
            __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
            if (strongSelf.coreView) {
                strongSelf.coreView->pauseRendering(pause);
                strongSelf.coreView->setStillMode(pause);
            }
        }];
    }
}

#pragma mark - Filter Setup

- (void) exposureCallback:(cv::Point2f)region {
    CGPoint point = CGPointMake(region.x,region.y);
    
    [self.videoCamera exposeAtPoint:point useAutoExposure:false];
}

- (void) focusCallback:(cv::Point2f)region {
    
}

- (void) colorsCallback:(std::vector<cbar::ColorInfo>)colors {

}

- (void) historyChanged:(std::shared_ptr<cbpipe::UndoState>)undo forward:(BOOL)forward {
    
}

- (void)startStopAccelerometer:(BOOL)start {
    //if (hasARKit()) return;
    
    if (start) {
        if (!self.motionManager) {
            self.motionManager = [[CMMotionManager alloc] init];
            self.motionManager.deviceMotionUpdateInterval = 1.0f/ACCELEROMETER_HZ;
            
            //TODO: causes crash
            // Tell CoreMotion to show the compass calibration HUD when required to provide true north-referenced attitude
            //CMErrorDeviceRequiresMovement should be reported
            self.motionManager.showsDeviceMovementDisplay = YES;
        }
        

        // New in iOS 5.0: Attitude that is referenced to true north
        [self.motionManager startDeviceMotionUpdatesUsingReferenceFrame:CMAttitudeReferenceFrameXArbitraryCorrectedZVertical
                                                                toQueue:self.motion_queue withHandler:^(CMDeviceMotion * _Nullable motion, NSError * _Nullable error) {
                                                                    if (error) {
                                                                        NSLog(@"Got gyro error. Pause?");
                                                                    }
                                                                }];
        
    } else {
        [self.motionManager stopDeviceMotionUpdates];
    }
}

- (BOOL) startRunning
{
    self.preparedViewport = NO;
    
    [self start];
    
    if (@available(iOS 11.0, *)) {
        self.arScene = [[ARSCNView alloc] initWithFrame:self.frame];
        self.arScene.delegate = self;
        self.arScene.playing = YES;
        
        self.arScene.session = self.videoCamera.arSession;
        
        self.arScene.scene = [SCNScene new];
        
        [self setupPhysics: self.arScene.scene];
        
        self.arScene.antialiasingMode = SCNAntialiasingModeMultisampling4X;
        
#if DEBUG_SCENEKIT
        SCNDebugOptions opts = SCNDebugOptionNone;

        opts |= SCNDebugOptionShowBoundingBoxes;
        opts |= ARSCNDebugOptionShowFeaturePoints;
        opts |= SCNDebugOptionShowPhysicsShapes;

        self.arScene.debugOptions = opts;
        self.arScene.showsStatistics = YES;
#endif
        
#if USE_HDR_MAP
        NSString *hdrPath = [[NSBundle mainBundle] pathForResource:@"CBAssets/environment_blur" ofType:@"exr"];
        UIImage *env = [UIImage imageWithContentsOfFile:hdrPath];
        self.arScene.scene.lightingEnvironment.contents = env;
        self.arScene.scene.lightingEnvironment.intensity = 50;
        
        self.arScene.autoenablesDefaultLighting = NO;
        self.arScene.automaticallyUpdatesLighting = NO;
#else
        self.arScene.autoenablesDefaultLighting = YES;
        self.arScene.automaticallyUpdatesLighting = YES;
#endif
        
        
        [self insertSubview:self.arScene atIndex:0];
        
        [self setArSceneVisible:false];
    }
    
    AVAuthorizationStatus status = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
    
    if(status != AVAuthorizationStatusAuthorized) { // unauthorized
        NSLog(@"Denied camera access, cannot start");
        [self handleInternalError:0];
        return NO;
    }
    
    if (self.isPaused) {
        [self pause:NO];
        return YES;
    }
    else if (self.startingUp || self.shuttingDown || self.isActive) {
        return NO;
    }
    
    self.startingUp = YES;
    
    [self startStopAccelerometer:YES];
    
    [[UIApplication sharedApplication] setIdleTimerDisabled:YES];
    
    self.isActive = YES;
    self.startingUp = NO;

    [self.videoCamera startRunning];
    
    return YES;
}

- (void)renderer:(id <SCNSceneRenderer>)renderer updateAtTime:(NSTimeInterval)time {
    
    if (@available(iOS 11.0, *)) {
        ARLightEstimate *lightEstimate = self.arScene.session.currentFrame.lightEstimate;
        if (lightEstimate) {
            self.arScene.scene.lightingEnvironment.intensity = lightEstimate.ambientIntensity / 40;
        }
    }
}

- (BOOL) stopRunning;
{
    if (self.shuttingDown) return NO;
    
    self.shuttingDown = YES;
    
    [self.motionManager stopAccelerometerUpdates];
    
    [[UIApplication sharedApplication] setIdleTimerDisabled:NO];
    
    [self startStopAccelerometer:NO];
    
    [self.videoCamera stopRunning];
    
    [self stop];
    
    [self destroy];
    
    __weak typeof(self) weakSelf = self;
    [self dispatch_cb_get_result:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        //do not call method here since it auto gens a new one
        if (strongSelf->_coreView) {
            strongSelf->_coreView->stopRunning();
        }
    }];
    
    self.isActive = NO;
    self.shuttingDown = NO;
    
    return YES;
}

- (void)setArSceneVisible:(BOOL)value {
    if (@available(iOS 11.0, *)) {
        _arSceneVisible = value;
        self.arScene.hidden = !value;
    }
}

- (SCNVector3) getGroundPosition3D:(CGPoint)touchPoint  {
    
    if (@available(iOS 11.0, *)) {
        // -------------------------------------------------------------------------------
        // 1. Always do a hit test against exisiting plane anchors first.
        //    (If any such anchors exist & only within their extents.
        
        ARHitTestResult *planeResult = [self.arScene
                                        hitTest:CGPointMake(touchPoint.x, touchPoint.y + 200)
                                        types:ARHitTestResultTypeEstimatedHorizontalPlane].lastObject;
        if (planeResult) {
            //return result;
            return SCNVector3Make(planeResult.worldTransform.columns[3][0],
                                  planeResult.worldTransform.columns[3][1],
                                  planeResult.worldTransform.columns[3][2]);
        }
        
        SCNVector3 point3D = [self.arScene unprojectPoint:SCNVector3Make(touchPoint.x, touchPoint.y, 1.0)];
        simd_float4 screenPosOnFarClippingPlane;
        screenPosOnFarClippingPlane.x = point3D.x;
        screenPosOnFarClippingPlane.y = point3D.y;
        screenPosOnFarClippingPlane.z = point3D.z;
        
        simd_float4 rayOrigin = self.videoCamera.arSession.currentFrame.camera.transform.columns[3];
        
        simd_float4 rayDirection = simd_normalize(screenPosOnFarClippingPlane - rayOrigin);
        
        if (rayDirection.y > -0.1) {
            return SCNVector3Zero; // almost infinity, forget it.
        }
        
        //return rayIntersectionWithHorizontalPlane(rayOrigin: ray.origin, direction: ray.direction, planeY: pointOnPlane.y)
        float planeY = self.groundPlane.position.y;
        
        if (rayDirection.y == 0) {
            if (rayOrigin.y == planeY) {
                // The ray is horizontal and on the plane, thus all points on the ray intersect with the plane.
                // Therefore we simply return the ray origin.
                return SCNVector3Make(rayOrigin[0], rayOrigin[1], rayOrigin[2]);
            } else {
                // The ray is parallel to the plane and never intersects.
                return SCNVector3Zero;
            }
        }
        
        auto dist = (planeY - rayOrigin.y) / rayDirection.y;
        
        if (dist < 0) {
            return SCNVector3Zero;
        }
        
        auto result =  rayOrigin + (rayDirection * dist);
        
        return SCNVector3Make(result[0], result[1], result[2]);
    }
    
    return SCNVector3Zero;
}

matrix_float4x4 simdMatrixFromRotation(const CMRotationMatrix &r)
{
    vector_float4 col0 = { float(r.m11), float(r.m21), float(r.m31), 0.0f };
    vector_float4 col1 = { float(r.m12), float(r.m22), float(r.m32), 0.0f };
    vector_float4 col2 = { float(r.m13), float(r.m23), float(r.m33), 0.0f };
    vector_float4 col3 = { 0.0f,  0.0f,  1.0f, 1.0f };
    
    return matrix_from_columns(col0, col1, col2, col3);
}

matrix_float4x4 simdMatrixFromGLKMatrix4(const GLKMatrix4 &matrix)
{
    return matrix_from_columns(
                    vector_float4{ matrix.m00,matrix.m01,matrix.m02,matrix.m03 },
                    vector_float4{ matrix.m10,matrix.m11,matrix.m12,matrix.m13 },
                    vector_float4{ matrix.m20,matrix.m21,matrix.m22,matrix.m23 },
                    vector_float4{ matrix.m30,matrix.m31,matrix.m32,matrix.m33 }
                    );
}

void getCameraTransform(const matrix_float4x4 &matrixIn, UIInterfaceOrientation orient, Eigen::Matrix4f &matrixOut) {
    
    // rotation  matrix
    // [ cos    -sin]
    // [ sin     cos]
    matrix_float4x4 rotatedMatrix = matrix_identity_float4x4;
    switch (orient) {
        case UIInterfaceOrientationPortrait:
            rotatedMatrix.columns[0][0] = 0;
            rotatedMatrix.columns[0][1] = 1;
            rotatedMatrix.columns[1][0] = -1;
            rotatedMatrix.columns[1][1] = 0;
            break;
        case UIInterfaceOrientationLandscapeLeft:
            rotatedMatrix.columns[0][0] = -1;
            rotatedMatrix.columns[0][1] = 0;
            rotatedMatrix.columns[1][0] = 0;
            rotatedMatrix.columns[1][1] = -1;
            break;
        case UIInterfaceOrientationPortraitUpsideDown:
            rotatedMatrix.columns[0][0] = 0;
            rotatedMatrix.columns[0][1] = -1;
            rotatedMatrix.columns[1][0] = 1;
            rotatedMatrix.columns[1][1] = 0;
            break;
        default:
            break;
    }
    
    matrix_float4x4 result = matrix_multiply(matrixIn, rotatedMatrix);
    matToEigen(result, matrixOut);
}

void matToEigen(const matrix_float4x4 &matrixIn, Eigen::Matrix4f &matrixOut) {
    vector_float4 c0 = matrixIn.columns[0];
    vector_float4 c1 = matrixIn.columns[1];
    vector_float4 c2 = matrixIn.columns[2];
    vector_float4 c3 = matrixIn.columns[3];
    
    Eigen::Matrix4f result;
    
    matrixOut <<    c0.x, c1.x, c2.x, c3.x,
                    c0.y, c1.y, c2.y, c3.y,
                    c0.z, c1.z, c2.z, c3.z,
                    c0.w, c1.w, c2.w, c3.w;
}

- (BOOL) coreIsUnavailable {
    return self.shuttingDown || self.startingUp || self.isCapturingState || !_coreView;
}

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer
{
    return YES;
}

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

//CALLBACK is on correct queue

//bug, so have to skip

- (void)prepareViewportForWidth:(int)width height:(int)height {
    return [self prepareViewportForWidth:width height:height rotation:self.frameRotation];
}

- (void)prepareViewportForWidth:(int)width height:(int)height rotation:(int)rotation {
    
    __weak typeof(self) weakSelf = self;
    
    dispatch_once(&_prepareToken, ^{
        dispatch_async(dispatch_get_main_queue(), ^{
            __strong typeof(self) strongSelf = weakSelf; if (!strongSelf || !strongSelf.coreView) return;
            
            float displayDensity = [[UIScreen mainScreen] scale];
            displayDensity = 2.0;
            
            strongSelf.viewSize = CGSizeMake(displayDensity * strongSelf.frame.size.width, displayDensity * strongSelf.frame.size.height);
            
            [strongSelf setContentScaleFactor: displayDensity];
            
            
            if (strongSelf.coreView && !strongSelf.coreView->getScene()) {
                [strongSelf dispatch_cb_get_result:^{
                    __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
                    strongSelf.coreView->setScene(strongSelf.scene.coreScene);
                }];
            }

            float fov = 1.11568153;
            Eigen::Matrix3f intrinsics = Eigen::Matrix3f::Identity();
            
            if (strongSelf.videoCamera.inputCamera.activeFormat) {
                fov = toRadians(strongSelf.videoCamera.inputCamera.activeFormat.videoFieldOfView);
            }

            strongSelf.coreView->prepareViewport((__bridge void *)strongSelf.layer,
                                                 (__bridge void *)strongSelf.eaglContext,
                                                 int(width), int(height),
                                                 strongSelf.viewSize.width,
                                                 strongSelf.viewSize.height,
                                                 fov, rotation, intrinsics);
            
            strongSelf.eaglContext = [EAGLContext currentContext];
            strongSelf.preparedViewport = YES;
        });
    });
}

- (void) sendFrame:(CVPixelBufferRef)pixelBuffer {
    
    if (self.shuttingDown || !self.isActive || self.isInBackground || !self.hasPerformedLayout) {
        return;
    }
    
    //printf("Transform=%f, %f, %f\n", transform.columns[0][0], transform.columns[0][1], transform.columns[0][2]);
    
    if (!self.preparedViewport) {
        size_t width = CVPixelBufferGetWidth(pixelBuffer);
        size_t height = CVPixelBufferGetHeight(pixelBuffer);
        [self prepareViewportForWidth:int(width) height:int(height)];
    }
    else {
        
#if SEND_GL_FRAMES
        [self sendOpenGLFrame:pixelBuffer];
#else
        [self sendPixelFrame:pixelBuffer];
#endif
        
    }
    
    if (hasARKit()) {
        if (@available(iOS 11.0, *)) {
            ARCamera *camera = self.arScene.session.currentFrame.camera;
            
            Eigen::Matrix4f transform;
            getCameraTransform(camera.transform, self.deviceOrientation, transform);
            self.scene.coreScene->setCameraTransform(transform);
            
            CGFloat unityCameraNearZ = .01;
            CGFloat unityCameraFarZ = 30;
            
            matrix_float4x4 projectionMatrix = [camera projectionMatrixForOrientation:self.deviceOrientation
                                                                         viewportSize:self.viewSize
                                                                                zNear:unityCameraNearZ
                                                                                 zFar:unityCameraFarZ];
            Eigen::Matrix4f proj;
            matToEigen(projectionMatrix, proj);
            
            self.scene.coreScene->setCameraProjection(proj);
        }
    }
    else if (self.motionManager) {
#if 1
        const matrix_float4x4 r = simdMatrixFromRotation(self.motionManager.deviceMotion.attitude.rotationMatrix);

        Eigen::Matrix4f transform;
        getCameraTransform(r, _deviceOrientation, transform);
        self.scene.coreScene->setWorldTransform(transform, false);
#else
        cv::Vec4f cameraPosition = cv::Vec4f(0.0, 0.0, 1.0, 1.0);
        const CMQuaternion &r = self.motionManager.deviceMotion.attitude.quaternion;
        self.scene.coreScene->setWorldTransform(cv::Vec4f(r.w, r.x, r.y, r.z), cameraPosition);
#endif
    }
}

- (BOOL)initializeBuffersWithOutputWidth:(int)width height:(int)height
{
    BOOL success = YES;
    
    glDisable( GL_DEPTH_TEST );
    
    m_offscreenTexture = m_offscreenRenderer.create(width, height, GL_TEXTURE_2D, true);
    
    CVReturn err = CVOpenGLESTextureCacheCreate( kCFAllocatorDefault, NULL, self.eaglContext, NULL, &m_textureCache );
    if ( err ) {
        NSLog( @"Error at CVOpenGLESTextureCacheCreate %d", err );
        success = NO;
        return NO;
    }
    
    err = CVOpenGLESTextureCacheCreate( kCFAllocatorDefault, NULL, self.eaglContext, NULL, &m_renderTextureCache );
    if ( err ) {
        NSLog( @"Error at CVOpenGLESTextureCacheCreate %d", err );
        success = NO;
        return NO;
    }
    
    return YES;
}

- (void) sendOpenGLFrame:(CVPixelBufferRef)pixelBuffer {
    
    if (![EAGLContext setCurrentContext:self.eaglContext]) {
        printf("EAGLContext set failed\n");
        return;
    }
    
#if DIRECT_GPU_IMAGE
    @autoreleasepool {
        self.coreImage = [[CIImage alloc] initWithCVPixelBuffer:pixelBuffer];
    }
#endif
    
    //getting output from scenekit
    //https://stackoverflow.com/questions/29060465/rendering-a-scenekit-scene-to-video-output
    
    size_t width = CVPixelBufferGetWidth(pixelBuffer);
    size_t height = CVPixelBufferGetHeight(pixelBuffer);
    auto type = CVPixelBufferGetPixelFormatType(pixelBuffer);
    bool isYUV = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange == type || kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange == type;
    
    if (!m_textureCache) {
        [self initializeBuffersWithOutputWidth:int(width) height:int(height)];
    }
    
    if (!m_textureCache) return;
    
    GLenum err = 0;
    
    err = glGetError();
    NSAssert(0 == err, @"GL call failed with GL error: 0x%04x.", err);

    CVReturn res = 0;
    
    cbar::RawFrame frame;
    
    frame.frameRotation = self.frameRotation;
    frame.dataLength = CVPixelBufferGetDataSize(pixelBuffer);
    
    if (isYUV) {
        CVOpenGLESTextureRef lumTexture = NULL;
        
        //kCVReturnPixelBufferNotOpenGLCompatible
        // Y-plane
        //glActiveTexture(GL_TEXTURE0);
        res = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                           m_textureCache,
                                                           pixelBuffer,
                                                           NULL,
                                                           GL_TEXTURE_2D,
                                                           GL_RED_EXT,
                                                           int(width),
                                                           int(height),
                                                           GL_RED_EXT,
                                                           GL_UNSIGNED_BYTE,
                                                           0,
                                                           &lumTexture);
        
        if (res != kCVReturnSuccess)
        {
            NSLog(@"Could not create Y texture from image. %d", err);
            return;
        }
        auto lumTextureTarget = CVOpenGLESTextureGetTarget(lumTexture);
        auto lumTextureName = CVOpenGLESTextureGetName(lumTexture);
        
        glBindTexture(lumTextureTarget, lumTextureName);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // UV-plane
        CVOpenGLESTextureRef chromaTexture = NULL;
        //glActiveTexture(GL_TEXTURE1);
        res = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                           m_textureCache,
                                                           pixelBuffer,
                                                           NULL,
                                                           GL_TEXTURE_2D, GL_RG_EXT,
                                                           int(width) / 2,
                                                           int(height) / 2,
                                                           GL_RG_EXT,
                                                           GL_UNSIGNED_BYTE, 1,
                                                           &chromaTexture);
       
        if (res != kCVReturnSuccess)
        {
            NSLog(@"Could not create luminance texture from image. %d", err);
            return;
        }
        auto chromaTextureTarget = CVOpenGLESTextureGetTarget(chromaTexture);
        auto chromaTextureName = CVOpenGLESTextureGetName(chromaTexture);
        
        glBindTexture(chromaTextureTarget, chromaTextureName);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        m_offscreenRenderer.render(lumTextureName, chromaTextureName);
        
        frame.frameType = frame_type_420YpCbCr8;
        
        frame.data = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 0);
        frame.chromaData = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 1);
        
        frame.width = CVPixelBufferGetWidthOfPlane(pixelBuffer, 0);
        frame.height = CVPixelBufferGetHeightOfPlane(pixelBuffer, 0);
        frame.textureID = m_offscreenTexture;
        frame.stride = CVPixelBufferGetBytesPerRowOfPlane( pixelBuffer, 0);

        self.coreView->addFrame(frame);
        
        if (self.isHalted || self.isInBackground) return;
        
        if (chromaTexture) CFRelease(chromaTexture);
        if (lumTexture) CFRelease(lumTexture);
    }
    else {
        
        CVOpenGLESTextureRef bgraTexture = NULL;
        
        res = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                           m_textureCache,
                                                           pixelBuffer,
                                                           NULL, // texture attributes
                                                           GL_TEXTURE_2D,
                                                           GL_RGBA, // opengl format
                                                           int(width),
                                                           int(height),
                                                           GL_BGRA, //input format
                                                           GL_UNSIGNED_BYTE,
                                                           0,
                                                           &bgraTexture);
        
        if (res != kCVReturnSuccess) {
            printf("Texture creation failed %d\n", res);
            return;
        }
        
        // Set up our destination pixel buffer as the framebuffer's render target.
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( CVOpenGLESTextureGetTarget( bgraTexture ), CVOpenGLESTextureGetName( bgraTexture ) );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        
        frame.frameType = frame_type_bgra;
        frame.stride = CVPixelBufferGetBytesPerRow( pixelBuffer );
        frame.data = (uint8_t*)CVPixelBufferGetBaseAddress(pixelBuffer);
        frame.textureID = CVOpenGLESTextureGetName(bgraTexture);
        frame.width =  width;
        frame.height = height;
        
        self.coreView->addFrame(frame);
        if (bgraTexture) CFRelease(bgraTexture);
    }
}

- (void) sendPixelFrame:(CVPixelBufferRef)pixelBuffer transform:(matrix_float4x4)transform {
    
    size_t width = CVPixelBufferGetWidth(pixelBuffer);
    size_t height = CVPixelBufferGetHeight(pixelBuffer);
    auto type = CVPixelBufferGetPixelFormatType(pixelBuffer);
    bool isYUV = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange == type || kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange == type;
    
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow( pixelBuffer );
    uint8_t *baseAddress = (uint8_t*)CVPixelBufferGetBaseAddress(pixelBuffer);
    
    cbar::RawFrame frame;
    frame.frameType = isYUV ? frame_type_420YpCbCr8 : frame_type_bgra;
    
    frame.width = width;
    frame.height = height;
    frame.stride = bytesPerRow;
    
    if (isYUV) {
        frame.data = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(pixelBuffer, 0);
    } else {
        frame.data = baseAddress;
    }
    
    frame.dataLength = CVPixelBufferGetDataSize(pixelBuffer);
    
    self.coreView->addFrame(frame);
}

#pragma mark - Analysis Private Methods

#pragma mark - Painting

- (void) setDebugMode:(BOOL)value;
{
    _debugMode = value;
}

- (void) clearAll {
    __weak typeof(self) weakSelf = self;
    [self dispatch_cb:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        strongSelf.coreView->clearAll();
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

- (void)renderFrame {
    if (!self.isInBackground && self.preparedViewport && self.isPaused) {
        self.coreView->stillRender();
    }
}

- (int) undoSize {
    __block int numUndoItems = 0;
    __weak typeof(self) weakSelf = self;
    [self dispatch_cb_get_result:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        numUndoItems = strongSelf.coreView->getScene()->getUndoSize();
    }];
    
    return numUndoItems;
}

- (int) maxUndoSize {
    __block int maxUndoItems = 0;
    __weak typeof(self) weakSelf = self;
    [self dispatch_cb_get_result:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        maxUndoItems = strongSelf.coreView->getScene()->getMaxUndoSize();
    }];
    
    return maxUndoItems;
}

- (void) setMaxUndoSize:(int)size {
    __weak typeof(self) weakSelf = self;
    [self dispatch_cb:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        strongSelf.coreView->getScene()->setMaxUndoSize(size);
    }];
}

- (void) undo {
    __weak typeof(self) weakSelf = self;
    [self dispatch_cb_get_result:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        strongSelf.coreView->getScene()->undoState();
    }];
}

- (int) maxRedoSize {
    __block int maxRedoItems = 0;
    __weak typeof(self) weakSelf = self;
    [self dispatch_cb_get_result:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        maxRedoItems = strongSelf.coreView->getScene()->getMaxRedoSize();
    }];
    
    return maxRedoItems;
}

- (void) setMaxRedoSize:(int)size {
    __weak typeof(self) weakSelf = self;
    [self dispatch_cb:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        strongSelf.coreView->getScene()->setMaxRedoSize(size);
    }];
}

- (int) redoSize {
    __block int numRedoItems = 0;
    __weak typeof(self) weakSelf = self;
    [self dispatch_cb_get_result:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        numRedoItems = strongSelf.coreView->getScene()->getRedoSize();
    }];
    
    return numRedoItems;
}

- (void) redo {
    __weak typeof(self) weakSelf = self;
    [self dispatch_cb_get_result:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        strongSelf.coreView->getScene()->redoState();
    }];
}

- (BOOL) isLive {
    return self.videoCamera.isRunning;
}

- (void) setScene:(CBAugmentedScene *)scene {
    _scene = scene;
    
    auto frame = scene.coreScene->getVideoFrame();
    
    if (!frame.empty()) {        
        [self pause:YES];
        
        self.prepareToken = 0;
        self.frameRotation = 0;
        [self prepareViewportForWidth:frame->frameSize().width height:frame->frameSize().height];
        [self start];
    }
}

- (void)session:(ARSession *)session didUpdateFrame:(ARFrame *)frame API_AVAILABLE(ios(11.0))  {
    
    //printf("elevation: %f, %f, %f\n", frame.camera.transform.columns[3][0], frame.camera.transform.columns[3][1], frame.camera.transform.columns[3][2]);
}

- (void)session:(ARSession *)session didAddAnchors:(NSArray<ARAnchor*>*)anchors API_AVAILABLE(ios(11.0)) {
    if (@available(iOS 11.0, *)) {
        if (![anchors.firstObject isKindOfClass:[ARPlaneAnchor class]]) {
            return;
        }
        
        //ARPlaneAnchor *anchor = (ARPlaneAnchor *) anchors.firstObject;
        
        [self addAnchor:session];
    }
    
}

- (void) addAnchor:(ARSession *)session API_AVAILABLE(ios(11.0)) {
//    if (@available(iOS 11.0, *)) {
//        auto types = ARHitTestResultTypeEstimatedHorizontalPlane | ARHitTestResultTypeExistingPlane | ARHitTestResultTypeExistingPlaneUsingExtent;
//        NSArray<ARHitTestResult *> *planes = [self.videoCamera.arSession.currentFrame hitTest:CGPointMake(0.5, 0.5) types:types];
//        NSLog(@"lowest plane = %f", planes.firstObject.worldTransform.columns[3][1]);
//        //self.groundPlane = planes.lastObject.anchor;
//        //NSLog(@"arSession=%@", self.videoCamera.arSession);
//
//
//        //self.groundPlane = [[ARAnchor alloc] initWithTransform:planes.lastObject.worldTransform];
//        // Add it to the session
//        //[session addAnchor:self.groundPlane];
//    }
}

- (void)session:(ARSession *)session didUpdateAnchors:(NSArray<ARAnchor*>*)anchors API_AVAILABLE(ios(11.0)) {
    //NSLog(@"anchor updated: %@", anchors.firstObject);
    if (@available(iOS 11.0, *)) {
        if (![anchors.firstObject isKindOfClass:[ARPlaneAnchor class]]) {
            return;
        }
        
        //NSLog(@"num anchors=%lu", (unsigned long) anchors.count);
        
        //[self addAnchor:session];
    }
   
}

- (void)session:(ARSession *)session didRemoveAnchors:(NSArray<ARAnchor*>*)anchors API_AVAILABLE(ios(11.0)) {
    if (@available(iOS 11.0, *)) {
        

    }
}
- (void)renderer:(id <SCNSceneRenderer>)renderer didAddNode:(SCNNode *)node forAnchor:(ARAnchor *)anchor API_AVAILABLE(ios(11.0)) {
    //NSLog(@"node added");
}

- (void)renderer:(id <SCNSceneRenderer>)renderer willUpdateNode:(SCNNode *)node forAnchor:(ARAnchor *)anchor API_AVAILABLE(ios(11.0)) {
    //NSLog(@"node will update");
}

- (void)renderer:(id <SCNSceneRenderer>)renderer didUpdateNode:(SCNNode *)node forAnchor:(ARAnchor *)anchor API_AVAILABLE(ios(11.0)) {
    //NSLog(@"node did update");
}

- (void)renderer:(id <SCNSceneRenderer>)renderer didRemoveNode:(SCNNode *)node forAnchor:(ARAnchor *)anchor API_AVAILABLE(ios(11.0)) {
    //NSLog(@"node removed");
}

@end


void CBARCallback::historyChanged(std::shared_ptr<cbpipe::UndoState> undo, bool forward) {
    if (!m_instance) return;
    //__weak typeof(CBAugmentedView) *weakSelf = (__bridge CBAugmentedView *)_instance;
    
    @autoreleasepool {
        [m_instance historyChanged:undo forward:forward];
    }
}

void CBARCallback::colorsCallback(std::vector<cbar::ColorInfo>&colors) {
    if (!m_instance) return;
    //__weak typeof(CBAugmentedView) *weakSelf = (__bridge CBAugmentedView *)_instance;
    @autoreleasepool {
        [m_instance colorsCallback: colors];
    }
}

void CBARCallback::exposureCallback(cv::Point2f exposurePoint) {
    if (!m_instance) return;
    //__weak typeof(CBAugmentedView) *weakSelf = (__bridge CBAugmentedView *)_instance;
    @autoreleasepool {
        [m_instance exposureCallback:exposurePoint];
    }
}

void CBARCallback::focusCallback(cv::Point2f focusPoint) {
    if (!m_instance) return;
    //__weak typeof(CBAugmentedView) *weakSelf = (__bridge CBAugmentedView *)m_context;
    @autoreleasepool {
        [m_instance focusCallback:focusPoint];
    }
}

#if COREML_ENABLED
bool CBARCallback::prepareForSegmentation(PaintPointType type, cv::Ptr<cbpipe::texture_grid> grid) {
    if (!m_instance) return false;
    
    return false;//turned off.
    
    @autoreleasepool {
        if (type == PaintPointType_Surface) {
            //cv::resize(reducedRGB, m_floodImage, grid->srcImage.size());
            
            if (@available(iOS 11.0, *)) {
                m_matcher = [[SurfaceMatcher alloc] init];
                
                //            auto test = [ImageTranslation CVMatForCoreImage:m_instance.coreImage];
                //            Diagnostics::SaveDiagnosticImage(false, test, "m_image.png");
                
                @autoreleasepool {
                    if (m_instance.coreImage) {
                        bool isRotated = m_instance.frameRotation == 90 || m_instance.frameRotation == 270;
                        CIFilter *scaleFilter = [CIFilter filterWithName:@"CILanczosScaleTransform"];
                        float originalHeight = isRotated ? m_instance.coreImage.extent.size.width : m_instance.coreImage.extent.size.height;
                        float originalWidth = isRotated ? m_instance.coreImage.extent.size.height : m_instance.coreImage.extent.size.width;
                        
                        float yScale=(float)grid->srcImage.rows / (float)originalHeight;
                        float xScale=(float)grid->srcImage.cols / (float)originalWidth;
                        float scale=fminf(yScale, xScale);
                        
                        [scaleFilter setValue:[NSNumber numberWithFloat:scale]
                                       forKey:@"inputScale"];
                        [scaleFilter setValue:[NSNumber numberWithFloat:1.0]
                                       forKey:@"inputAspectRatio"];
                        [scaleFilter setValue: m_instance.coreImage
                                       forKey:@"inputImage"];
                        
                        m_image = [[scaleFilter valueForKey:@"outputImage"] imageByApplyingOrientation:m_instance.coreImageRotation];
                        
                        m_ciTransform = CGAffineTransformMakeScale(1, -1);
                        m_ciTransform = CGAffineTransformTranslate(m_ciTransform, 0, -m_image.extent.size.height);
                        
                    }
                    else {
                        cv::cvtColor(grid->srcImage, m_floodImage, CV_RGB2RGBA);
                        //Diagnostics::SaveDiagnosticImage(false, m_floodImage, "self.png");
                    }
                }
                
                CBLog(@"Last time taken=%f", usec_to_seconds(m_totalTime));
                m_totalTime = 0;
                
                return true;
            }
            
        }
    }
    
    return false;//will not delegate
}

void CBARCallback::segmentationCompleted(PaintPointType type, cv::Ptr<cbpipe::texture_grid> grid, cv::Mat &smallResult) {
    //cleanup
    @autoreleasepool {
        for (int i=0; i<grid->rows.size(); i++) {
            for (int j=0; j<grid->rows[i].samples.size(); j++) {
                CVPixelBufferRef ref = (CVPixelBufferRef)grid->rows[i].samples[j].userData;
                if (ref) {
                    CVPixelBufferRelease(ref);
                    grid->rows[i].samples[j].userData = 0;
                }
            }
        }
    }
}

void CBARCallback::ensureData(cbpipe::texture_sample &element, const cv::Mat &rgbImag) {
    if (!element.userData) {
        @autoreleasepool {
            CIImage *crop;
            if (m_instance.coreImage) {
                const CGRect ciRect = CGRectApplyAffineTransform(CGRectMake(element.roi.x, element.roi.y, element.roi.width, element.roi.height), m_ciTransform);
                crop = [m_image imageByCroppingToRect:ciRect];
            } else {
                cv::Mat rgbCrop = m_floodImage(element.roi);
                crop = [ImageTranslation ciimageWithBytes:rgbCrop.ptr()
                                                imageSize:CGSizeMake(rgbCrop.cols, rgbCrop.rows)
                                                 channels:rgbCrop.channels() isBGR:NO];
            }
            
            static NSDictionary *pOptions = @{(NSString*)kCVPixelBufferOpenGLESCompatibilityKey : @YES};
            
            element.userData = (char *)[ImageTranslation pixelBufferFromCoreImage:crop options:pOptions];
        }
    }
}

bool CBARCallback::isMatch(PaintPointType type, cv::Ptr<cbpipe::texture_grid> grid, cbpipe::texture_sample &elementA, cbpipe::texture_sample &elementB) {
    if (!m_instance) return false;
    
    if (@available(iOS 11.0, *)) {
        
        @autoreleasepool {
            ensureData(elementA, grid->srcImage);
            ensureData(elementB, grid->srcImage);
            
            NSError *error = nil;
            
            if (!m_features) m_features = [SurfaceMatcherInput new];
            
            m_features.image_a = (CVPixelBufferRef) elementA.userData;
            m_features.image_b = (CVPixelBufferRef) elementB.userData;
            
            if (!m_features.image_a || !m_features.image_b) return false;
            
            auto start = sys_usec_time();
            SurfaceMatcherOutput *output = [m_matcher predictionFromFeatures:m_features error:&error];
            auto taken = sys_usec_time() - start;
            m_totalTime += taken;
            
            double maxClass = 0;
            int value = [ImageTranslation getMaxClassForArray:output.is_match probability:&maxClass];
            
            return value > 0;
        }
    }
    
    return true;
}
#endif
