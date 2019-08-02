//
//  CBARInternal.h
//  CambrianAR
//
//  Created by Joel Teply on 2/6/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBARInternal_h
#define CBARInternal_h

#define COREML_ENABLED 0

#include <memory>

#import "CBAugmentedView.h"
#import "CBAugmentedScene.h"
#import "CBAugmentedAsset.h"
#import "CBMaskedAsset.h"
#import "CBAugmentedModel.h"

#include <cbar/CBAR_View.hpp>
#include <cbar/scene/scene.h>
#include <cbcommon/CB_Types.h>
#include <ARKit/ARKit.h>

NS_ASSUME_NONNULL_BEGIN

struct CBARAssetCallback : public cbscene::CBAR_AssetCallbackI {
    
    CBARAssetCallback(CBAugmentedAsset * _Nonnull asset) : m_asset(asset) {
        
    }
    
    virtual void attachedToScene();
    
    virtual void detachedFromScene();
    
    virtual void touchedAtPoint(cbpipe::TouchPoint touch);
    
    virtual void rotatedBy(cbpipe::TouchPoint touch);
    
    CBAugmentedAsset* _Nonnull getAsset() {
        return m_asset;
    }
    
private:
    void _touchesCommon(const cbpipe::TouchPoint &touch,
                        CBAugmentedAsset * _Nonnull * _Nonnull asset,
                        CGPoint * _Nonnull touchPoint);
    CBAugmentedAsset * _Nonnull m_asset;
};

struct CBARMaskedAssetCallback : public CBARAssetCallback, public cbscene::CBAR_MaskedAssetCallbackI {

    CBARMaskedAssetCallback(CBAugmentedAsset * _Nonnull asset) : CBARAssetCallback(asset), cbscene::CBAR_MaskedAssetCallbackI() {
        
    }
    
    virtual void attachedToScene();
    virtual void detachedFromScene();
    virtual void maskChanged(const cv::Mat &mask);
};

typedef NS_OPTIONS(NSUInteger, CollisionCategory) {
    CollisionCategoryBottom  = 1 << 0,
    CollisionCategoryCube    = 1 << 1,
};

inline bool hasARKit() {
    if (@available(iOS 11.0, *)) {
        return ARWorldTrackingConfiguration.isSupported;
    }
    return false;
}

//how to swift null ?! things: https://developer.apple.com/swift/blog/?id=25

using namespace imaging;
using namespace cbar;

@interface CBAugmentedView ()

@property (weak, nonatomic, nullable) id<CBAugmentedViewViewDelegate> rootDelegate;
@property (assign, nonatomic) std::shared_ptr<CBAR_View> coreView;
@property (nonatomic, strong) ARSCNView *arScene API_AVAILABLE(ios(11.0));
@property (nonatomic, readonly) simd_float4 cameraPosition API_AVAILABLE(ios(11.0));
@property (nonatomic, assign) BOOL arSceneVisible;

@property (nonatomic, strong) SCNNode *groundPlane;
@property (nonatomic, assign) float groundPlaneHeight;

+ (CBAugmentedView *) getInstance;

- (std::shared_ptr<CBAR_View>) generateCoreView:(std::shared_ptr<CBAR_CallbackI>)callback;

- (void) touch:(NSSet *)touches step:(TouchStep)step;

- (void) initialize;

- (void) handleInternalError:(int)error;

- (void) generateScene;

- (void) historyChanged:(std::shared_ptr<cbpipe::UndoState>)undo forward:(BOOL)forward;

- (SCNVector3) getGroundPosition3D:(CGPoint)touchPoint;

- (CBAugmentedAsset *)assetAtPoint:(CGPoint)point;

@property (nonatomic, strong) dispatch_queue_t cb_queue;
- (void) dispatch_cb:(dispatch_block_t) block;
- (void) dispatch_cb_get_result:(dispatch_block_t) block;

@end

@interface CBAugmentedScene ()

@property (assign, nonatomic) std::shared_ptr<cbscene::CBAR_Scene> coreScene;
@property (readonly, nonatomic) cbscene::generate_asset_fn genAssetFunction;

- (void) generateCoreScene:(NSString *)sceneID;
- (void) generateCoreScene:(NSString *)sceneID rgbaImage:(cv::Mat)rgbaImage;

- (void)pushUserData;
- (void)pullUserData;

@end

@interface CBAugmentedAsset ()

@property (readonly, nonatomic) std::shared_ptr<cbscene::CBAR_Asset> coreAsset;
@property (readonly, nonatomic) std::shared_ptr<CBARAssetCallback> coreCallback;

- (void) touchedGroundPlaneAt:(ARHitTestResult *)hitResult scene:(ARSCNView *)scene API_AVAILABLE(ios(11.0));

- (void) touchedAtPoint:(CGPoint)point step:(TouchStep)step;
- (void) rotatedAtPoint:(CGPoint)point step:(TouchStep)step rotation:(float)amount;

- (void) attachedToScene:(CBAugmentedScene *)scene;
- (void) detachedFromScene:(CBAugmentedScene *)scene;
- (void) releaseCore;//kind of a hack

- (std::string)getPassedAssetID;

@end

@interface CBMaskedAsset ()

@property (readonly, nonatomic) std::shared_ptr<CBARMaskedAssetCallback> maskedCallback;

@end

@interface CBAugmentedModel ()

@property (strong, nonatomic) SCNNode *modelNode;
- (BOOL)isMyNode:(SCNNode *)node;

@end

NS_ASSUME_NONNULL_END

#endif /* CBARInternal_h */
