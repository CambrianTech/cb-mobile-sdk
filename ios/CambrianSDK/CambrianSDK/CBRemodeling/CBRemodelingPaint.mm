//
//  CBRemodelingPaint.m
//  CambrianAR
//
//  Created by Joel Teply on 3/3/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import "CBRemodelingPaint.h"
#import "CBARInternal.h"
#import "ImageTranslation.h"
#import "CBRemodelingTypes.h"

@interface CBRemodelingPaint() {
    std::shared_ptr<cbscene::CBAR_Paint> _coreAsset;
    std::shared_ptr<CBARSurfaceAssetCallback> _coreCallback;
}

@property (readonly, nonatomic) std::shared_ptr<cbscene::CBAR_Paint> corePaint;

@end

@implementation CBRemodelingPaint

@synthesize corePaint = _coreAsset;
@dynamic color;
@dynamic sheen;
@dynamic transparency;

- (nullable CBAugmentedAsset *) init {
    
    if (self = [super init]) {
        NSString *assetID = [[NSUUID UUID] UUIDString];
        _coreCallback = std::shared_ptr<CBARSurfaceAssetCallback>(new CBARSurfaceAssetCallback(self));
        _coreAsset = std::shared_ptr<cbscene::CBAR_Paint>(new cbscene::CBAR_Paint([assetID UTF8String], _coreCallback));
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
    return CBAssetTypePaint;
}

- (UIColor *)color {
    auto cvColor = self.corePaint->getColor();
    return [ImageTranslation UIColorForScalar:cvColor];
}

- (void)setColor:(UIColor *)color {
    auto cvColor = [ImageTranslation scalarForUIColor:color];
    self.corePaint->setColor(cvColor);
    
    [self assetChanged];
}

- (CBPaintSheen)sheen {
    Sheen coreSheen = self.corePaint->getSheen();
    return (CBPaintSheen) coreSheen;
}

- (void)setSheen:(CBPaintSheen)sheen {
    self.corePaint->setSheen((Sheen)sheen);
    [self assetChanged];
}

- (CBTransparency)transparency {
    Transparency coreSheen = self.corePaint->getTransparency();
    return (CBTransparency) coreSheen;
}

- (void)setTransparency:(CBTransparency)transparency {
    self.corePaint->setTransparency((Transparency)transparency);
    [self assetChanged];
}

- (void)assetChanged {
    auto cvColor = self.corePaint->getColor();
    
    std::string assetID = self.corePaint->getAssetID();
    
    CBPaintAssetParams params;
    params.assetType = CBAssetTypePaint;
    params.index = self.corePaint->getIndex();
    
    params.colorRed = cvColor[0] / 255.0f;
    params.colorGreen = cvColor[1] / 255.0f;
    params.colorBlue = cvColor[2] / 255.0f;
    params.colorAlpha = cvColor[3] / 255.0f;
    
    [CambrianUnityInterface.sharedInstance.session paintAssetUpdated:params];
}

@end
