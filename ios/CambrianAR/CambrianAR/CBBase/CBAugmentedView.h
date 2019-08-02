//
//  CBAugmentedView.h
//  CambrianAR
//
//  Created by Joel Teply on 11/17/15.
//  Copyright © 2015 Joel Teply. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#import "CBAugmentedScene.h"

@class CBTexture;
@class CBAugmentedAsset;

NS_ASSUME_NONNULL_BEGIN

@protocol CBAugmentedViewViewDelegate <NSObject>

@optional

- (void) assetLongPressed:(CBAugmentedAsset *)asset;
- (void) assetTapped:(CBAugmentedAsset *)asset;

@end

CB_PUBLIC
@interface CBAugmentedView : UIView

@property (nonatomic, strong, nullable) CBAugmentedScene *scene;

@property (nonatomic, assign) CBToolMode toolMode;

@property (nonatomic, readonly) BOOL isLive;

- (void) captureCurrentState;

- (BOOL) startRunning;
- (BOOL) stopRunning;

- (void) clearAll;

- (void) pause:(BOOL)pause;

@property (nonatomic, readonly) int undoSize;
@property (nonatomic, assign) int maxUndoSize;
- (void) undo;

@property (nonatomic, readonly) int redoSize;
@property (nonatomic, assign) int maxRedoSize;
- (void) redo;

- (CBAugmentedAsset *)assetAtPoint:(CGPoint)point;

@end

NS_ASSUME_NONNULL_END
