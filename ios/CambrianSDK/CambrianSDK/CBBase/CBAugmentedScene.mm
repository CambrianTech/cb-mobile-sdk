//
//  CBTexture.m
//  HomeAugmentation
//
//  Created by Joel Teply on 11/7/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#import "CBAugmentedScene.h"
#import "CBARInternal.h"
#import "ImageTranslation.h"

#import "CBAugmentedAsset.h"
#import "CBRemodelingPaint.h"
#import "CBRemodelingFloor.h"
#import "CBAugmentedModel.h"

#include <cbar/pipeline/CBP_Types.hpp>

@implementation CBAugmentedScene {
    std::shared_ptr<CBARSceneCallback> _coreCallback;
    NSString *_storedSceneID;
}

@dynamic selectedAsset;
@dynamic selectedAssetID;
@dynamic assets;
@dynamic lightingAdjustment;
@synthesize coreScene = _coreScene;

- (void)dealloc
{
    NSLog(@"Deallocating %@", NSStringFromClass([self class]));
    //if (_coreScene) _coreScene.reset();
}

- (std::shared_ptr<cbscene::CBAR_Scene>)coreScene {
    if (!_coreScene) {
        [self generateCoreScene:_storedSceneID ? _storedSceneID : [[NSUUID UUID] UUIDString]];
    }
    return _coreScene;
}

- (std::shared_ptr<CBARSceneCallback>) coreCallback {
    if (!_coreCallback) {
        auto *callback = [self generateCoreCallback];
        _coreCallback = std::shared_ptr<CBARSceneCallback>(callback);
    }
    return _coreCallback;
}

- (CBARSceneCallback *) generateCoreCallback {
    return new CBARSceneCallback(self);
}

- (id) init {
    if (self = [super init]) {
        _userData = [NSMutableDictionary<NSString *, NSString *> new];
    }
    return self;
}

- (nullable CBAugmentedScene *) initWithSceneID:(nonnull NSString*)sceneID {
    if (self = [super init]) {
        _storedSceneID = sceneID;
        _userData = [NSMutableDictionary<NSString *, NSString *> new];
        [self generateCoreScene:sceneID];
    }
    return self;
}


- (nullable CBAugmentedScene *) initWithUIImage:(nonnull UIImage *)image {
    return [self initWithUIImage:image sceneID:@""];
}

- (nullable CBAugmentedScene *) initWithUIImage:(UIImage *)image sceneID:(nonnull NSString*)sceneID {
    if (self = [super init]) {
        image = [ImageTranslation scaleAndRotateImage:image constrainedToSize:CGSizeMake(1280, 1280)];
        cv::Mat rgbaImage = [ImageTranslation CVMatForImage:image];
        cv::cvtColor(rgbaImage, rgbaImage, CV_BGRA2RGBA);
        [self generateCoreScene:sceneID rgbaImage:rgbaImage];
    }
    
    return self;
}

- (nullable CBAugmentedScene *) initWithPath:(nonnull NSString *)path {
    if (self = [self init]) {
        CBSessionState state;
        state.isVideo = false;
        [CambrianUnityInterface.sharedInstance.session sceneChanging:state];
        self.coreScene->loadFromDirectory([path UTF8String], 0);
    }
    return self;
}

- (void) generateCoreScene:(NSString *)sceneID {
    self.coreScene = std::shared_ptr<cbscene::CBAR_Scene>(new cbscene::CBAR_Scene(self.coreCallback, [sceneID UTF8String], (__bridge void *)self));
}

- (void) generateCoreScene:(NSString *)sceneID rgbaImage:(cv::Mat)rgbaImage {
    self.coreScene = std::shared_ptr<cbscene::CBAR_Scene>(new cbscene::CBAR_Scene(self.coreCallback, rgbaImage, [sceneID UTF8String], (__bridge void *)self));
}

- (NSString *)sceneID {
    return [NSString stringWithUTF8String:self.coreScene->getSceneID().c_str()];
}

- (nullable CBAugmentedAsset *)createAssetOfType:(cbscene::CBAR_Asset::asset_type)assetType assetID:(NSString *)assetID {
    CBAugmentedAsset *asset = nil;
    switch (assetType) {
        case cbscene::CBAR_Asset::asset_type_paint:
            asset = [[CBRemodelingPaint alloc] initWithAssetID:assetID];
            break;
        case cbscene::CBAR_Asset::asset_type_floor:
            asset = [[CBRemodelingFloor alloc] initWithAssetID:assetID];
            break;
        case cbscene::CBAR_Asset::asset_type_model:
            asset = [[CBAugmentedModel alloc] initWithAssetID:assetID];
            break;
        default:
            asset = [[CBAugmentedAsset alloc] initWithAssetID:assetID];
            break;
    }
    
    return asset;
}

