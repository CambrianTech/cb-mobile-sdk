//
//  CBRemodelingPaint.h
//  CambrianAR
//
//  Created by Joel Teply on 3/3/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "CBMaskedAsset.h"
#import "CBRemodelingTypes.h"

NS_ASSUME_NONNULL_BEGIN

CB_PUBLIC
@interface CBRemodelingPaint : CBMaskedAsset

@property (nonatomic, strong, nonnull) UIColor* color;
@property (nonatomic, assign) CBTransparency transparency;
@property (nonatomic, assign) CBPaintSheen sheen;

@end

NS_ASSUME_NONNULL_END
