//
//  VideoTranslation.h
//  HomeAugmentationFramework
//
//  Created by Joel Teply on 6/9/14.
//  Copyright (c) 2014 Joel Teply. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKIt/UIKit.h>
#import <AVFoundation/AVFoundation.h>

@interface VideoTranslation : NSObject {
    
}

+ (UIImage *) UIImageForBuffer:(CMSampleBufferRef)sampleBuffer;

@end