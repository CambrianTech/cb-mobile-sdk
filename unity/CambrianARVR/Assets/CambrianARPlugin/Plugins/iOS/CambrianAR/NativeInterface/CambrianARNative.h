//
//  CambrianARNative.h
//  UnityAsFramework
//
//  Created by Joel Teply on 12/10/17.
//

#import <ARKit/ARKit.h>

#ifndef CambrianARNative_h
#define CambrianARNative_h

#ifdef __cplusplus
#define START_STRUCT(T, Base)   struct T : Base {
#define END_STRUCT(T)           };
#else
#define START_STRUCT(T, Base)   typedef struct T { struct Base;
#define END_STRUCT(T)           } T;
#endif

#include <vector>
#include <string>

//structs MUST MATCH UNITY!!!
#pragma pack(push, 0)

typedef struct
{
    float x;
    float y;
} CBPoint;

typedef struct
{
    float x,y,z;
} CBVector3;

typedef struct
{
    float x=0.f;
    float y=0.f;
    float z=0.f;
    float w=0.f;
} CBVector4;

typedef struct CBPointCloudData {
    std::vector<CBVector3> points;
    std::vector<uint64_t> identifiers;
} CBPointCloudData;

typedef struct
{
    CBVector4 column0;
    CBVector4 column1;
    CBVector4 column2;
    CBVector4 column3;
} CBMatrix4x4;

typedef struct CBPlane {
    CBVector3 center;
    CBVector3 normal;
} CBPlane;

enum CBTrackingState
{
    UnityARTrackingStateNotAvailable,
    UnityARTrackingStateLimited,
    UnityARTrackingStateNormal,
};

enum CBTrackingReason
{
    UnityARTrackingStateReasonNone,
    UnityARTrackingStateReasonInitializing,
    UnityARTrackingStateReasonExcessiveMotion,
    UnityARTrackingStateReasonInsufficientFeatures,
};

typedef struct
{
    uint32_t yWidth = 0;
    uint32_t yHeight = 0;
    uint32_t yStride = 0;
    uint32_t screenOrientation = 0;
    float texCoordScale;
    char* pYPixelBytes = 0;
    char* pUVPixelBytes = 0;
}CBVideoParams;

typedef struct
{
    float ambientIntensity;
    float ambientColorTemperature;
}CBLightEstimation;

typedef struct
{
    CBVector4 primaryLightDirectionAndIntensity;
    float *sphericalHarmonicsCoefficients;
} CBDirectionalLightEstimate;

enum CBLightDataType
{
    LightEstimate,
    DirectionalLightEstimate
};

typedef struct
{
    CBLightDataType lightingType;
    CBLightEstimation lightEstimate;
    CBDirectionalLightEstimate directionalLightEstimate;
} CBLightData;

typedef struct
{
    CBMatrix4x4 cameraTransform;
    CBMatrix4x4 projectionMatrix;
    CBTrackingState trackingState;
    CBTrackingReason trackingReason;
    CBVideoParams videoParams;
    CBLightData lightData;
    CBMatrix4x4 displayTransform;
} CBCameraFrame;

typedef enum CBHitTestResultType
{
    /** Result type from intersecting the nearest feature point. */
    CBHitTestResultTypeFeaturePoint     = (1 << 0),
    
    /** Result type from detecting and intersecting a new horizontal plane. */
    CBHitTestResultTypeHorizontalPlane  = (1 << 1),
    
    /** Result type from detecting and intersecting a new vertical plane. */
    CBHitTestResultTypeVerticalPlane    = (1 << 2),
    
    /** Result type from intersecting with an existing plane anchor. */
    CBHitTestResultTypeExistingPlane    = (1 << 3),
    
    /** Result type from intersecting with an existing plane anchor, taking into account the plane's extent. */
    CBHitTestResultTypeExistingPlaneUsingExtent  = ( 1 << 4),
    
    CBHitTestResultTypeExistingPlaneUsingGeometry  = ( 1 << 5)
} CBHitTestResultType;

typedef struct
{
    CBHitTestResultType type;
    double distance;
    CBMatrix4x4 localTransform;
    CBMatrix4x4 worldTransform;
    
    //    void* anchorIdentifier;
    //    bool isValid = false;
    //    std::string anchorIdentifierStr;
    
} CBHitTestResult;

typedef enum CBPlaneAnchorAlignment {
    CBPlaneAnchorAlignmentHorizontal = (1 << 0),
    CBPlaneAnchorAlignmentVertical = (1 << 1)
} CBPlaneAnchorAlignment;

