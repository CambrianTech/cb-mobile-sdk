//
//  CBRemodelingScene.m
//  CambrianAR
//
//  Created by Joel Teply on 3/3/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import "CBRemodelingScene.h"
#import "CBARInternal.h"
#include <utility/Threads.h>

@interface CBRemodelingScene()
@property (readonly, nonatomic) std::shared_ptr<cbscene::CBAR_RemodelingScene> coreRemodelingScene;
@end

@implementation CBRemodelingScene {
    CBMutex assetMutex;
}

@dynamic coreRemodelingScene;
@dynamic isMasked;

- (void) generateCoreScene:(NSString *)sceneID {
    self.coreScene = std::shared_ptr<cbscene::CBAR_RemodelingScene>(new cbscene::CBAR_RemodelingScene(self.genAssetFunction, [sceneID UTF8String], (__bridge void *)self));
}

- (void) generateCoreScene:(NSString *)sceneID rgbaImage:(cv::Mat)rgbaImage {
    self.coreScene = std::shared_ptr<cbscene::CBAR_RemodelingScene>(new cbscene::CBAR_RemodelingScene(self.genAssetFunction, rgbaImage, [sceneID UTF8String], (__bridge void *)self));
}

- (std::shared_ptr<cbscene::CBAR_RemodelingScene>) coreRemodelingScene{
    return std::dynamic_pointer_cast<cbscene::CBAR_RemodelingScene>(self.coreScene);
}

- (CBRemodelingPaint *) generateSetSelected:(Class)aClass {
    //find first
    __block CBAugmentedAsset *firstObject = nil;
    [self.assets enumerateKeysAndObjectsUsingBlock:^(NSString * key, CBAugmentedAsset *value, BOOL* stop) {
        if ([value isKindOfClass:aClass]) {
            firstObject = value;
            *stop = YES;
        }
    }];
    
    return (CBRemodelingPaint *) self.selectedAsset;
}

- (BOOL) isMasked {
    return self.coreRemodelingScene->isMasked();
}

@end
