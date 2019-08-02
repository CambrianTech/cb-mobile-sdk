//
//  CBARInternal.h
//  CambrianAR
//
//  Created by Joel Teply on 12/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBARInternal_h
#define CBARInternal_h


#include <memory>
#import "CBAugmentedView.h"
#import "CBAugmentedScene.h"
#import "CBAugmentedAsset.h"
#import "CBSurfaceAsset.h"
#import "CBAugmentedModel.h"

#include <cbar/CBAR_View.hpp>
#include <cbar/scene/scene.h>
#include <cbcommon/CB_Types.h>
#include <ARKit/ARKit.h>
#import <CambrianUnity/CambrianUnity.h>

namespace cbar {
    class CBAR_View;
}

struct CBARSceneCallback : public cbscene::CBAR_SceneCallbackI {
    
    CBARSceneCallback(__weak CBAugmentedScene * _Nonnull scene) : m_scene(scene) {
        
    }
    
    CBAugmentedScene* _Nonnull getScene() {
        return m_scene;
    }
    
    virtual std::shared_ptr<cbscene::CBAR_Asset> generateAsset(cbscene::CBAR_Asset::asset_type assetType, const std::string &assetID);
    virtual void assetAppended(std::shared_ptr<cbscene::CBAR_Asset> asset);
    virtual void assetRemoved(std::shared_ptr<cbscene::CBAR_Asset> asset);
    
private:
    __weak CBAugmentedScene * _Nonnull m_scene;
};

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
    
    void releaseAsset() {
        m_asset = 0;
    }
    
private:
    void _touchesCommon(const cbpipe::TouchPoint &touch,
                        CBAugmentedAsset * _Nonnull * _Nonnull asset,
                        CGPoint * _Nonnull touchPoint);
    CBAugmentedAsset * _Nonnull m_asset;
};

struct CBARSurfaceAssetCallback : public CBARAssetCallback, public cbscene::CBAR_SurfaceAssetCallbackI {
    CBARSurfaceAssetCallback(CBAugmentedAsset * _Nonnull asset);
    
    virtual void attachedToScene();
    virtual void detachedFromScene();
    
    virtual void surfaceDataUpdated(const cbscene::CBAR_SurfaceData &cbSurface);
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

NS_ASSUME_NONNULL_BEGIN

@interface CBAugmentedView()

@property (assign, nonatomic) std::shared_ptr<cbar::CBAR_View> coreView;
@property (weak, nonatomic, nullable) id<CBAugmentedViewViewDelegate> rootDelegate;

+ (CBAugmentedView *) getInstance;

- (void) runCommand:(dispatch_block_t)command;

- (std::shared_ptr<cbar::CBAR_View>) generateCoreView:(std::shared_ptr<cbar::CBAR_CallbackI>)callback;

- (void) touch:(NSSet *)touches step:(TouchStep)step;

- (void) generateScene;

- (void) historyChanged:(std::shared_ptr<cbpipe::UndoState>)undo forward:(BOOL)forward;

- (SCNVector3) getGroundPosition3D:(CGPoint)touchPoint;

- (CBAugmentedAsset *)assetAtPoint:(CGPoint)point;

@property (nonatomic, strong) dispatch_queue_t cb_queue;
- (void) dispatch_cb:(dispatch_block_t) block;
- (void) dispatch_cb_get_result:(dispatch_block_t) block;

@property (nonatomic, strong) dispatch_queue_t unity_queue;

@property (nonatomic, strong) dispatch_queue_t upload_queue;

@end

@interface CBAugmentedScene ()

@property (assign, nonatomic) std::shared_ptr<cbscene::CBAR_Scene> coreScene;
@property (readonly, nonatomic) std::shared_ptr<CBARSceneCallback> coreCallback;
- (CBARSceneCallback *) generateCoreCallback;

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

- (void) assetChanged;

@end

@interface CBSurfaceAsset ()

@end

@interface CBAugmentedModel ()

@property (readonly, nonatomic) std::shared_ptr<cbscene::CBAR_Model> coreModel;

@end

NS_ASSUME_NONNULL_END

inline Eigen::Matrix4f cbMatToEigen(const CBMatrix4x4 &position) {
    Eigen::Matrix4f transform;
    memcpy(transform.data(), &position, sizeof(position));
    return transform;
}

inline CBMatrix4x4 eigenToCBMat(const Eigen::Matrix4f &position) {
    CBMatrix4x4 transform;
    memcpy(&transform, position.data(), sizeof(transform));
    return transform;
}

#endif /* CBARInternal_h */
