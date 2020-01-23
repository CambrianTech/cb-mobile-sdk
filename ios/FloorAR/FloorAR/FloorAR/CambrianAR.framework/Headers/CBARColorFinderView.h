//
//  CBColorFinderView.h
//  CambrianAR
//
//  Created by Joel Teply on 4/17/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import <CambrianAR/CambrianAR.h>

@interface CBARColorResult : NSObject
@property (strong, nonatomic, nonnull) UIColor *color;
@property (assign, nonatomic) CGPoint position;
@end

@protocol CBARColorFinderDelegate <NSObject>
- (void) colorsFound:(nonnull NSArray<CBARColorResult *>*)results;
@end

@interface CBColorFinderView : CBARAugmentedView

@property (weak, nonatomic, nullable) id<CBARColorFinderDelegate> delegate;

-(UIColor *_Nullable)getColorAtPoint:(CGPoint)point;

@end
