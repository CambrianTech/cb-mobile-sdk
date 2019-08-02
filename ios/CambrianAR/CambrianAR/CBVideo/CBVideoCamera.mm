//
//  CBVideoCamera.m
//  Cambrian
//
//  Created by Joel Teply on 10/19/12.
//
//

#import "CBVideoCamera.h"
#import <ImageIO/CGImageProperties.h>
#import "VideoTranslation.h"
#import "ImageTranslation.h"

#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#import <sys/utsname.h>

//just used for making sure fps is good
#define FORCE_FPS 0

#define VIDEO_FORMAT kCVPixelFormatType_420YpCbCr8BiPlanarFullRange

@interface DeviceInfo : NSObject 
+ (NSString *)model;
@end

@implementation DeviceInfo

+ (NSString *)model
{
    struct utsname systemInfo;
    uname(&systemInfo);
    
    return [NSString stringWithCString: systemInfo.machine encoding: NSUTF8StringEncoding];
}

//iphone 7=3, iphone8=4, etc.. ipad air2=2, pro=3
+ (int)level {
    NSString *model = DeviceInfo.model;
    
    NSArray *substrings = [model componentsSeparatedByString:@","];
    NSString *majorModel = [substrings objectAtIndex:0];
    
    NSCharacterSet *nonDigitCharacterSet = [[NSCharacterSet decimalDigitCharacterSet] invertedSet];
    NSString *numbers = [[majorModel componentsSeparatedByCharactersInSet:nonDigitCharacterSet] componentsJoinedByString:@""];
    
    //http://stackoverflow.com/questions/26028918/ios-how-to-determine-the-current-iphone-device-model-in-swift
    if ([majorModel hasPrefix:@"iPhone"]) {
        return fmax(0, [numbers integerValue] - 6);
    } else if ([majorModel hasPrefix:@"iPad"]) {
        return fmax(0, [numbers integerValue] - 3);
    }
    
    return 0;
}

@end

@interface CBVideoCamera() <AVCaptureVideoDataOutputSampleBufferDelegate, ARSessionDelegate> {
    AVCaptureDevice *_inputCamera;
}

@property (nonatomic, weak) id <CBVideoCameraDelegate>delegate;
@property (nonatomic, strong) AVCaptureVideoDataOutput *videoOutput;
@property (nonatomic, strong) AVCaptureDeviceInput *videoInput;
@property (nonatomic, strong) AVCaptureSession *session;

@property (nonatomic, assign) BOOL isNV21;

@property (nonatomic, assign) int frameIndex;
@property (nonatomic, assign) BOOL isLockedForConfiguration;
@property CVEAGLContext eaglContext;
@property (atomic, assign) BOOL frameBusy;

@property (atomic, strong) dispatch_queue_t arQueue;

@end


@implementation CBVideoCamera

@synthesize inputCamera = _inputCamera;
@synthesize arSession = _arSession;

- (void)dealloc {
    NSLog(@"VideoCamera deallocated");
    
    [self stopRunning];
    
    // Fix intermittent crash:
    // http://stackoverflow.com/questions/18757784/avcapturesession-addinput-not-working-with-ios7
    [self.session removeInput:self.videoInput];
    [self.session removeOutput:self.videoOutput];
    self.session = nil;
    self.videoInput = nil;
    self.videoOutput = nil;
}

- (id) init:(id <CBVideoCameraDelegate>)delegate;
{
    
    if (self = [super init]) {
        self.delegate = delegate;
        self.isNV21 = NO;
        self.isPaused = YES;
        
        BOOL success = false;
        
        dispatch_queue_attr_t highPriorityAttr = dispatch_queue_attr_make_with_qos_class (DISPATCH_QUEUE_CONCURRENT, QOS_CLASS_USER_INTERACTIVE,-1);
        self.arQueue = dispatch_queue_create ("com.cambrian.ar_queue",highPriorityAttr);
        
        if ((success = hasARKit())) {
            [self initializeARSession];
        } else {
            success = [self initializeCameras];
        }
        
        if (!success) {
            return nil;
        }
    }
    return self;
}

- (void)captureStillImageWithCompletionHandler:(void (^)(UIImage *image, NSError *error))block;
{
    [self playShutterSound];
    [self flashScreen];
}

