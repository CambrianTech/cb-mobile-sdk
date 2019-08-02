#import <CoreVideo/CoreVideo.h>
#import "CambrianARNative.h"
#include "stdlib.h"
#include "UnityAppController.h"
#include <string>
#include <map>
#include <mutex>
#import <Availability.h>

#define SEND_POINTS_BACK 0

#if TARGET_IPHONE_SIMULATOR || defined(__x86_64__)
#   define IOS_SIMULATOR 1
#else
#   define IOS_DEVICE 1
#endif

#define DEFAULT_SCENE "CambrianARPlugin/Scenes/CambrianScene"

typedef void* MonoArray;

enum { kMonoArrayOffset = 16 };
template<class T>
T GetMonoArrayElement(MonoArray* array, int i) {
    char* raw = kMonoArrayOffset + i * sizeof (T) + (char*)array;
    return *(T*)raw;
}

int mono_array_length(MonoArray* array) {
    char* raw = 12 + (char*)array;
    return *reinterpret_cast<uint32_t*> (raw);
}

__weak CambrianARSession * m_sharedSession;

@interface CambrianARSession()

@property (assign, nonatomic) CBMatrix4x4 displayMatrix;

@property (assign, nonatomic) CBPointCloudData pointCloud;
@property (assign, nonatomic) bool sendingPointCloud;

@property (assign, nonatomic) CBLineData lineData;
@property (assign, nonatomic) bool sendingLineData;

@property (strong, nonatomic) ARSession *session;

- (id <MTLTexture>)generateExternalTexture:(int)width height:(int)height isColor:(BOOL)isColor;
- (void)setMTLTextureRegion:(id <MTLTexture>)texture
                      bytes:(const void *)bytes width:(int)width height:(int)height bytesPerRow:(int)bytesPerRow;


@end

class CBSurfaceInfo {
public:
    CBSurfaceAssetParams assetInfo;
    
    CBSurfaceInfo(CBSurfaceAssetParams params) : assetInfo(params) {

    }
    
    ~CBSurfaceInfo() {
        if (_maskPixelBytes) delete[] _maskPixelBytes;
        if (_shadowPixelBytes) delete[] _shadowPixelBytes;
    }
    
    void getMaskPixelData(uint8_t **bytes, unsigned int *length) {
        std::lock_guard<std::mutex> lock(_maskPixelBytesMutex);
        *bytes = _maskPixelBytes;
        *length = _maskPixelBytesLength;
    }
    
    void setMaskPixelData(const void *bytes, unsigned int length) {
        std::lock_guard<std::mutex> lock(_maskPixelBytesMutex);
        if (_maskPixelBytes) {
            delete[] _maskPixelBytes;
        }
        _maskPixelBytesLength = length;
        _maskPixelBytes = new uint8_t[_maskPixelBytesLength];
        memcpy(_maskPixelBytes, bytes, _maskPixelBytesLength);
    }
    
    void getShadowPixelData(uint8_t **bytes, unsigned int *length) {
        std::lock_guard<std::mutex> lock(_shadowPixelBytesMutex);
        *length = _shadowPixelBytesLength;
        *bytes = _shadowPixelBytes;
    }
    
    void setShadowPixelData(const void *bytes, unsigned int length) {
        std::lock_guard<std::mutex> lock(_shadowPixelBytesMutex);
        if (_shadowPixelBytes) {
            delete[] _shadowPixelBytes;
        }
        _shadowPixelBytesLength = length;
        _shadowPixelBytes = new uint8_t[_shadowPixelBytesLength];
        memcpy(_shadowPixelBytes, bytes, _shadowPixelBytesLength);
    }
    
private:
    uint8_t* _maskPixelBytes = 0;
    std::mutex _maskPixelBytesMutex;
    unsigned int _maskPixelBytesLength = 0;
    
    uint8_t* _shadowPixelBytes = 0;
    std::mutex _shadowPixelBytesMutex;
    unsigned int _shadowPixelBytesLength = 0;
    
    CBMatrix4x4 _matrixResult;
};

@implementation CambrianARSession {
    CAMBRIAN_SESSION_CALLBACK _sessionStartedCallback;
    CAMBRIAN_SESSION_CALLBACK _sessionStoppedCallback;
    CAMBRIAN_SESSION_CALLBACK _sceneChangingCallback;
    CAMBRIAN_FRAME_READY_CALLBACK _frameReadyCallback;
    CAMBRIAN_IMAGE_READY_CALLBACK _imageReadyCallback;
    CAMBRIAN_CAPTURE_CALLBACK _captureCallback;
    
    CAMBRIAN_ORIENTATION_CALLBACK _orientationNormalFoundCallback;
    CAMBRIAN_PLANE_ANCHOR_CALLBACK _planeAnchorAddedCallback;
    CAMBRIAN_PLANE_ANCHOR_CALLBACK _planeAnchorUpdatedCallback;
    CAMBRIAN_PLANE_ANCHOR_CALLBACK _planeAnchorRemovedCallback;
    
    CAMBRIAN_USER_ANCHOR_CALLBACK _userAnchorAddedCallback;
    CAMBRIAN_USER_ANCHOR_CALLBACK _userAnchorUpdatedCallback;
    CAMBRIAN_USER_ANCHOR_CALLBACK _userAnchorRemovedCallback;
    
    CAMBRIAN_ASSET_CALLBACK _assetAddedCallback;
    CAMBRIAN_ASSET_CALLBACK _assetRemovedCallback;
    CAMBRIAN_ASSET_CALLBACK _assetSelectedCallback;
    
    CAMBRIAN_SURFACE_CREATED_CALLBACK _surfaceCreatedCallback;
    CAMBRIAN_SURFACE_REMOVED_CALLBACK _surfaceRemovedCallback;
    CAMBRIAN_SURFACE_DATA_CALLBACK _surfaceDataCallback;
    
    CAMBRIAN_PAINT_ASSET_UPDATED_CALLBACK _paintAssetUpdatedCallback;
    CAMBRIAN_FLOOR_ASSET_UPDATED_CALLBACK _floorAssetUpdatedCallback;
    CAMBRIAN_MODEL_ASSET_UPDATED_CALLBACK _modelAssetUpdatedCallback;
    
    CAMBRIAN_DRAWING_UPDATED_CALLBACK _drawingUpdatedCallback;
    
    CAMBRIAN_SCREENSHOT_CALLBACK _screenshotCallback;
    CAMBRIAN_COMMAND_CALLBACK _commandCallback;
    
    NSMutableArray *_scheduledCommands;
    
    NSObject *m_surfaceTexturesMutex;
    std::vector<std::shared_ptr<CBSurfaceInfo>> m_surfaceTextures;
    
    int _shouldStart;
    
    id <MTLDevice> _device;
    CVMetalTextureCacheRef _textureCache;
    
    id <MTLTexture> _drawingTexture;
    id <MTLTexture> _textureY;
    id <MTLTexture> _textureCbCr;
    id <MTLTexture> _textureMask;
    bool m_isRunning;
    
    std::vector<CBHitTestResult> _hitTestResults;
    
    CGAffineTransform _displayTransform;
    
    dispatch_queue_t _frameQueue;
    bool _frameQueueBusy;
    size_t _imageWidth;
    size_t _imageHeight;
    
    std::string _sceneName;
    
    CBSurfaceData _surfaceData;
}