+ (nullable UIImage *)getOriginal:(NSString *)path {
    cv::Mat result = cbscene::CBAR_Scene::getOriginalImageAtPath([path UTF8String]);
    if (!result.empty()) {
        return [ImageTranslation imageWithCVMat:result];
    }
    return nil;
}

+ (nullable UIImage *)getPreview:(NSString *)path {
    cv::Mat result = cbscene::CBAR_Scene::getPreviewImageAtPath([path UTF8String]);
    if (!result.empty()) {
        return [ImageTranslation imageWithCVMat:result];
    }
    return nil;
}

+ (nullable UIImage *)getThumbnail:(NSString *)path {
    return [self getPreview:path];
}

+ (nullable UIImage *)getBeforeAfter:(NSString *)imagePath isHorizontal:(BOOL)isHorizontal {
    cv::Mat preview = cbscene::CBAR_Scene::getPreviewImageAtPath([imagePath UTF8String]);
    cv::Mat original = cbscene::CBAR_Scene::getOriginalImageAtPath([imagePath UTF8String]);
    if (!preview.empty() && !original.empty()) {
        cv::Mat combined;
        cv::resize(preview, preview, original.size());
        if (isHorizontal) {
            cv::hconcat(original, preview, combined);
        } else {
            cv::vconcat(original, preview, combined);
        }
        return [ImageTranslation imageWithCVMat:combined];
    }
    
    return nil;
}

- (void) saveToDirectory:(nonnull NSString *)path compressed:(BOOL)compress completion:(void (^)(NSString* finalPath, NSString* projectJSON))completion; {
    
    __weak typeof(self) weakSelf = self;
    
    [[CBAugmentedView getInstance] dispatch_cb_get_result:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        std::string json;
        std::string fPath;
        strongSelf.coreScene->saveToDirectory([path UTF8String], compress, fPath, json);
        if (completion) {
            NSString *finalPath = [NSString stringWithUTF8String:fPath.c_str()];
            NSString *projectJSON = [NSString stringWithUTF8String:json.c_str()];
            dispatch_async(dispatch_get_main_queue(), ^{
                completion(finalPath, projectJSON);
            });
        }
    }];
}

- (NSDictionary<NSString *, CBAugmentedAsset *> *) assets {
    NSMutableDictionary<NSString *, CBAugmentedAsset *> *assets = [NSMutableDictionary<NSString *, CBAugmentedAsset *> new];
    
    for (const auto &it: self.coreScene->getAssets()) {
        NSString *key = [NSString stringWithUTF8String:it.first.c_str()];
        auto coreAsset = it.second;
        if (coreAsset) {
            auto callback = std::dynamic_pointer_cast<CBARAssetCallback>(coreAsset->getCallback());
            assets[key] = callback->getAsset();
        }
    }
    
    return assets;
}

- (CBLightingType) lightingAdjustment {
    CBLightingType lighting = (CBLightingType)self.coreScene->getLighting();
    return lighting;
}

- (void) setLightingAdjustment:(CBLightingType)lighting {
    self.coreScene->setLighting((LightingType) lighting);
}

- (CBAugmentedAsset *)selectedAsset {
    if (self.selectedAssetID) {
        return self.assets[self.selectedAssetID];
    }
    return nil;
}

- (void) setSelectedAsset:(CBAugmentedAsset *)selectedAsset {
    self.coreScene->setSelectedAsset(selectedAsset.coreAsset);
    
    [self assetSelectionChanged];
}

- (NSString *) selectedAssetID {
    std::string coreID = self.coreScene->getSelectedAssetID();
    if (coreID.empty()) return nil;
    return [NSString stringWithUTF8String:coreID.c_str()];
}

- (void) setSelectedAssetID:(NSString *)selectedAssetID {
    self.coreScene->setSelectedAssetID([selectedAssetID UTF8String]);
    [self assetSelectionChanged];
}

- (NSDictionary<NSString *, CBAugmentedAsset *>*)getAssets:(CBAssetType)type {
    NSDictionary<NSString *, CBAugmentedAsset *> *assets = self.assets;
    
    if (type == CBAssetTypeAll) return assets;
    
    NSMutableDictionary<NSString *, CBAugmentedAsset *>*filtered = [NSMutableDictionary<NSString *, CBAugmentedAsset *> new];
    for (NSString * key in assets) {
        CBAugmentedAsset *asset = assets[key];
        if (asset.assetType == type) {
             filtered[key] = asset;
        }
    }

    return filtered;
}