typedef struct CBPlaneGeometry {
    int vertexCount;
    float *vertices;
    int textureCoordinateCount;
    float *textureCoordinates;
    int triangleCount;
    int *triangleIndices;
    int boundaryVertexCount;
    float *boundaryVertices;
} CBPlaneGeometry;

typedef struct CBPlaneAnchor {
    
    void* anchorIdentifier;
    
    CBMatrix4x4 transform;
    CBPlaneAnchorAlignment alignment;
    
    CBVector3 center;
    CBVector3 extent;
    CBPlaneGeometry geometry;
} CBPlaneAnchor;

typedef struct CBUserAnchor {
    void* anchorIdentifier;
    CBMatrix4x4 transform;
} CBUserAnchor;

typedef struct CBContourData {
    int index;
    std::vector<CBVector3> points3D;
    CBMatrix4x4 transform;
} CBContourData;

typedef struct CBMeshData {
    std::vector<CBVector3> vertices;
    std::vector<int> indices;
} CBMeshData;

typedef struct CBImageData
{
    void *image;
    int channels = 1;
    int width;
    int height;
} CBImageData;

typedef struct CBSurfaceData {
    int index;
    
    CBVector3 center;
    CBVector3 normal;
    CBPoint rotation2D;
    
    CBVector4 meshExtents3D;
    
    CBVector4 maskExtents3D;
    CBImageData maskImageData;
    
    CBVector4 shadowsExtents3D;
    CBImageData shadowsImageData;
    
    CBMeshData mesh;    
} CBSurfaceData;

typedef struct CBSurfaceDataExtents
{
    CBVector3 center;
    CBVector3 normal;
    CBPoint rotation2D;
    
    CBVector4 meshExtents3D;
    CBVector4 maskExtents3D;
    CBVector4 shadowsExtents3D;
} CBSurfaceDataExtents;

typedef struct CBLineData {
    std::vector<CBVector3> points;
    CBVector4 color;
} CBLineData;

typedef struct CBSessionState {
    bool isVideo;
    CBMatrix4x4 worldTransform;
} CBSessionState;

typedef struct CBDrawingParams
{    
    float colorRed = 0;
    float colorGreen = 0;
    float colorBlue = 0;
    float colorAlpha = 0;
    
} CBDrawingParams;

typedef struct CBScreenshotData
{
    void *path;
    bool isJPEG;
    int qualityLevel;
} CBScreenshotData;

typedef struct CBCommandData
{
    void *command;
    void *json;
} CBCommandData;

typedef struct CBAssetParams
{
    int index = 0;
    int assetType = 0;
    CBMatrix4x4 worldTransform;
} CBAssetParams;

typedef struct CBPaintAssetParams
{
    int index = 0;
    int assetType = 0;
    
    float colorRed = 0;
    float colorGreen = 0;
    float colorBlue = 0;
    float colorAlpha = 0;
} CBPaintAssetParams;

typedef struct CBFloorAssetParams
{
    int index = 0;
    int assetType = 0;
    
    float scale;
    
    void * diffuseTexturePath;
    void * normalTexturePath;
    void * roughnessTexturePath;
    
} CBFloorAssetParams;

typedef struct CBModelAssetParams
{
    int index = 0;
    int assetType = 0;
    
    float scale;
    void * modelPath;
    
} CBModelAssetParams;

typedef struct CBSurfaceAssetParams
{
    int index = 0;
    int assetType = 0;
    
    int maskWidth;
    int maskHeight;
    
    int shadowsWidth;
    int shadowsHeight;
} CBSurfaceAssetParams;

typedef enum CBTouchPhase
{
    CBTouchPhaseBegan,
    CBTouchPhaseMoved,
    CBTouchPhaseStationary,
    CBTouchPhaseEnded,
    CBTouchPhaseCanceled
} CBTouchPhase;

typedef void (*CAMBRIAN_SESSION_CALLBACK)(CBSessionState info);
typedef void (*CAMBRIAN_FRAME_READY_CALLBACK)(CBCameraFrame frame);
typedef void (*CAMBRIAN_IMAGE_READY_CALLBACK)(CBImageData image);
typedef void (*CAMBRIAN_CAPTURE_CALLBACK)();

typedef void (*CAMBRIAN_ORIENTATION_CALLBACK)(CBVector3 normal);
typedef void (*CAMBRIAN_PLANE_ANCHOR_CALLBACK)(CBPlaneAnchor plane);
typedef void (*CAMBRIAN_USER_ANCHOR_CALLBACK)(CBUserAnchor anchor);

typedef void (*CAMBRIAN_ASSET_CALLBACK)(CBAssetParams info);