- (void)dealloc
{
    NSLog(@"Deallocating %@", NSStringFromClass([self class]));
}

- (id)init
{
    if (self = [super init])
    {
        m_sharedSession = self;
        _scheduledCommands = [NSMutableArray new];
        
        m_surfaceTexturesMutex = [NSObject new];
        
        dispatch_queue_attr_t highPriorityAttr = dispatch_queue_attr_make_with_qos_class (DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INITIATED,-1);
        _frameQueue = dispatch_queue_create ("com.cambrian.frameQueue", highPriorityAttr);
    }
    return self;
}

- (void)setupMetal
{
    _device = MTLCreateSystemDefaultDevice();
    CVMetalTextureCacheCreate(NULL, NULL, _device, NULL, &_textureCache);
    NSLog(@"Created Metal Device");
}

- (void)teardownMetal
{
    if (_textureCache) {
        CFRelease(_textureCache);
        _textureCache = 0;
    }
}

- (void)setSceneName:(NSString *)scene {
    self.displaysVideo = (scene == nil);
    if (scene) {
        _sceneName = [scene UTF8String];
    } else {
        _sceneName = DEFAULT_SCENE;
    }
}

#define CB_CALLBACK(name, ...) \
    if (name) { \
        name(__VA_ARGS__); \
    } else { \
        __weak CambrianARSession* weakSelf = self; \
        [self scheduleCommand:^{ \
            __strong CambrianARSession* strongSelf = weakSelf; if (!strongSelf) return; \
            if (strongSelf->name) { \
                strongSelf->name(__VA_ARGS__); \
            } \
        }]; \
    }

- (void)scheduleCommand:(dispatch_block_t)command {
    [_scheduledCommands addObject:command];
}

- (void)runScheduledCommands {
    @synchronized(_scheduledCommands) {
        for (id command in _scheduledCommands) {
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0), command);
        }
        [_scheduledCommands removeAllObjects];
    }
}

- (void)clearScheduledCommands {
    @synchronized(_scheduledCommands) {
        [_scheduledCommands removeAllObjects];
    }
}

- (void)startRunning:(CBSessionState)state {
    [self setupMetal];
    [self startARKit];
    
    if (state.isVideo) {
        
    }
    
    if (_sessionStartedCallback) {
        _sessionStartedCallback(state);
    } else {
        _shouldStart = state.isVideo ? 1 : 2;
    }
    m_isRunning = true;
}

- (void)stopRunning:(CBSessionState)state {
    
    m_isRunning = false;
    
    CB_CALLBACK(_sessionStoppedCallback, state);
    
    [self clearAllSurfaces];
    
    [self.session pause];
    [self teardownMetal];
}

- (void)captureToStill {
    CB_CALLBACK(_captureCallback);
}

- (void)sendUnityCommand:(CBCommandData)command {
    //printf("Sending unity command %s(%s)\n", command.command, command.json);
    CB_CALLBACK(_commandCallback, command);
}

- (void)sceneChanging:(CBSessionState)state {
    CB_CALLBACK(_sceneChangingCallback, state);
}

- (id <MTLTexture>)generateExternalTexture:(int)width
                                    height:(int)height
                                   isColor:(BOOL)isColor {
    
#if IOS_DEVICE
    MTLPixelFormat format = isColor ? MTLPixelFormatBGRA8Unorm : MTLPixelFormatR8Unorm;
    MTLTextureDescriptor *txDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:format
                                                                                      width:width
                                                                                     height:height
                                                                                  mipmapped:NO];
    if (!_device || !txDesc) {
        return NULL;
    }
    
    //txDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    
    id <MTLTexture> texture = [_device newTextureWithDescriptor:txDesc];
    
    if (texture) {
        printf("generateExternalTexture: Generated texture at %dx%d\n", width, height);
        return texture;
    } else {
        printf("generateExternalTexture: ERROR GENERATING TEXTURE!\n");
    }

#endif
    
    return NULL;
}

- (void)clearAllSurfaces {
    
    @synchronized(m_surfaceTexturesMutex) {
        @autoreleasepool {
            for (const auto &surface : m_surfaceTextures) {
                [self surfaceAssetRemoved:surface->assetInfo];
            }
            m_surfaceTextures.clear();
        }
    }
}

- (void)orientationNormalFound:(CBVector3)normal {
    CB_CALLBACK(_orientationNormalFoundCallback, normal);
}