- (void)pushUserData {
    self.coreScene->getUserData().clear();
    for (NSString * key in self.userData) {
        NSString *value = self.userData[key];
        self.coreScene->getUserData()[std::string(key.UTF8String)] = std::string(value.UTF8String);
    }
}

- (void)pullUserData {
    [self.userData removeAllObjects];
    
    for (auto it: self.coreScene->getUserData()) {
        NSString *key = [NSString stringWithUTF8String:it.first.c_str()];
        NSString *value = [NSString stringWithUTF8String:it.second.c_str()];
        self.userData[key] = value;
    }
}

- (int) assetCount:(CBAssetType)assetType {
    return self.coreScene->getAssetCount(cbscene::CBAR_Asset::asset_type(assetType));
}

- (BOOL) canAppendAsset:(CBAssetType)assetType {
    return self.coreScene->canAppendAsset(cbscene::CBAR_Asset::asset_type(assetType));
}

- (BOOL) appendAsset:(CBAugmentedAsset *)asset {
    bool success = self.coreScene->appendAsset(asset.coreAsset);
    if (success) {
        
        __weak typeof(self) weakSelf = self;
        [[CBAugmentedView getInstance] runCommand: ^{
            __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
            CBAssetParams params;
            
            std::string assetID = asset.coreAsset->getAssetID();//retain, until out of scope
            
            params.assetType = asset.coreAsset->getType();
            params.worldTransform = eigenToCBMat(asset.coreAsset->getWorldTransform());
            
            [CambrianUnityInterface.sharedInstance.session assetAdded:params];
            
            if (auto surfaceAsset = std::dynamic_pointer_cast<cbscene::CBAR_SurfaceAsset>(asset.coreAsset)) {
                CBSurfaceAssetParams surfaceParams;
                surfaceParams.assetType = params.assetType;
                surfaceParams.index = surfaceAsset->getIndex();
                surfaceParams.maskWidth = surfaceAsset->getMaskSize().width;
                surfaceParams.maskHeight = surfaceAsset->getMaskSize().height;
                surfaceParams.shadowsWidth = surfaceAsset->getShadowsSize().width;
                surfaceParams.shadowsHeight = surfaceAsset->getShadowsSize().height;
                [CambrianUnityInterface.sharedInstance.session surfaceAssetCreated:surfaceParams];
            }
        }];
        //printf("ASSET ID= %s", assetID.c_str());
    }
    return success;
}

- (BOOL) removeAsset:(NSString *)assetID {
    auto asset = self.coreScene->getAssets().find([assetID UTF8String]);
    
    bool success = self.coreScene->removeAsset([assetID UTF8String]);
    if (success) {
        CBAssetParams params;
        params.assetType = asset->second->getType();
        [CambrianUnityInterface.sharedInstance.session assetRemoved:params];
    }
    return success;
}

- (void)assetSelectionChanged {
    if (auto asset = self.selectedAsset) {
        CBAssetParams params;
        params.assetType = asset.assetType;
        [CambrianUnityInterface.sharedInstance.session assetSelected:params];
    }
}

- (void)coreAssetAppended:(std::shared_ptr<cbscene::CBAR_Asset>)coreAsset {
    
}

- (void)coreAssetRemoved:(std::shared_ptr<cbscene::CBAR_Asset>)coreAsset {
    
}

@end

std::shared_ptr<cbscene::CBAR_Asset> CBARSceneCallback::generateAsset(cbscene::CBAR_Asset::asset_type assetType, const std::string &assetID) {
    @autoreleasepool {
        if (CBAugmentedAsset *asset = [m_scene createAssetOfType:assetType assetID:[NSString stringWithUTF8String:assetID.c_str()]]) {
            [m_scene appendAsset:asset];
            return asset.coreAsset;
        }
        return std::shared_ptr<cbscene::CBAR_Asset>();
    }
}

void CBARSceneCallback::assetAppended(std::shared_ptr<cbscene::CBAR_Asset> asset) {
    @autoreleasepool {
        [m_scene coreAssetAppended:asset];
    }
}

void CBARSceneCallback::assetRemoved(std::shared_ptr<cbscene::CBAR_Asset> asset) {
    @autoreleasepool {
        if (auto callback = std::dynamic_pointer_cast<CBARAssetCallback>(asset->getCallback())) {
            callback->releaseAsset();
        }
    }
}

