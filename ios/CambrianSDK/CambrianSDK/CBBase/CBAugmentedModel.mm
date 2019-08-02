//
//  CBRemodelingPaint.m
//  CambrianAR
//
//  Created by Joel Teply on 3/3/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import "CBAugmentedModel.h"

#import "CBARInternal.h"
#import "ImageTranslation.h"
#import "CBRemodelingTypes.h"
#include <cbar/CBAR_Common.hpp>
#import "CBARInternal.h"

#define SHOW_BOUNDING_CUBE 0

@interface CBAugmentedModel() {
    NSString *_path;
    float _scale;
    
    std::shared_ptr<cbscene::CBAR_Model> _coreAsset;
    std::shared_ptr<CBARAssetCallback> _coreCallback;
}

@property (strong, nonatomic) NSString *lastPath;

@end

@implementation CBAugmentedModel

- (nullable CBAugmentedAsset *) init {

    if (self = [super init]) {
        NSString *assetID = [[NSUUID UUID] UUIDString];
        _coreCallback = std::shared_ptr<CBARAssetCallback>(new CBARAssetCallback(self));
        _coreAsset = std::shared_ptr<cbscene::CBAR_Model>(new cbscene::CBAR_Model([assetID UTF8String], _coreCallback));
    }
    return self;
}

- (std::shared_ptr<CBARAssetCallback>) coreCallback {
    return _coreCallback;
}

- (std::shared_ptr<cbscene::CBAR_Asset>)coreAsset {
    return std::dynamic_pointer_cast<cbscene::CBAR_Asset>(_coreAsset);
}

- (CBAssetType)assetType {
    return CBAssetTypeModel;
}

- (std::shared_ptr<cbscene::CBAR_Model>) coreModel {
    return std::dynamic_pointer_cast<cbscene::CBAR_Model>(self.coreAsset);
}

- (void)setPath:(NSString *) path scale:(float)scale {
    //auto path = ""//convert NS to std
    _path = path;
    _scale = scale;
    
    if (self.scene) {
        [self assetChanged];
    }
}

- (void) attachedToScene:(CBAugmentedScene *)scene {
    [super attachedToScene:scene];
}

- (void) detachedFromScene:(CBAugmentedScene *)scene {
    [super detachedFromScene:scene];
}

- (std::string) getImagePath:(NSString *)path named:(NSString *)name {
    return cbar::getUserAssetPath(string_sprintf("%s/%s", [path UTF8String], [name UTF8String]));
}

- (NSString *)getModelFile:(std::string)basePath {
    NSError *error;
    NSString *rootPath = [[NSString stringWithUTF8String:basePath.c_str()] stringByAppendingPathComponent:_path];
    NSArray<NSString *> *files = [NSFileManager.defaultManager contentsOfDirectoryAtPath:rootPath error:&error];
    for (NSString * file in files) {
        if ([file.pathExtension isEqualToString:@"dae"]) {
            return [rootPath stringByAppendingPathComponent:file];
            break;
        }
    }
    return nil;
}

- (void) assetChanged {
    
    CambrianARSession *session = CambrianUnityInterface.sharedInstance.session;
    
    //retain, until out of scope
    cbar::CBConfig config = cbar::getCBConfig();
    
    NSString *modelPath = [self getModelFile:config.primaryAssetPath];
    if (!modelPath) {
        modelPath = [self getModelFile:config.secondaryAssetPath];
    }
    
    std::string assetID = self.coreModel->getAssetID();
    
    CBModelAssetParams params;
    params.assetType = CBAssetTypeModel;
    params.scale = _scale;
    
    params.modelPath = (void *)[modelPath UTF8String];
    
    [session modelAssetUpdated:params];
}

+ (BOOL)canCreate {
    return hasARKit();
}

@end