- (void)surfaceDataUpdated:(const CBSurfaceData&)data {
    
    @synchronized(m_surfaceTexturesMutex) {
        if (data.index >= m_surfaceTextures.size()) {
            return;
        }
        
        auto &surface = m_surfaceTextures[data.index];
        CBSurfaceDataExtents extents;
        
        _surfaceData = data;
        
        extents.center = _surfaceData.center;
        extents.normal = _surfaceData.normal;
        extents.rotation2D = _surfaceData.rotation2D;
        
        extents.meshExtents3D = _surfaceData.meshExtents3D;
        extents.maskExtents3D = _surfaceData.maskExtents3D;
        extents.shadowsExtents3D = _surfaceData.shadowsExtents3D;
        
        surface->setMaskPixelData(_surfaceData.maskImageData.image, _surfaceData.maskImageData.width * _surfaceData.maskImageData.height);
        surface->setShadowPixelData(_surfaceData.shadowsImageData.image, _surfaceData.shadowsImageData.width * _surfaceData.shadowsImageData.height);
        
        CB_CALLBACK(_surfaceDataCallback, surface->assetInfo, extents);
    }
}


- (void)surfaceAssetCreated:(const CBSurfaceAssetParams&)params {
    m_surfaceTextures.push_back(std::shared_ptr<CBSurfaceInfo>(new CBSurfaceInfo(params)));
    CB_CALLBACK(_surfaceCreatedCallback, m_surfaceTextures[m_surfaceTextures.size()-1]->assetInfo);
}

- (void)surfaceAssetRemoved:(const CBSurfaceAssetParams&)params {
    CB_CALLBACK(_surfaceRemovedCallback, params);
}

- (void)assetAdded:(CBAssetParams)params {
    CB_CALLBACK(_assetAddedCallback, params);
}

- (void)assetRemoved:(CBAssetParams)params {
    CB_CALLBACK(_assetRemovedCallback, params);
}

- (void)assetSelected:(CBAssetParams)params {
    CB_CALLBACK(_assetSelectedCallback, params);
}

- (void)paintAssetUpdated:(CBPaintAssetParams)params {
    CB_CALLBACK(_paintAssetUpdatedCallback, params);
}

- (void)floorAssetUpdated:(CBFloorAssetParams)params {
    CB_CALLBACK(_floorAssetUpdatedCallback, params);
}

- (void)modelAssetUpdated:(CBModelAssetParams)params {
    CB_CALLBACK(_modelAssetUpdatedCallback, params);
}

- (void)drawingUpdated:(const void *)bytes width:(int)width height:(int)height color:(float*)color;
{
    
#if IOS_DEVICE
    if (!_drawingTexture || width != _drawingTexture.width) return;
    
    [self setMTLTextureRegion:_drawingTexture bytes:bytes width:width height:height bytesPerRow:width];
    
    CBDrawingParams drawing;
    
    drawing.colorRed = color[0];
    drawing.colorGreen = color[1];
    drawing.colorBlue = color[2];
    drawing.colorAlpha = color[3];
    
    CB_CALLBACK(_drawingUpdatedCallback, drawing);
#endif
    
}

- (void)setMTLTextureRegion:(id <MTLTexture>)texture bytes:(const void *)bytes width:(int)width height:(int)height bytesPerRow:(int)bytesPerRow {
    
    auto region = MTLRegionMake2D(0, 0, width, height);
    
    [texture replaceRegion:region
               mipmapLevel:0
                 withBytes:bytes
               bytesPerRow:bytesPerRow];
}

- (void)setMainCameraImage:(const void *)bytes width:(int)width height:(int)height {
    //TODO: callback similar to update shadows and mask
}

- (void)saveScreenshot:(CBScreenshotData)screenshot {
    CB_CALLBACK(_screenshotCallback, screenshot);
}

- (std::vector<CBHitTestResult>) hitTestAtPoint:(CBPoint)point resultType:(CBHitTestResultType)resultType {
    std::vector<CBHitTestResult> results;
    
    CGPoint cgpoint = CGPointApplyAffineTransform(CGPointMake(point.x, point.y), _displayTransform);
    
    NSArray<ARHitTestResult *> *arResults = [self.session.currentFrame hitTest:cgpoint types:(ARHitTestResultType)resultType];
    
    for (ARHitTestResult *hitResult : arResults) {

        CBHitTestResult result;
        
        result.type = (CBHitTestResultType) hitResult.type;
        result.distance = hitResult.distance;
        
        ARKitMatrixToCBMatrix4x4(swapHandedness(hitResult.localTransform), &result.localTransform);
        ARKitMatrixToCBMatrix4x4(swapHandedness(hitResult.worldTransform), &result.worldTransform);

//        if (hitResult.anchor) {
//            result.anchorIdentifierStr = [hitResult.anchor.identifier.UUIDString UTF8String];
//        }
//        result.isValid = true;
        results.push_back(result);
    }
    
    return results;
}

inline void CBLightDataFromARFrame(CBLightData& lightData, ARFrame *arFrame) {
    if (arFrame.lightEstimate != NULL) {
        lightData.lightingType = LightEstimate;
        lightData.lightEstimate.ambientIntensity = arFrame.lightEstimate.ambientIntensity;
        lightData.lightEstimate.ambientColorTemperature = arFrame.lightEstimate.ambientColorTemperature;
    }
}

- (void) startARKit
{
    ARWorldTrackingConfiguration* config = [ARWorldTrackingConfiguration new];
    config.planeDetection = ARPlaneDetectionHorizontal | ARPlaneDetectionVertical;
    ARSessionRunOptions runOpts = ARSessionRunOptionResetTracking | ARSessionRunOptionRemoveExistingAnchors;

    if (!self.session) {
        self.session = [ARSession new];
        self.session.delegate = self;
    }
    [self.session runWithConfiguration:config options:runOpts];
}

