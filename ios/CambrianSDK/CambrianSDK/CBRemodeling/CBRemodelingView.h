//
//  CBRemodelingView.h
//  CambrianAR
//
//  Created by Joel Teply on 3/3/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CBAugmentedView.h"
#import "CBRemodelingTypes.h"
#import "CBRemodelingScene.h"

NS_ASSUME_NONNULL_BEGIN

@protocol CBRemodelingViewDelegate <CBAugmentedViewViewDelegate>

@optional

- (void) historyChanged:(CBAugmentedAsset *)asset
                 change:(CBUndoChange)change
                forward:(BOOL)forward;

@end

CB_PUBLIC
@interface CBRemodelingView : CBAugmentedView

@property (weak, nonatomic, nullable) id<CBRemodelingViewDelegate> delegate;

@property (nonatomic, strong, nonnull) CBRemodelingScene *scene;


@end

NS_ASSUME_NONNULL_END