- (void) playShutterSound {
    SystemSoundID shutterSoundID = 1108;
    //play shutter sound (app developer himself, shutters at this apple requirement)
    AudioServicesPlaySystemSound(shutterSoundID);
}

- (void) flashScreen
{
    //__weak typeof(self) weakSelf = self;
    
//    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
//        __strong typeof(self) strongSelf = weakSelf;
//    });
}


- (BOOL)initializeCameras {
    
    //init video session and get ahold of cameras
    self.session = [[AVCaptureSession alloc] init];
    NSArray* devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    
    for (AVCaptureDevice* device in devices) {
        if (device.position == AVCaptureDevicePositionBack) {
            _inputCamera = device;
        }
    }
    
    if (!self.inputCamera) {
        return NO;
    }
    
    NSError* error = nil;
    self.videoInput = [AVCaptureDeviceInput deviceInputWithDevice:self.inputCamera error:&error];
    [self.session addInput:self.videoInput];
    if (error) {
        return NO;
    }
    
    
    int deviceLevel = DeviceInfo.level;
    
    int desiredFPS = 30;
    
    if ((UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)) {
        self.session.sessionPreset = AVCaptureSessionPresetPhoto;
    } else {
        if (deviceLevel > 2) {
            desiredFPS = 60;
        }
        self.session.sessionPreset = AVCaptureSessionPresetHigh;
    }
    
#if FORCE_FPS
    if (desiredFPS > 30) {
        for(AVCaptureDeviceFormat *vFormat in self.inputCamera.formats )
        {
            CMFormatDescriptionRef description= vFormat.formatDescription;
            float maxrate=((AVFrameRateRange*)[vFormat.videoSupportedFrameRateRanges objectAtIndex:0]).maxFrameRate;
            
            if(maxrate == desiredFPS && CMFormatDescriptionGetMediaSubType(description)==kCVPixelFormatType_420YpCbCr8BiPlanarFullRange)
            {
                if ( YES == [self.inputCamera lockForConfiguration:NULL] )
                {
                    self.inputCamera.activeFormat = vFormat;
                    [self.inputCamera setActiveVideoMinFrameDuration:CMTimeMake(1,desiredFPS)];
                    [self.inputCamera setActiveVideoMaxFrameDuration:CMTimeMake(1,desiredFPS)];
                    [self.inputCamera unlockForConfiguration];
                    //NSLog(@"formats  %@ %@ %@",vFormat.mediaType,vFormat.formatDescription,vFormat.videoSupportedFrameRateRanges);
                }
            }
        }
    } else {
        if ( YES == [self.inputCamera lockForConfiguration:NULL] )
        {
            [self.inputCamera setActiveVideoMinFrameDuration:CMTimeMake(1,desiredFPS)];
            [self.inputCamera setActiveVideoMaxFrameDuration:CMTimeMake(1,desiredFPS)];
            [self.inputCamera unlockForConfiguration];
        }
    }
#endif

    
    //setup video output
    self.videoOutput = [[AVCaptureVideoDataOutput alloc] init];
    
    self.videoOutput.videoSettings = @{ (NSString *)kCVPixelBufferPixelFormatTypeKey: @(VIDEO_FORMAT) };
    
    self.videoOutput.alwaysDiscardsLateVideoFrames=YES;
    
    [self.videoOutput setSampleBufferDelegate:self queue:self.arQueue];
    
    [self.session addOutput:self.videoOutput];
    
    return YES;
}

- (void)initializeARSession {
    if (@available(iOS 11.0, *)) {
        _arSession = [ARSession new];
            
        self.arSession.delegateQueue = self.arQueue;
        self.arSession.delegate = self;
    }
}

- (void)startARSession {
    if (@available(iOS 11.0, *)) {
        ARWorldTrackingConfiguration *config = [ARWorldTrackingConfiguration new];
        config.planeDetection = ARPlaneDetectionHorizontal;
        config.lightEstimationEnabled = YES;
        
        [self.arSession runWithConfiguration:config];
    }
}

- (void)stopARSession {
    if (@available(iOS 11.0, *)) {
        [self.arSession pause];
    }
}

- (BOOL) startRunning {
    
    if (self.isRunning) return NO;
    
    _isRunning = YES;
    
    self.frameIndex = 0;
    
    if (hasARKit()) {
        [self startARSession];
    } else {
        [self.session startRunning];
    }

    return YES;
}