- (CBUserAnchor) addUserAnchorWithTransform:(const CBMatrix4x4 &)transform {
    
    matrix_float4x4 trans;
    
    CBMatrix4x4ToARKitMatrix(transform, &trans);
    
    ARAnchor *arAnchor = [[ARAnchor alloc] initWithTransform:swapHandedness(trans)];

    [self.session addAnchor:arAnchor];
    
    CBUserAnchor cbAnchor;
    ARKitAnchorToCBUserAnchor(arAnchor, cbAnchor);
    return cbAnchor;
}

inline void ARKitMatrixToCBMatrix4x4(const matrix_float4x4& matrixIn, CBMatrix4x4* matrixOut)
{
    vector_float4 c0 = matrixIn.columns[0];
    matrixOut->column0.x = c0.x;
    matrixOut->column0.y = c0.y;
    matrixOut->column0.z = c0.z;
    matrixOut->column0.w = c0.w;
    
    vector_float4 c1 = matrixIn.columns[1];
    matrixOut->column1.x = c1.x;
    matrixOut->column1.y = c1.y;
    matrixOut->column1.z = c1.z;
    matrixOut->column1.w = c1.w;
    
    vector_float4 c2 = matrixIn.columns[2];
    matrixOut->column2.x = c2.x;
    matrixOut->column2.y = c2.y;
    matrixOut->column2.z = c2.z;
    matrixOut->column2.w = c2.w;
    
    vector_float4 c3 = matrixIn.columns[3];
    matrixOut->column3.x = c3.x;
    matrixOut->column3.y = c3.y;
    matrixOut->column3.z = c3.z;
    matrixOut->column3.w = c3.w;
}

inline void CBMatrix4x4ToARKitMatrix(const CBMatrix4x4& matrixIn, matrix_float4x4* matrixOut)
{
    matrixOut->columns[0].x = matrixIn.column0.x;
    matrixOut->columns[0].y = matrixIn.column0.y;
    matrixOut->columns[0].z = matrixIn.column0.z;
    matrixOut->columns[0].w = matrixIn.column0.w;
    
    matrixOut->columns[1].x = matrixIn.column1.x;
    matrixOut->columns[1].y = matrixIn.column1.y;
    matrixOut->columns[1].z = matrixIn.column1.z;
    matrixOut->columns[1].w = matrixIn.column1.w;
    
    matrixOut->columns[2].x = matrixIn.column2.x;
    matrixOut->columns[2].y = matrixIn.column2.y;
    matrixOut->columns[2].z = matrixIn.column2.z;
    matrixOut->columns[2].w = matrixIn.column2.w;
    
    matrixOut->columns[3].x = matrixIn.column3.x;
    matrixOut->columns[3].y = matrixIn.column3.y;
    matrixOut->columns[3].z = matrixIn.column3.z;
    matrixOut->columns[3].w = matrixIn.column3.w;
    
}

inline void CBMatrix4x4FromCGAffineTransform(CBMatrix4x4& outMatrix, CGAffineTransform displayTransform, BOOL isLandscape)
{
    if (isLandscape)
    {
        outMatrix.column0.x = displayTransform.a;
        outMatrix.column0.y = displayTransform.c;
        outMatrix.column0.z = displayTransform.tx;
        outMatrix.column1.x = displayTransform.b;
        outMatrix.column1.y = -displayTransform.d;
        outMatrix.column1.z = 1.0f - displayTransform.ty;
        outMatrix.column2.z = 1.0f;
        outMatrix.column3.w = 1.0f;
    }
    else
    {
        outMatrix.column0.x = displayTransform.a;
        outMatrix.column0.y = -displayTransform.c;
        outMatrix.column0.z = -displayTransform.tx;
        outMatrix.column1.x = displayTransform.b;
        outMatrix.column1.y = displayTransform.d;
        outMatrix.column1.z = displayTransform.ty;
        outMatrix.column2.z = 1.0f;
        outMatrix.column3.w = 1.0f;
    }
}

inline matrix_float4x4 swapHandedness(const matrix_float4x4 &input) {
    
    matrix_float3x3 rotation;
    
    rotation.columns[0] = {input.columns[0][0], input.columns[0][1], input.columns[0][2]};
    rotation.columns[1] = {input.columns[1][0], input.columns[1][1], input.columns[1][2]};
    rotation.columns[2] = {input.columns[2][0], input.columns[2][1], input.columns[2][2]};
    
    simd_quatf quat = simd_quaternion(rotation);

    quat = simd_quaternion(quat.vector[0], quat.vector[1], -quat.vector[2], -quat.vector[3]);
    
    rotation = simd_matrix3x3(quat);
    
    matrix_float4x4 result;
    
    result.columns[0] = {rotation.columns[0][0], rotation.columns[0][1], rotation.columns[0][2], input.columns[0][3]};
    result.columns[1] = {rotation.columns[1][0], rotation.columns[1][1], rotation.columns[1][2], input.columns[0][3]};
    result.columns[2] = {rotation.columns[2][0], rotation.columns[2][1], rotation.columns[2][2], input.columns[0][3]};
    result.columns[3] = {input.columns[3][0], input.columns[3][1], -input.columns[3][2], input.columns[3][3]};
    
    return result;
}

/**
 This is called when a new frame has been updated.
 
 @param session The session being run.
 @param frame The frame that has been updated.
 */
