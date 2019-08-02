//
//  CBRemodelingFloor.m
//  CambrianAR
//
//  Created by Joel Teply on 3/3/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import "CBRemodelingFloor.h"

#import "CBARInternal.h"
#import "ImageTranslation.h"
#import "CBRemodelingTypes.h"

@interface CBRemodelingFloor() {
    NSString *_path;
    float _scale;
    
    std::shared_ptr<cbscene::CBAR_Floor> _coreAsset;
    std::shared_ptr<CBARSurfaceAssetCallback> _coreCallback;
}

@property (readonly, nonatomic) std::shared_ptr<cbscene::CBAR_Floor> coreFloor;

@end

@implementation CBRemodelingFloor
@synthesize coreFloor = _coreAsset;

- (nullable CBAugmentedAsset *) init {
    
    if (self = [super init]) {
        NSString *assetID = [[NSUUID UUID] UUIDString];
        _coreCallback = std::shared_ptr<CBARSurfaceAssetCallback>(new CBARSurfaceAssetCallback(self));
        _coreAsset = std::shared_ptr<cbscene::CBAR_Floor>(new cbscene::CBAR_Floor([assetID UTF8String], _coreCallback));
    }
    return self;
}

- (nullable CBAugmentedAsset *) initWithAssetID:(NSString *)assetID {
    if (![[self class] canCreate]) {
        return nil;
    }
    return [self init];
}

- (std::shared_ptr<CBARAssetCallback>) coreCallback {
    return dynamic_pointer_cast<CBARAssetCallback>(_coreCallback);
}

- (std::shared_ptr<cbscene::CBAR_Asset>)coreAsset {
    return std::dynamic_pointer_cast<cbscene::CBAR_Asset>(_coreAsset);
}

- (CBAssetType)assetType {
    return CBAssetTypeFloor;
}

- (std::string) getImagePath:(NSString *)path named:(NSString *)name {
    return cbar::getUserAssetPath(string_sprintf("%s/%s", [path UTF8String], [name UTF8String]));
}

- (void)setPath:(NSString *) path scale:(float)scale {
    //auto path = ""//convert NS to std
    _path = path;
    _scale = scale;
    
    if (self.scene) {
        [self assetChanged];
    }
}

- (void) assetChanged {
    CambrianARSession *session = CambrianUnityInterface.sharedInstance.session;
    
    std::string diffusePath = [self getImagePath:_path named:@"Base_Color.jpg"];
    std::string normalPath = [self getImagePath:_path named:@"Normal.jpg"];
    std::string roughnessPath = [self getImagePath:_path named:@"Roughness.jpg"];
    std::string assetID = self.coreFloor->getAssetID();
    
    CBFloorAssetParams params;
    params.assetType = CBAssetTypeFloor;
    params.index = self.coreFloor->getIndex();
    
    params.scale = _scale;
    
    params.diffuseTexturePath = (void *)diffusePath.c_str();
    params.normalTexturePath = (void *)normalPath.c_str();
    params.roughnessTexturePath = (void *)roughnessPath.c_str();
    
    [session floorAssetUpdated:params];
}


@end
