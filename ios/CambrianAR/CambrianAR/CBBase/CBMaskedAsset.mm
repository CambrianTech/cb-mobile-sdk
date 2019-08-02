//
//  CBARMaskedAsset.m
//  CambrianSDK
//
//  Created by Joel Teply on 12/10/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import "CBMaskedAsset.h"
#import "CBARInternal.h"

@implementation CBMaskedAsset {
    std::shared_ptr<CBARMaskedAssetCallback> m_maskedCallback;
}

@end

void CBARMaskedAssetCallback::attachedToScene() {
    CBARAssetCallback::attachedToScene();
}

void CBARMaskedAssetCallback::detachedFromScene() {
    CBARAssetCallback::detachedFromScene();
}

void CBARMaskedAssetCallback::maskChanged(const cv::Mat &mask) {
    CBLog("Mask changed!");
}