- (void)session:(ARSession *)session didUpdateFrame:(ARFrame *)frame {
    
    if (!m_isRunning) return;
    
    CBCameraFrame cbFrame;
    CVPixelBufferRef pixelBuffer = frame.capturedImage;
    
    _imageWidth = CVPixelBufferGetWidth(pixelBuffer);
    _imageHeight = CVPixelBufferGetHeight(pixelBuffer);
    
    cbFrame.videoParams.yWidth = (uint32_t)_imageWidth;
    cbFrame.videoParams.yHeight = (uint32_t)_imageHeight;

    // rotation  matrix
    // [ cos    -sin]
    // [ sin     cos]
    matrix_float4x4 rotatedMatrix = matrix_identity_float4x4;
    UIInterfaceOrientation orient = [[UIApplication sharedApplication] statusBarOrientation];
    switch (orient) {
        case UIInterfaceOrientationPortrait:
            rotatedMatrix.columns[0][0] = 0;
            rotatedMatrix.columns[0][1] = 1;
            rotatedMatrix.columns[1][0] = -1;
            rotatedMatrix.columns[1][1] = 0;
            cbFrame.videoParams.screenOrientation = 1;
            break;
        case UIInterfaceOrientationLandscapeLeft:
            rotatedMatrix.columns[0][0] = -1;
            rotatedMatrix.columns[0][1] = 0;
            rotatedMatrix.columns[1][0] = 0;
            rotatedMatrix.columns[1][1] = -1;
            cbFrame.videoParams.screenOrientation = 4;
            break;
        case UIInterfaceOrientationPortraitUpsideDown:
            rotatedMatrix.columns[0][0] = 0;
            rotatedMatrix.columns[0][1] = -1;
            rotatedMatrix.columns[1][0] = 1;
            rotatedMatrix.columns[1][1] = 0;
            cbFrame.videoParams.screenOrientation = 2;
            break;
        default:
            cbFrame.videoParams.screenOrientation = 3;
            break;
    }
    
    //arkit stuff
    {
        CGRect nativeBounds = [[UIScreen mainScreen] nativeBounds];
        CGSize nativeSize = GetAppController().rootView.bounds.size;
        
        _displayTransform = CGAffineTransformInvert([frame displayTransformForOrientation:orient viewportSize:nativeSize]);
        
        CBMatrix4x4FromCGAffineTransform(cbFrame.displayTransform, _displayTransform, UIInterfaceOrientationIsLandscape(orient));
        
        float unityCameraNearZ = .01;
        float unityCameraFarZ = 30;
        matrix_float4x4 projectionMatrix = [session.currentFrame.camera projectionMatrixForOrientation:orient viewportSize:nativeSize
                                                                                                 zNear:(CGFloat)unityCameraNearZ zFar:(CGFloat)unityCameraFarZ];
        
        ARKitMatrixToCBMatrix4x4(projectionMatrix, &cbFrame.projectionMatrix);
        
        matrix_float4x4 matrix = matrix_multiply(frame.camera.transform, rotatedMatrix);
        
        ARKitMatrixToCBMatrix4x4(swapHandedness(matrix), &cbFrame.cameraTransform);
    }

    //pixel buffers
    if (self.delegate.needsFrame) {
        //get out y an u bytes:
        CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
        
        //intensity bytes
        {
            cbFrame.videoParams.yStride = CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, 0);
            unsigned long numBytes = cbFrame.videoParams.yStride * CVPixelBufferGetHeightOfPlane(pixelBuffer,0);
            void* baseAddress = CVPixelBufferGetBaseAddressOfPlane(pixelBuffer,0);
            cbFrame.videoParams.pYPixelBytes = new char[numBytes];
            memcpy(cbFrame.videoParams.pYPixelBytes, baseAddress, numBytes);
        }
        
        //color bytes
        {
            unsigned long stride = CVPixelBufferGetBytesPerRowOfPlane(pixelBuffer, 1);
            unsigned long numBytes = stride * CVPixelBufferGetHeightOfPlane(pixelBuffer,1);
            void* baseAddress = CVPixelBufferGetBaseAddressOfPlane(pixelBuffer,1);
            cbFrame.videoParams.pUVPixelBytes = new char[numBytes];
            memcpy(cbFrame.videoParams.pUVPixelBytes, baseAddress, numBytes);
            
//            for (int i=0; i<10; i++) {
//                printf("pixel: %d,", uint(cbFrame.videoParams.pUVPixelBytes[i*10]));
//            }
        }
        
        CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
        
        //send here
        [self.delegate arFrameCallback:cbFrame];
        
        if (cbFrame.videoParams.pYPixelBytes) delete[] cbFrame.videoParams.pYPixelBytes;
        if (cbFrame.videoParams.pUVPixelBytes) delete[] cbFrame.videoParams.pUVPixelBytes;
    }
    
    if (_frameReadyCallback == nil || _frameQueueBusy) {
        return;
    }

    if (self.displaysVideo) {
        CBLightDataFromARFrame(cbFrame.lightData, frame);
        
        id<MTLTexture> textureY = nil;
        id<MTLTexture> textureCbCr = nil;
        
        // textureY
        {
            const size_t width = CVPixelBufferGetWidthOfPlane(pixelBuffer, 0);
            const size_t height = CVPixelBufferGetHeightOfPlane(pixelBuffer, 0);
            MTLPixelFormat pixelFormat = MTLPixelFormatR8Unorm;
            
            
            CVMetalTextureRef texture = NULL;
            CVReturn status = CVMetalTextureCacheCreateTextureFromImage(NULL, _textureCache, pixelBuffer, NULL, pixelFormat, width, height, 0, &texture);
            if(status == kCVReturnSuccess)
            {
                textureY = CVMetalTextureGetTexture(texture);
                CFRelease(texture);
            }
        }
        
        // textureCbCr
        {
            const size_t width = CVPixelBufferGetWidthOfPlane(pixelBuffer, 1);
            const size_t height = CVPixelBufferGetHeightOfPlane(pixelBuffer, 1);
            MTLPixelFormat pixelFormat = MTLPixelFormatRG8Unorm;
            
            CVMetalTextureRef texture = NULL;
            CVReturn status = CVMetalTextureCacheCreateTextureFromImage(NULL, _textureCache, pixelBuffer, NULL, pixelFormat, width, height, 1, &texture);
            if(status == kCVReturnSuccess)
            {
                textureCbCr = CVMetalTextureGetTexture(texture);
                CFRelease(texture);
            }
        }
        
        if (textureY != nil && textureCbCr != nil) {
            _textureY = textureY;
            _textureCbCr = textureCbCr;
        }
    }
    
    _frameQueueBusy = true;
    dispatch_sync(_frameQueue, ^{
        _frameReadyCallback(cbFrame);
        _frameQueueBusy = false;
    });
    
    if (frame.rawFeaturePoints.count) {
        CBPointCloudData pcd;
        pcd.points.resize(frame.rawFeaturePoints.count);
        pcd.identifiers.resize(frame.rawFeaturePoints.count);
        
        for (int i=0; i<frame.rawFeaturePoints.count; i++) {
            vector_float3 point = frame.rawFeaturePoints.points[i];
            pcd.points[i] = {point[0], point[1], -point[2]};
            uint64_t identifier = frame.rawFeaturePoints.identifiers[i];
            pcd.identifiers[i] = identifier;
        }
        
        [self.delegate pointCloudDataUpdated:pcd];
    }
    
    
}