typedef void (*CAMBRIAN_SURFACE_CREATED_CALLBACK)(CBSurfaceAssetParams info);
typedef void (*CAMBRIAN_SURFACE_REMOVED_CALLBACK)(CBSurfaceAssetParams info);
typedef void (*CAMBRIAN_SURFACE_DATA_CALLBACK)(CBSurfaceAssetParams info, CBSurfaceDataExtents data);

typedef void (*CAMBRIAN_PAINT_ASSET_UPDATED_CALLBACK)(CBPaintAssetParams info);
typedef void (*CAMBRIAN_FLOOR_ASSET_UPDATED_CALLBACK)(CBFloorAssetParams info);
typedef void (*CAMBRIAN_MODEL_ASSET_UPDATED_CALLBACK)(CBModelAssetParams info);

typedef void (*CAMBRIAN_DRAWING_UPDATED_CALLBACK)(CBDrawingParams params);

typedef void (*CAMBRIAN_SCREENSHOT_CALLBACK)(CBScreenshotData screenshot);
typedef void (*CAMBRIAN_COMMAND_CALLBACK)(CBCommandData command);

//START_STRUCT(PaintAssetParams, AssetParams)
//
//END_STRUCT(PaintAssetParams)

#pragma pack(pop)

@protocol CambrianARSessionListener<NSObject>

- (void) notifyReady:(BOOL)isReady;
- (void) screenshotSaved;

- (float) segmentationAverageSeconds;
- (BOOL) needsFrame;
- (void) arFrameCallback:(CBCameraFrame)arFrame;

- (void) cameraPositionUpdated:(const CBMatrix4x4 &)modelView;
- (void) cameraProjectionUpdated:(const CBMatrix4x4 &)projection;

- (void) stillRender;

- (void) touchEvent:(CBTouchPhase)phase position:(CBPoint)point;

- (void) assetTransformUpdated:(const CBMatrix4x4 &)position assetID:(NSString *)assetID;
- (void) pointCloudDataUpdated:(CBPointCloudData)data;

- (void) planeAnchorAdded:(CBPlaneAnchor) anchordata;
- (void) planeAnchorUpdated:(CBPlaneAnchor) anchordata;
- (void) planeAnchorRemoved:(CBPlaneAnchor) anchordata;

- (void) userAnchorAdded:(CBUserAnchor) anchordata;
- (void) userAnchorUpdated:(CBUserAnchor) anchordata;
- (void) userAnchorRemoved:(CBUserAnchor) anchordata;

- (void)clearAll;

- (CBMatrix4x4) get2DAffineTransform:(const CBVector3 *)before after:(const CBVector3 *)after;

@end

@interface CambrianARSession : NSObject <ARSessionDelegate>

@property (nonatomic, weak) id<CambrianARSessionListener> delegate;
@property (nonatomic, assign) BOOL displaysVideo;

- (void)setSceneName:(NSString *)scene;

- (void)startRunning:(CBSessionState)state;
- (void)stopRunning:(CBSessionState)state;
- (void)captureToStill;
- (void)sceneChanging:(CBSessionState)state;

- (void)clearAllSurfaces;
- (void)surfaceAssetCreated:(const CBSurfaceAssetParams&)params;
- (void)surfaceDataUpdated:(const CBSurfaceData&)surface;

- (void)assetAdded:(CBAssetParams)params;
- (void)assetRemoved:(CBAssetParams)params;
- (void)assetSelected:(CBAssetParams)params;

- (void)paintAssetUpdated:(CBPaintAssetParams)params;
- (void)floorAssetUpdated:(CBFloorAssetParams)params;
- (void)modelAssetUpdated:(CBModelAssetParams)params;

- (void)drawingUpdated:(const void *)bytes width:(int)width height:(int)height color:(float*)color;

- (void)setMainCameraImage:(const void *)bytes width:(int)width height:(int)height;

- (void)saveScreenshot:(CBScreenshotData)screenshot;

- (void)orientationNormalFound:(CBVector3)normal;

//- (void)pointCloudUpdated:(CBPointCloudData)cloud;
//- (void)lineDataUpdated:(CBLineData)lineData;
//- (CBVector3) viewportToWorldPoint:(const CBVector3&)viewportPoint;
//- (CBVector3) worldToViewportPoint:(const CBVector3&)worldPoint;

- (std::vector<CBHitTestResult>) hitTestAtPoint:(CBPoint)point resultType:(CBHitTestResultType)resultType;

- (CBUserAnchor) addUserAnchorWithTransform:(const CBMatrix4x4 &)transform;

- (void)sendUnityCommand:(CBCommandData)command;

@end


#endif /* CambrianARNative_h */