-(BOOL) stopRunning {
    if (!self.isRunning) return NO;
    _isRunning = NO;

    if (hasARKit()) {
        [self stopARSession];
    } else {
        [self.session stopRunning];
    }
    
    return YES;
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput
didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
       fromConnection:(AVCaptureConnection *)connection {
    CVPixelBufferRef cameraFrame = CMSampleBufferGetImageBuffer(sampleBuffer);
    
    [self sendFrame:cameraFrame];
}

- (void) sendFrame:(CVPixelBufferRef)pixelBuffer; {
    if (self.isRunning && !self.frameBusy && pixelBuffer) {
        self.frameBusy = true;
        auto res = CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
        if (res == kCVReturnSuccess && pixelBuffer) {
            [self.delegate sendFrame:pixelBuffer];
        }
        CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
        self.frameBusy = false;
    }
}

- (BOOL)lockForConfiguration {
    @synchronized (self) {
        if (!self.isLockedForConfiguration) {
            self.isLockedForConfiguration = [self.inputCamera lockForConfiguration:nil];
            return self.isLockedForConfiguration;
        }
        return NO;
    }
}

- (BOOL)unlockForConfiguration {
    @synchronized (self) {
        if (self.isLockedForConfiguration) {
            [self.inputCamera unlockForConfiguration];
            self.isLockedForConfiguration = NO;
            return YES;
        }
        return NO;
    }
}

- (BOOL)exposeAtPoint:(CGPoint)point
      useAutoExposure:(BOOL)useAutoExposure {
    @synchronized (self) {
        BOOL success = NO;
        //do not adjust exposure if busy
        
        
        BOOL lockedByMe = [self lockForConfiguration];
        
        [self.inputCamera setExposurePointOfInterest:point];
        //NSLog(@"Set exposure at point %f,%f", point.x, point.y);
        
        if (useAutoExposure) {
            success = [self setExposureMode:AVCaptureExposureModeAutoExpose];
        } else if (self.inputCamera.exposureMode != AVCaptureExposureModeContinuousAutoExposure){
            success = [self setExposureMode:AVCaptureExposureModeContinuousAutoExposure];
        }
        
        if (lockedByMe) {
            [self unlockForConfiguration];
        }
        
        return success;
    }
}

- (BOOL)setExposureMode:(AVCaptureExposureMode)exposureMode {
    @synchronized (self) {
        
        if (exposureMode == AVCaptureExposureModeAutoExpose
            && ![self.inputCamera isExposureModeSupported:AVCaptureExposureModeAutoExpose]) {
            //switch to continuous if auto is not supported
            exposureMode = AVCaptureExposureModeContinuousAutoExposure;
        }
        
        BOOL success = YES;
        
        BOOL lockedByMe = [self lockForConfiguration];
        
        if ([self.inputCamera isExposureModeSupported:exposureMode]) {
            [self.inputCamera setExposureMode:exposureMode];
        } else {
            success = NO;
        }
        
        if (lockedByMe) {
            [self unlockForConfiguration];
        }
        
        return success;
    }
}

- (void)startVideoPreview {
    if (!self.isPaused) return;
    @synchronized(self) {
        [self resume];
    }
}

- (void)stopVideoPreview {
    if (self.isPaused) return;
    @synchronized(self) {
        [self pause];
    }
}

- (void) pause {
    [self stopRunning];
    self.isPaused = YES;
}

- (void) resume {
    [self startRunning];
    self.isPaused = NO;
}

- (void)session:(ARSession *)session didUpdateFrame:(ARFrame *)frame API_AVAILABLE(ios(11.0)) {
    [self.delegate session:session didUpdateFrame:frame];
    [self sendFrame:frame.capturedImage];
}

- (void)session:(ARSession *)session didAddAnchors:(NSArray<ARAnchor*>*)anchors API_AVAILABLE(ios(11.0)) {
    [self.delegate session:session didAddAnchors:anchors];
}

- (void)session:(ARSession *)session didUpdateAnchors:(NSArray<ARAnchor*>*)anchors API_AVAILABLE(ios(11.0)) {
    [self.delegate session:session didUpdateAnchors: anchors];
}

- (void)session:(ARSession *)session didRemoveAnchors:(NSArray<ARAnchor*>*)anchors API_AVAILABLE(ios(11.0)) {
    [self.delegate session:session didRemoveAnchors: anchors];
}

@end