inline void ARPlaneGeometryToCBPlaneGeometry(const ARPlaneGeometry *arPlaneGeometry, CBPlaneGeometry& planeGeometry) {
    planeGeometry.vertexCount = arPlaneGeometry.vertexCount;
    planeGeometry.triangleCount = arPlaneGeometry.triangleCount;
    planeGeometry.textureCoordinateCount = arPlaneGeometry.textureCoordinateCount;
    planeGeometry.boundaryVertexCount = arPlaneGeometry.boundaryVertexCount;
    planeGeometry.vertices = (float *) arPlaneGeometry.vertices;
    planeGeometry.triangleIndices = (int *) arPlaneGeometry.triangleIndices;
    planeGeometry.textureCoordinates = (float *) arPlaneGeometry.textureCoordinates;
    planeGeometry.boundaryVertices = (float *) arPlaneGeometry.boundaryVertices;
}

void ARKitAnchorToCBPlaneAnchor(const ARPlaneAnchor *arkitAnchor, CBPlaneAnchor &cbAnchor) {
    
    NSString *anchorID = [arkitAnchor.identifier UUIDString];
    
    cbAnchor.anchorIdentifier = (void *)[anchorID UTF8String];
    
    cbAnchor.alignment = arkitAnchor.alignment == ARPlaneAnchorAlignmentVertical ? CBPlaneAnchorAlignmentVertical : CBPlaneAnchorAlignmentHorizontal;
    ARKitMatrixToCBMatrix4x4(swapHandedness(arkitAnchor.transform), &cbAnchor.transform);
    cbAnchor.center = {arkitAnchor.center[0], arkitAnchor.center[1], -arkitAnchor.center[2]};
    cbAnchor.extent = {arkitAnchor.extent[0], arkitAnchor.extent[1], -arkitAnchor.extent[2]};
    
    ARPlaneGeometryToCBPlaneGeometry(arkitAnchor.geometry, cbAnchor.geometry);
}

void ARKitAnchorToCBUserAnchor(ARAnchor *anchor, CBUserAnchor &cbAnchor) {
    cbAnchor.anchorIdentifier = (__bridge void *)[anchor.identifier UUIDString];
    ARKitMatrixToCBMatrix4x4(swapHandedness(anchor.transform), &cbAnchor.transform);
}

/**
 This is called when new anchors are added to the session.
 
 @param session The session being run.
 @param anchors An array of added anchors.
 */
- (void)session:(ARSession *)session didAddAnchors:(NSArray<ARAnchor*>*)anchors {

    for (ARAnchor *anchor in anchors) {
        if ([anchor isKindOfClass:[ARPlaneAnchor class]]) {
            CBPlaneAnchor cbAnchor;
             ARKitAnchorToCBPlaneAnchor((ARPlaneAnchor*)anchor, cbAnchor);
            [self.delegate planeAnchorAdded:cbAnchor];
            
            CB_CALLBACK(_planeAnchorAddedCallback, cbAnchor);
        } else {
            CBUserAnchor cbAnchor;
            ARKitAnchorToCBUserAnchor(anchor, cbAnchor);
            [self.delegate userAnchorAdded:cbAnchor];
            
            CB_CALLBACK(_userAnchorAddedCallback, cbAnchor);
        }
    }
}

/**
 This is called when anchors are updated.
 
 @param session The session being run.
 @param anchors An array of updated anchors.
 */
- (void)session:(ARSession *)session didUpdateAnchors:(NSArray<ARAnchor*>*)anchors {

    for (ARAnchor *anchor in anchors) {
        if ([anchor isKindOfClass:[ARPlaneAnchor class]]) {
            CBPlaneAnchor cbAnchor;
            ARKitAnchorToCBPlaneAnchor((ARPlaneAnchor*)anchor, cbAnchor);
            [self.delegate planeAnchorUpdated:cbAnchor];
            
            CB_CALLBACK(_planeAnchorUpdatedCallback, cbAnchor);
        } else {
            CBUserAnchor cbAnchor;
            ARKitAnchorToCBUserAnchor(anchor, cbAnchor);
            [self.delegate userAnchorUpdated:cbAnchor];
            
            CB_CALLBACK(_userAnchorUpdatedCallback, cbAnchor);
        }
    }
}

/**
 This is called when anchors are removed from the session.
 
 @param session The session being run.
 @param anchors An array of removed anchors.
 */
- (void)session:(ARSession *)session didRemoveAnchors:(NSArray<ARAnchor*>*)anchors {

    for (ARAnchor *anchor in anchors) {
        if ([anchor isKindOfClass:[ARPlaneAnchor class]]) {
            CBPlaneAnchor cbAnchor;
            ARKitAnchorToCBPlaneAnchor((ARPlaneAnchor*)anchor, cbAnchor);
            [self.delegate planeAnchorRemoved:cbAnchor];
            
            CB_CALLBACK(_planeAnchorRemovedCallback, cbAnchor);
        } else {
            CBUserAnchor cbAnchor;
            ARKitAnchorToCBUserAnchor(anchor, cbAnchor);
            [self.delegate userAnchorRemoved:cbAnchor];
            
            CB_CALLBACK(_userAnchorRemovedCallback, cbAnchor);
        }
    }
}

