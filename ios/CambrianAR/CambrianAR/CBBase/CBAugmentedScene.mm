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
#import "CBRemodelingFurniture.h"

#include <cbar/pipeline/CBP_Types.hpp>

@implementation CBAugmentedScene {
    NSString *_storedSceneID;
}

@dynamic selectedAsset;
@dynamic selectedAssetID;
@dynamic assets;
@dynamic lightingAdjustment;
@synthesize coreScene = _coreScene;
@dynamic genAssetFunction;

- (void)dealloc
{
    NSLog(@"Deallocating %@", NSStringFromClass([self class]));
    
    NSEnumerator *enumerator = [self.assets keyEnumerator];
    id key;
    // extra parens to suppress warning about using = instead of ==
    while((key = [enumerator nextObject])) {
        CBAugmentedAsset *asset = [self.assets objectForKey:key];
        [asset releaseCore];
        //NSLog(@"key=%@", key);
    }
    
    if (_coreScene) _coreScene.reset();
}

- (std::shared_ptr<cbscene::CBAR_Scene>)coreScene {
    if (!_coreScene) {
        [self generateCoreScene:_storedSceneID ? _storedSceneID : @""];
    }
    return _coreScene;
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
        
        int outputRotation = 0;
        self.coreScene->loadFromDirectory([path UTF8String], outputRotation);
    }
    return self;
}

- (cbscene::generate_asset_fn ) genAssetFunction {
    __weak typeof(self) weakSelf = self;
    return [weakSelf](cbscene::CBAR_Asset::asset_type assetType, const std::string &assetID) {
        if (!weakSelf) return std::shared_ptr<cbscene::CBAR_Asset>();
        if (CBAugmentedAsset *asset = [weakSelf createAssetOfType:assetType assetID:[NSString stringWithUTF8String:assetID.c_str()]]) {
            [weakSelf appendAsset:asset];
            return asset.coreAsset;
        }
        return std::shared_ptr<cbscene::CBAR_Asset>();
    };
}

- (void) generateCoreScene:(NSString *)sceneID {
    self.coreScene = std::shared_ptr<cbscene::CBAR_Scene>(new cbscene::CBAR_Scene(self.genAssetFunction, [sceneID UTF8String], (__bridge void *)self));
}

- (void) generateCoreScene:(NSString *)sceneID rgbaImage:(cv::Mat)rgbaImage {
    self.coreScene = std::shared_ptr<cbscene::CBAR_Scene>(new cbscene::CBAR_Scene(self.genAssetFunction, rgbaImage, [sceneID UTF8String], (__bridge void *)self));
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
            asset = [[CBRemodelingFurniture alloc] initWithAssetID:assetID];
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

- (void) saveToDirectory:(nonnull NSString *)path compressed:(BOOL)compress completion:(void (^)(NSString* finalPath, NSString* json))completion; {
    
    __weak typeof(self) weakSelf = self;
    
    [[CBAugmentedView getInstance] dispatch_cb_get_result:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        std::string newPath;
        std::string projectJSON;
        strongSelf.coreScene->saveToDirectory([path UTF8String], compress, newPath, projectJSON);
        if (completion) {
            NSString *finalPath = [NSString stringWithUTF8String:newPath.c_str()];
            NSString *json = [NSString stringWithUTF8String:projectJSON.c_str()];
            dispatch_async(dispatch_get_main_queue(), ^{
                completion(finalPath, json);
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
    if (@available(iOS 11.0, *)) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [CBAugmentedView getInstance].arSceneVisible = [selectedAsset isKindOfClass:[CBAugmentedModel class]];
        });
    }
    self.coreScene->setSelectedAsset(selectedAsset.coreAsset);
}

- (NSString *) selectedAssetID {
    std::string coreID = self.coreScene->getSelectedAssetID();
    if (coreID.empty()) return nil;
    return [NSString stringWithUTF8String:coreID.c_str()];
}

- (void) setSelectedAssetID:(NSString *)selectedAssetID {
    self.coreScene->setSelectedAssetID([selectedAssetID UTF8String]);
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
    return self.coreScene->appendAsset(asset.coreAsset);
}

- (BOOL) removeAsset:(NSString *)assetID {
    return self.coreScene->removeAsset([assetID UTF8String]);
}

@end
