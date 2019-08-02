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
    std::shared_ptr<cbscene::CBAR_Paint> m_corePaint;
    std::shared_ptr<CBARMaskedAssetCallback> m_maskedCallback;
}

@property (readonly, nonatomic) std::shared_ptr<cbscene::CBAR_Paint> corePaint;

@end

@implementation CBRemodelingPaint

@dynamic corePaint;
@dynamic color;
@dynamic sheen;
@dynamic transparency;

- (CBAssetType)assetType {
    return CBAssetTypePaint;
}

- (void)releaseCore {
    m_corePaint.reset();
    m_maskedCallback.reset();
}

- (std::shared_ptr<cbscene::CBAR_Paint>) corePaint {
    @synchronized(self) {
        if (!m_corePaint) {
            m_maskedCallback = std::shared_ptr<CBARMaskedAssetCallback>(new CBARMaskedAssetCallback(self));
            m_corePaint = std::shared_ptr<cbscene::CBAR_Paint>(new cbscene::CBAR_Paint([self getPassedAssetID], m_maskedCallback));
        }
    }
    return m_corePaint;
}

- (std::shared_ptr<cbscene::CBAR_Asset>)coreAsset {
    return std::dynamic_pointer_cast<cbscene::CBAR_Asset>(self.corePaint);
}

- (std::shared_ptr<CBARAssetCallback>) coreCallback {
    return std::dynamic_pointer_cast<CBARAssetCallback>(self.maskedCallback);
}

- (std::shared_ptr<CBARMaskedAssetCallback>) maskedCallback {
    return m_maskedCallback;
}

- (UIColor *)color {
    auto cvColor = self.corePaint->getColor();
    return [ImageTranslation UIColorForScalar:cvColor];
}

- (void)setColor:(UIColor *)color {
    auto cvColor = [ImageTranslation scalarForUIColor:color];
    self.corePaint->setColor(cvColor);
}

- (CBPaintSheen)sheen {
    Sheen coreSheen = self.corePaint->getSheen();
    return (CBPaintSheen) coreSheen;
}

- (void)setSheen:(CBPaintSheen)sheen {
    self.corePaint->setSheen((Sheen)sheen);
}

- (CBTransparency)transparency {
    Transparency coreSheen = self.corePaint->getTransparency();
    return (CBTransparency) coreSheen;
}

- (void)setTransparency:(CBTransparency)transparency {
    self.corePaint->setTransparency((Transparency)transparency);
}

@end
