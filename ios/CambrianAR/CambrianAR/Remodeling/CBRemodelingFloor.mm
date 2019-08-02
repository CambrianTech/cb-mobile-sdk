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
    std::shared_ptr<cbscene::CBAR_Floor> m_coreFloor;
    std::shared_ptr<CBARMaskedAssetCallback> m_maskedCallback;
}

@property (readonly, nonatomic) std::shared_ptr<cbscene::CBAR_Floor> coreFloor;

@end

@implementation CBRemodelingFloor

- (CBAssetType)assetType {
    return CBAssetTypeFloor;
}

- (void)releaseCore {
    m_coreFloor.reset();
    m_maskedCallback.reset();
}

- (std::shared_ptr<cbscene::CBAR_Floor>) coreFloor {
    @synchronized(self) {
        if (!m_coreFloor) {
            m_maskedCallback = std::shared_ptr<CBARMaskedAssetCallback>(new CBARMaskedAssetCallback(self));
            m_coreFloor = std::shared_ptr<cbscene::CBAR_Floor>(new cbscene::CBAR_Floor([self getPassedAssetID], m_maskedCallback));
        }
    }
    return m_coreFloor;
}

- (std::shared_ptr<cbscene::CBAR_Asset>)coreAsset {
    return std::dynamic_pointer_cast<cbscene::CBAR_Asset>(self.coreFloor);
}

- (void) touchedGroundPlaneAt:(ARHitTestResult *)hitResult scene:(ARSCNView *)scene API_AVAILABLE(ios(11.0)) {
    if (!self.coreFloor || !CBAugmentedView.getInstance) return;
    float elevation = hitResult.worldTransform.columns[3][1];
    self.coreFloor->setPlanePosition(Eigen::Vector3f(0, elevation, 0));
}

- (cv::Mat) getImage:(NSString *)path named:(NSString *)name {
    auto imagePath = cbar::getUserAssetPath(string_sprintf("%s/%s", [path UTF8String], [name UTF8String]));
    UIImage *image = [UIImage imageWithContentsOfFile:[NSString stringWithUTF8String:imagePath.c_str()]];
    return [ImageTranslation CVMatForImage:image];
}

- (void)setPath:(NSString *) path scale:(float)scale {
    //auto path = ""//convert NS to std
    __weak typeof(self) weakSelf = self;
    
    if (!self.coreFloor || !CBAugmentedView.getInstance) return;
    
    dispatch_async(dispatch_get_global_queue( DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        cv::Mat diffuseImage = [self getImage:path named:@"Base_Color.jpg"];
        cv::Mat normalImage = [self getImage:path named:@"Normal.jpg"];
        cv::Mat roughnessImage = [self getImage:path named:@"Roughness.jpg"];
        
        [CBAugmentedView.getInstance dispatch_cb:^{
            __strong typeof(self) strongSelf = weakSelf; if (!strongSelf || !strongSelf.coreFloor) return;
            //strongSelf.coreFloor->setPath([path UTF8String], scale);
            strongSelf.coreFloor->setDiffuse(diffuseImage);
            strongSelf.coreFloor->setNormalRoughness(normalImage, roughnessImage);
            strongSelf.coreFloor->setScale(scale);
        }];
    });
}

@end
