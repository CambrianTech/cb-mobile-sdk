//
//  CBAugmentedView.h
//  CambrianAR
//
//  Created by Joel Teply on 12/4/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "CBAugmentedScene.h"
#import "CBAugmentedTypes.h"

NS_ASSUME_NONNULL_BEGIN

@class CBAugmentedScene;
@class CBAugmentedAsset;

@protocol CBAugmentedViewViewDelegate <NSObject>

@optional

- (void) assetLongPressed:(CBAugmentedAsset *)asset;
- (void) assetTapped:(CBAugmentedAsset *)asset;

- (void) uploadDiagnosticImageAtPath:(NSString *)path;

@end

CB_PUBLIC
@interface CBAugmentedView : UIView

@property (nonatomic, strong, nullable) CBAugmentedScene *scene;
@property (nonatomic, readonly) BOOL isLiveMode;
@property (nonatomic, readonly) BOOL isAugmentedReality;
@property (nonatomic, assign) CBToolMode toolMode;

- (void)startRunning;
- (void)startRunning:(BOOL)isVideo unityScene:(NSString *_Nullable)unityScene;

- (void)stopRunning;
- (void)captureToStill;

- (void)sendUnityCommand:(NSString *)commandName json:(NSString *)jsonString;

NS_ASSUME_NONNULL_END

@end
