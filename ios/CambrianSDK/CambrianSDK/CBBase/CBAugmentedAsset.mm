//
//  CBAugmentedAsset.m
//  CambrianAR
//
//  Created by Joel Teply on 3/3/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import "CBAugmentedAsset.h"
#import "CBARInternal.h"
#import "CBAugmentedScene.h"
#include <cbar/scene/scene.h>

@implementation CBAugmentedAsset {
    __weak CBAugmentedScene* _scene;
    NSString * _assetID;
}

@dynamic scene;
@dynamic assetType;
@dynamic assetID;
@dynamic isUtilized;
@dynamic isSelected;
@dynamic coreAsset;
@dynamic coreCallback;

- (void)dealloc
{
    NSLog(@"Deallocating %@", NSStringFromClass([self class]));
}

- (nullable CBAugmentedAsset *) init {
    if (![[self class] canCreate]) {
        return nil;
    }
    
    if (self = [super init]) {
        _assetID = [[NSUUID UUID] UUIDString];
    }
    return self;
}

- (CBAssetType)assetType {
    return CBAssetTypeModel;
}

- (NSString *)assetID {
    return [NSString stringWithUTF8String:self.coreAsset->getAssetID().c_str()];
}

- (nullable NSString *) getUserData:(NSString *)key {
    return [NSString stringWithUTF8String:self.coreAsset->userData[key.UTF8String].c_str()];
}

- (void) setUserData:(NSString *)key value:(NSString *)value {
    self.coreAsset->userData[key.UTF8String] = value.UTF8String;
}

- (void) touchedGroundPlaneAt:(ARHitTestResult *)hitResult scene:(ARSCNView *)scene API_AVAILABLE(ios(11.0)) {
    
}

- (CBAugmentedScene *)scene {
    return _scene;
}

- (BOOL) isUtilized {
    return self.coreAsset->isUtilized();
}

- (BOOL)isSelected {
    return [self.scene.selectedAssetID isEqualToString:self.assetID];
}

- (void) setIsSelected:(BOOL)value {
    if (value) {
        self.scene.selectedAsset = self;
    }
}

- (void) attachedToScene:(CBAugmentedScene *)scene {
    _scene = scene;
    [self assetChanged];
}

- (void) detachedFromScene:(CBAugmentedScene *)scene {
    
}

- (void) assetChanged {
    
}

- (void) rotatedAtPoint:(CGPoint)point step:(TouchStep)step rotation:(float)amount {
    
}

- (void) touchedAtPoint:(CGPoint)point step:(TouchStep)step {
    
}

+ (BOOL)canCreate {
    return YES;
}



@end

void CBARAssetCallback::attachedToScene() {
    @autoreleasepool {
        CBAugmentedScene *scene = [CBAugmentedView getInstance].scene;
        [getAsset() attachedToScene:scene];
    }
}

void CBARAssetCallback::detachedFromScene() {
    @autoreleasepool {
        CBAugmentedScene *scene = nil;
        [getAsset() detachedFromScene:scene];
    }
}

void CBARAssetCallback::_touchesCommon(const cbpipe::TouchPoint &touch, CBAugmentedAsset **asset, CGPoint *touchPoint) {
    double scale = [CBAugmentedView getInstance].contentScaleFactor;
    
    *touchPoint = CGPointMake(touch.imageOrigin.x * touch.srcSize.width / scale,
                              touch.imageOrigin.y * touch.srcSize.height / scale);
    
    //switch assets, if new one touched
    *asset = getAsset();
    if (touch.step == TouchStepBegan) {
        CBAugmentedAsset *otherAsset = [[CBAugmentedView getInstance] assetAtPoint:*touchPoint];
        if (otherAsset != *asset) {
            (*asset).isSelected = NO;
            otherAsset.isSelected = YES;
            
        }
        *asset = otherAsset;//nil maybe
    } else if (touch.step == TouchStepEnded) {
        if ([*asset isKindOfClass:[CBAugmentedModel class]]) {
            CBAugmentedModel *model = (CBAugmentedModel *)*asset;
            model.isEditingPosition = NO;
        }
    }
}

void CBARAssetCallback::touchedAtPoint(cbpipe::TouchPoint touch) {
    @autoreleasepool {
        dispatch_async(dispatch_get_main_queue(), ^{
            CBAugmentedAsset *asset;
            CGPoint touchPoint;
            _touchesCommon(touch, &asset, &touchPoint);
            
            [asset touchedAtPoint:touchPoint step:touch.step];
        });
    }
}

void CBARAssetCallback::rotatedBy(cbpipe::TouchPoint touch) {
    @autoreleasepool {
        dispatch_async(dispatch_get_main_queue(), ^{
            CBAugmentedAsset *asset;
            CGPoint touchPoint;
            _touchesCommon(touch, &asset, &touchPoint);
            
            [asset rotatedAtPoint:touchPoint step:touch.step rotation:touch.rotation];
        });
    }
}