extern "C" void* cambrian_getSceneName() {
    CambrianARSession *session = m_sharedSession;
    return (void*) session->_sceneName.c_str();
}

// Must match ARTextureHandles in UnityARSession.cs
extern "C" struct CBTextureHandles
{
    void* textureY;
    void* textureCbCr;
    void* textureMask;
};

extern "C" CBTextureHandles cambrian_GetVideoTextureHandles()
{
    CBTextureHandles handles;
    
    if (CambrianARSession *session = m_sharedSession) {
        handles.textureY = (__bridge_retained void*)session->_textureY;
        handles.textureCbCr = (__bridge_retained void*)session->_textureCbCr;
        handles.textureMask = (__bridge_retained void*)session->_textureMask;
    }

    return handles;
}

extern "C" void cambrian_StillRender() {
    CambrianARSession *session = m_sharedSession;
    
    [session.delegate stillRender];
}

extern "C" void cambrian_NotifyReady(bool isReady) {
    CambrianARSession *session = m_sharedSession; if (!session || !session->m_isRunning) return;
    
    if (isReady) {
        [session runScheduledCommands];
        [session.delegate notifyReady:true];
    } else {
        [session.delegate notifyReady:false];
    }
}

extern "C" void cambrian_ScreenshotSaved() {
    CambrianARSession *session = m_sharedSession; if (!session || !session->m_isRunning) return;
    
    [session.delegate screenshotSaved];
}

extern "C" void* cambrian_GetNativeARSession(CAMBRIAN_SESSION_CALLBACK sessionStarted,
                                             CAMBRIAN_SESSION_CALLBACK sessionStopped,
                                             CAMBRIAN_SESSION_CALLBACK sceneChanging,
                                             CAMBRIAN_FRAME_READY_CALLBACK frameReady,
                                             CAMBRIAN_IMAGE_READY_CALLBACK imageReady,
                                             CAMBRIAN_CAPTURE_CALLBACK capture,
                                             CAMBRIAN_SCREENSHOT_CALLBACK screenshot,
                                             CAMBRIAN_COMMAND_CALLBACK command)
{
    CambrianARSession *session = m_sharedSession; if (!session) return nullptr;
    
    session->_sessionStartedCallback = sessionStarted;
    session->_sessionStoppedCallback = sessionStopped;
    session->_sceneChangingCallback = sceneChanging;
    
    session->_frameReadyCallback = frameReady;
    session->_imageReadyCallback = imageReady;
    session->_captureCallback = capture;
    session->_screenshotCallback = screenshot;
    session->_commandCallback = command;
    
    return (__bridge_retained void*)session;
}

extern "C" void cambrian_TouchEvent(int touchPhase, float screenX, float screenY) {
    CambrianARSession *session = m_sharedSession; if (!session || !session->m_isRunning) return;
    
    [session.delegate touchEvent:(CBTouchPhase)touchPhase position:{screenX, screenY}];
}

extern "C" int cambrian_shouldStartMode() {
    CambrianARSession *session = m_sharedSession; if (!session) return 0;
    
    return session->_shouldStart;
}

extern "C" void cambrian_SetAnchorCallbacks(CAMBRIAN_ORIENTATION_CALLBACK normalFound,
                                            CAMBRIAN_PLANE_ANCHOR_CALLBACK anchorAdded,
                                            CAMBRIAN_PLANE_ANCHOR_CALLBACK anchorUpdated,
                                            CAMBRIAN_PLANE_ANCHOR_CALLBACK anchorRemoved,
                                            CAMBRIAN_USER_ANCHOR_CALLBACK userAnchorAdded,
                                            CAMBRIAN_USER_ANCHOR_CALLBACK userAnchorUpdated,
                                            CAMBRIAN_USER_ANCHOR_CALLBACK userAnchorRemoved) {
    CambrianARSession *session = m_sharedSession; if (!session) return;
    
    session->_orientationNormalFoundCallback = normalFound;
    session->_planeAnchorAddedCallback = anchorAdded;
    session->_planeAnchorUpdatedCallback = anchorUpdated;
    session->_planeAnchorRemovedCallback = anchorRemoved;
    
    session->_userAnchorAddedCallback = userAnchorAdded;
    session->_userAnchorUpdatedCallback = userAnchorUpdated;
    session->_userAnchorRemovedCallback = userAnchorRemoved;
}

extern "C" void cambrian_SetAssetCallbacks(CAMBRIAN_ASSET_CALLBACK assetAdded,
                                           CAMBRIAN_ASSET_CALLBACK assetRemoved,
                                           CAMBRIAN_ASSET_CALLBACK assetSelected) {
    CambrianARSession *session = m_sharedSession; if (!session) return;
    session->_assetAddedCallback = assetAdded;
    session->_assetRemovedCallback = assetRemoved;
    session->_assetSelectedCallback = assetSelected;
}

extern "C" void cambrian_SetSurfaceCallbacks(CAMBRIAN_SURFACE_CREATED_CALLBACK createdCallback,
                                             CAMBRIAN_SURFACE_REMOVED_CALLBACK removedCallback,
                                             CAMBRIAN_SURFACE_DATA_CALLBACK dataCallback) {
    CambrianARSession *session = m_sharedSession; if (!session) return;
    session->_surfaceCreatedCallback = createdCallback;
    session->_surfaceRemovedCallback = removedCallback;
    session->_surfaceDataCallback = dataCallback;
}

extern "C" void cambrian_ClearAllSurfaces() {
    CambrianARSession *session = m_sharedSession; if (!session) return;
    
    [session.delegate clearAll];
}

extern "C" void cambrian_SetPaintCallbacks(CAMBRIAN_PAINT_ASSET_UPDATED_CALLBACK paintAssetUpdated) {
    CambrianARSession *session = m_sharedSession; if (!session || !session->m_isRunning) return;
    
    session->_paintAssetUpdatedCallback = paintAssetUpdated;
}

