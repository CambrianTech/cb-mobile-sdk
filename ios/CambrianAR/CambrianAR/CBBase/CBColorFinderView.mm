//
//  CBColorFinderView.m
//  CambrianAR
//
//  Created by Joel Teply on 4/17/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import "CBColorFinderView.h"
#import "CBARInternal.h"
#import <imaging/ImageProcessing.h>
#import "ImageTranslation.h"

@implementation CBColorResult

@end

@implementation CBColorFinderView

- (std::shared_ptr<CBAR_View>) generateCoreView:(std::shared_ptr<CBAR_CallbackI>)callback;
{
    auto coreView = std::shared_ptr<CBAR_View>(new CBAR_View(callback));
    coreView->setToolMode(ToolModeFindColor);
    return coreView;
}

- (void) colorsCallback:(std::vector<cbar::ColorInfo>)colors {
    
    if (![self.delegate respondsToSelector:@selector(colorsFound:)]) {
        return;
    }
    
    __weak typeof(self) weakSelf = self;
    dispatch_async(dispatch_get_main_queue(), ^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf || !strongSelf.delegate) return;
        
        NSMutableArray<CBColorResult *> *colorArray = [NSMutableArray arrayWithCapacity:colors.size()];
        for(int i = 0; i < colors.size(); i++) {
            const auto &color = colors[i];
            
            CBColorResult *result = [CBColorResult new];
            result.color = [ImageTranslation UIColorForScalar: color.color];
            result.position = CGPointMake(color.normalizedPosition.x * self.frame.size.width,
                                          color.normalizedPosition.y * self.frame.size.height);
            
            colorArray[i] = result;
        }
        
        [strongSelf.delegate colorsFound: colorArray];
    });
}

- (UIColor *)getColorAtPoint:(CGPoint)point {
    __block cv::Scalar color;
    __block CGRect frame;
    
    __weak typeof(self) weakSelf = self;
    
    if ([NSThread isMainThread]) {
        frame = self.frame;
    } else {
        dispatch_sync(dispatch_get_main_queue(), ^{
            frame = weakSelf.frame;
        });
    }
    
    cv::Point2f normalizedPoint = cv::Point2f((point.x) / frame.size.width,
                                              (point.y) / frame.size.height);
    
    
    [self dispatch_cb_get_result:^{
        __strong typeof(self) strongSelf = weakSelf; if (!strongSelf) return;
        color = strongSelf.coreView->getColorInVideoAtPoint(normalizedPoint);
    }];
    
    return [ImageTranslation UIColorForScalar:color];
}

@end
