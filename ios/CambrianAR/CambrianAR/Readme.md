

## General
The Cambrian Remodeling SDK consists of various views, such as a CBRemodelingView to do augmented reality in video and still mode or a CBColorFinderView to find colors in an image. They are sub classes of UIView and handle the GPU and touches for you. They may be placed directly into a storyboard or instantiated directly and pass events, such as colors found or a tool utilized (historyChanged) as a delegate callback. In general, add the framework to your project and change a view's class within your user interface to CBRemodelingView or CBColorFinderView. The SDK is written in Objective-C, but has utilized swift compatible types and nullability so it will work well with either language. 

Getting this SDK to run requires only providing a view in your storyboard that is CBRemodelingView and calling the method startRunning().

## CBRemodelingView extends CBAugmentedView
This is the video and still based view for changing paint colors or flooring. After being added to a view controller, it will either create a scene automatically if not provided or load an existing one. 

property (weak, nonatomic, nullable) id<CBRemodelingViewDelegate> delegate; 
weak var delegate: CBRemodelingViewDelegate?

Delegate method for receiving events, such as tool actions or undo steps

@property (nonatomic, strong, nonnull) CBRemodelingScene scene;  
*var scene: CBRemodelingScene?  

A scene is a project that can be loaded or saved. It contains all information about the current view, such as paint colors used and flooring assets. To change what is present in augmented reality, interact with this property.

@property (nonatomic, assign) CBToolMode toolMode;*  
var toolMode  

Tool mode is to control what user touches do in a remodeling scene. 

    CBToolModeNone: Do nothing
    CBToolModeFill: Fill paint from a point
    CBToolModeUnfill: Remove paint from a point
    CBToolModePaintbrush: Smart drawing tool (paints up to edges with rough touches)
    CBToolModeEraser: Smart eraser tool


@property (nonatomic, readonly) BOOL isRunning;  
*var isRunning: Bool*  

Property indicating whether the video based augmented reality is active.

## State methods
(void) captureCurrentState;  
func captureCurrentState()  

Capture from video to still

(BOOL) startRunning;
func startRunning()

Start running video based augmented reality

(BOOL) stopRunning;
func stopRunning() 

Pause augmented reality.

(void) clearAll;
func clearAll()

Remove all from scene

##### History methods and properties

@property (nonatomic, readonly) int undoSize;
var undoSize: Int

This indicates how many undo steps are possible.

(void) undo;
func undo()

Undo last action, such as paint color or something painted

@property (nonatomic, readonly) int redoSize;
var redoSize: Int

This indicates how many redo steps are possible.

(void) redo;
func undo()

Redo last action.

##### CBRemodelingViewDelegate
When providing your class as a delegate to CBRemodelingView, receive events.

(void) historyChanged:(CBUndoChange)change assetID:(NSString *)assetID userData:(NSDictionary<NSString, NSString>)userData forward:(BOOL)forward;
func historyChanged(_ change: CBUndoChange, assetID: String, userData: [String: String], forward: Bool)

This event indicates the user has performed some action, such as painting, but also redoing or undoing an action.

## CBRemodelingScene
A scene object contains all asset information, such as paints and flooring, that have been utilized in an image. It is from this object, which is part of a remodeling view, that the content is controlled. It is also how projects can be saved, loaded, or copied.

#### Initialization
(nullable CBAugmentedScene ) initWithSceneID:(nonnull NSString*)sceneID;
init(sceneID: String)

Initialize with a unique scene ID, usually a UUID.

(nullable CBAugmentedScene ) initWithUIImage:(nonnull UIImage *)image;
init(uiImage image: UIImage)

Initialize from an image, such as a user's gallery.

(nullable CBAugmentedScene *) initWithUIImage:(nonnull UIImage *)image sceneID:(nonnull NSString*)sceneID;  
init(uiImage image: UIImage, sceneID: String)

Initialize from an image, such as a user's gallery, assigning a scene ID.

(nullable CBAugmentedScene *) initWithPath:(nonnull NSString *)path;
init(path: String)

Basic initialize.

##### Properties and methods
@property (readonly, nonatomic, nonnull) NSString *sceneID;
var sceneID:String 

Read only property for storage and identification

@property (readonly, nonatomic, nonnull) NSDictionary<NSString *, CBAugmentedAsset *> *assets;
var assets: [String: CBAugmentedAsset]  