extern "C" void cambrian_SetFloorCallbacks(CAMBRIAN_FLOOR_ASSET_UPDATED_CALLBACK floorAssetUpdated) {
    CambrianARSession *session = m_sharedSession; if (!session || !session->m_isRunning) return;
    
    session->_floorAssetUpdatedCallback = floorAssetUpdated;
}

extern "C" void cambrian_SetModelCallbacks(CAMBRIAN_MODEL_ASSET_UPDATED_CALLBACK modelAssetUpdated) {
    CambrianARSession *session = m_sharedSession; if (!session || !session->m_isRunning) return;
    
    session->_modelAssetUpdatedCallback = modelAssetUpdated;
}

extern "C" void cambrian_SetDrawingCallbacks(CAMBRIAN_DRAWING_UPDATED_CALLBACK drawingCallback) {
    CambrianARSession *session = m_sharedSession; if (!session || !session->m_isRunning) return;
    
    session->_drawingUpdatedCallback = drawingCallback;
}

extern "C" void* cambrian_GenerateDrawingTexture(int width, int height) {
    CambrianARSession *session = m_sharedSession; if (!session || !session->m_isRunning) return nullptr;
    
    session->_drawingTexture = [session generateExternalTexture:width height:height isColor:NO];
    
    return (__bridge_retained void*) session->_drawingTexture;
}

extern "C" CBMatrix4x4 cambrian_Get2DAffineTransform(void *pointsBefore0, void *pointsAfter0) {
    
    CambrianARSession *session = m_sharedSession; if (!session) return CBMatrix4x4();
    
    CBVector3 *_pointsBefore0 = (CBVector3 *)pointsBefore0;
    CBVector3 pointsBefore[4];
    for (int i=0; i<4; i++) {
        pointsBefore[i] = _pointsBefore0[i];
    }
    
    CBVector3 *_pointsAfter0 = (CBVector3 *)pointsAfter0;
    CBVector3 pointsAfter[4];
    for (int i=0; i<4; i++) {
        pointsAfter[i] = _pointsAfter0[i];
    }
    
    return [session.delegate get2DAffineTransform:pointsBefore after:pointsAfter];
}

extern "C" bool cambrian_GetPointCloudData(float** verts, uint64_t** identifiers, unsigned int* vertLength) {
    CambrianARSession *session = m_sharedSession; if (!session) return false;
    
    if (session.sendingPointCloud) {
        *verts = reinterpret_cast<float*>(session.pointCloud.points.data());
        *identifiers = reinterpret_cast<uint64_t *>(session.pointCloud.identifiers.data());
        *vertLength = (unsigned int)session.pointCloud.points.size() * 3;
        
        session.sendingPointCloud = NO;
        return true;
    }
    
    return false;
}

extern "C" bool cambrian_GetMeshData(int index, float** verts3D, unsigned int* vertLength, int** indices3D, unsigned int* indicesLength) {
    CambrianARSession *session = m_sharedSession; if (!session || !session->m_isRunning) return false;
    
    index = fmin(session->_surfaceData.index, index);
    
    *vertLength = (unsigned int)session->_surfaceData.mesh.vertices.size() * 3;
    auto vSize = (*vertLength)*sizeof(float);
    *verts3D = static_cast<float*>(malloc(vSize));
    memcpy(*verts3D, session->_surfaceData.mesh.vertices.data(), vSize);
    
    *indicesLength = (unsigned int)session->_surfaceData.mesh.indices.size();
    auto iSize = (*indicesLength)*sizeof(int);
    *indices3D = static_cast<int*>(malloc(iSize));
    memcpy(*indices3D, session->_surfaceData.mesh.indices.data(), iSize);
    
    return true;
}

extern "C" bool cambrian_GetSurfaceMaskData(int index, uint8_t** data, unsigned int* length) {
    CambrianARSession *session = m_sharedSession; if (!session || !session->m_isRunning) return false;
    
    @synchronized(session->m_surfaceTexturesMutex) {
        auto &surface = session->m_surfaceTextures[index];
        surface->getMaskPixelData(data, length);
    }
    return true;
}

extern "C" bool cambrian_GetSurfaceShadowData(int index, uint8_t** data, unsigned int* length) {
    CambrianARSession *session = m_sharedSession; if (!session || !session->m_isRunning) return false;
    
    @synchronized(session->m_surfaceTexturesMutex) {
        auto &surface = session->m_surfaceTextures[index];
        surface->getShadowPixelData(data, length);
    }
    return true;
}

extern "C" int cambrian_HitTest(CBPoint point, CBHitTestResultType types)
{
    CambrianARSession *session = m_sharedSession; if (!session || !session->m_isRunning) return 0;
    
    session->_hitTestResults = [session hitTestAtPoint:{point.x, 1.0f - point.y} resultType:types];
    
    return (int)session->_hitTestResults.size();
}

extern "C" CBHitTestResult cambrian_GetLastHitTestResult(int index)
{
    CBHitTestResult result;
    
    CambrianARSession *session = m_sharedSession; if (!session || !session->m_isRunning) return result;

    if (index >= 0 && index < session->_hitTestResults.size()) {
        result = session->_hitTestResults[index];
    }

    return result;
}

extern "C" CBUserAnchor cambrian_AddUserAnchorWithTransform(CBMatrix4x4 transform) {
    CBUserAnchor returnValue;
    CambrianARSession *session = m_sharedSession; if (!session || !session->m_isRunning) return returnValue;
    
    returnValue = [session addUserAnchorWithTransform:transform];
    
    return returnValue;
}

extern "C" float cambrian_SegmentationAverageSeconds() {
    CambrianARSession *session = m_sharedSession; if (!session || !session->m_isRunning) return 1.0;
    
    return [m_sharedSession.delegate segmentationAverageSeconds];
}

@end
