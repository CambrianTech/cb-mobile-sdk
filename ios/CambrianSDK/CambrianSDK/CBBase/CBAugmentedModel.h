//
//  CBRemodelingPaint.h
//  CambrianAR
//
//  Created by Joel Teply on 3/3/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "CBAugmentedAsset.h"

NS_ASSUME_NONNULL_BEGIN

CB_PUBLIC
@interface CBAugmentedModel : CBAugmentedAsset

@property (assign, nonatomic) BOOL isEditingPosition;

- (void)setPath:(NSString *) path scale:(float)scale;

+ (BOOL)canCreate;

@end

NS_ASSUME_NONNULL_END