Dictionary of scene assets, such as paints or flooring in the scene. These are mapped by the asset ID key.

@property (readonly, nonatomic, nonnull) NSMutableDictionary<NSString *, NSString *> userData;  
var userData: [String: String]

Arbitrary user data dictionary for storing information about the scene, such as brand information or other important user data.

@property (strong, nonatomic, nullable) CBRemodelingPaint *selectedPaint;
var selectedPaint: CBRemodelingPaint

Returns the currently selected paint, if available.

@property (strong, nonatomic, nullable) CBRemodelingFloor *selectedFloor; 
var selectedFloor: CBRemodelingFloor 

Returns the currently selected floor, if available.

@property (strong, nonatomic, nullable) NSString *selectedAssetID;
var selectedAssetID: String?

Returns the currently selected asset ID, if available.

@property (nonatomic, assign) CBLightingType lightingAdjustment; 
var lightingAdjustment: CBLightingType

Get or set the current lighting condition:

    CBLightingTypeNone,
    CBLightingTypeIncandescent,
    CBLightingTypeFluorescent,
    CBLightingTypeLEDWhite,
    CBLightingTypeLEDWarm,
    CBLightingTypeDaylightMorning,
    CBLightingTypeDaylight,
    CBLightingTypeDaylightEvening,
    CBLightingTypeDaylightOvercast,

@property (readonly, nonatomic) BOOL isMasked;
var isMasked: Bool

Returns whether or not anything in this scene has been painted or added.

(NSDictionary<NSString *, CBAugmentedAsset *>*)getAssets:(CBAssetType)type;
func getAssets(_ type: CBAssetType) -> [String: CBAugmentedAsset]

Return all assets of a particular type:

    CBAssetTypeAll,
    CBAssetTypePaint,
    CBAssetTypeFloor,
    CBAssetTypeModel,

(NSString *) saveToDirectory:(nonnull NSString *)path compressed:(BOOL)compress;
func save(toDirectory path: String, compressed compress: Bool) -> String

(BOOL) appendAsset:(nonnull CBAugmentedAsset *)asset;
func appendAsset(asset:CBAugmentedAsset)->Bool

Append an asset, such as a new paint layer or a new floor

(BOOL) removeAsset:(nonnull NSString *)assetID;
func removeAsset(assetID:String)->Bool

Remove an asset

#### Static accessors for outside view controllers and data access

(nullable UIImage *)getOriginal:(NSString *)path;
class func getOriginal(_ path: String) -> UIImage?

Return the raw image this is based upon, un-rendered.

(nullable UIImage *)getPreview:(NSString *)path; 
class func getPreview(_ path: String) -> UIImage? 

Return a preview image of this scene.

(nullable UIImage *)getThumbnail:(NSString *)path;  
class func getThumbnail(_ path: String) -> UIImage? 

Return a thumbnail of this preview image.

(nullable UIImage *)getBeforeAfter:(NSString *)imagePath isHorizontal:(BOOL)isHorizontal;
class func getBeforeAfter(_ imagePath: String, isHorizontal: Bool) -> UIImage?

Return a before and after image side by side, the original and the preview image.

## CBRemodelingPaint extends CBAugmentedAsset
(nonnull CBAugmentedAsset *) initWithAssetID:(NSString *)assetID;
init(assetID: String) 

Initialize this paint or floor with an asset ID.

@property (readonly, nonatomic) CBAssetType assetType;
var assetType = CBAssetType

Retrieve an asset type, such as flooring or paint, for this asset

@property (strong, nonatomic, nonnull, readonly) NSString *assetID;
var assetID: String

Retrieve an asset ID for this item

@property (nonatomic, assign) CBPaintSheen sheen; 
var sheen: CBPaintSheen 

Get or set the current sheen value

    CBPaintSheenFlat,
    CBPaintSheenMatte,
    CBPaintSheenEggshell,
    CBPaintSheenSatin,
    CBPaintSheenSemiGloss,
    CBPaintSheenHighGloss,

@property (nonatomic, strong, nonnull) UIColor* color;**  
var color: UIColor?**  

Get or set the current color

(nullable NSString *) getUserData:(NSString *)key;
func getUserData(_ key: String) -> String?
Set the user controlled dictionary of information about this asset, such as paint color code.

(void) setUserData:(NSString *)key value:(NSString *)value;
func setUserData(_ key: String) -> String?
Set the user controlled dictionary of information about this asset, such as paint color code.




