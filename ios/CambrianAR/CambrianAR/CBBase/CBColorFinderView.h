//
//  CBColorFinderView.h
//  CambrianAR
//
//  Created by Joel Teply on 4/17/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import <CambrianAR/CambrianAR.h>

CB_PUBLIC
@interface CBColorResult : NSObject
@property (strong, nonatomic, nonnull) UIColor *color;
@property (assign, nonatomic) CGPoint position;
@end

@protocol CBColorFinderDelegate <NSObject>
- (void) colorsFound:(nonnull NSArray<CBColorResult *>*)results;
@end

CB_PUBLIC
@interface CBColorFinderView : CBAugmentedView

@property (weak, nonatomic, nullable) id<CBColorFinderDelegate> delegate;

-(UIColor *_Nullable)getColorAtPoint:(CGPoint)point;

@end
