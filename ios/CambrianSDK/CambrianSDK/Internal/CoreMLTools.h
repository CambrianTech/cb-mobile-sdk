//
//  CoreMLTools.h
//  CambrianSDK
//
//  Created by Joel Teply on 1/9/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreML/CoreML.h>

NS_ASSUME_NONNULL_BEGIN

@interface CoreMLTools : NSObject

+ (int)getFeatureLength:(MLMultiArray *)features;

+ (MLMultiArray *)populateFeatures:(const cv::Mat &)image;
+ (cv::Mat)sizeImageForModel:(MLModel *)model image:(const cv::Mat &)src;

+ (int)getMaxClassForArray:(MLMultiArray *)features probability:(double *)prob;

+ (cv::Mat)getSegmentationMask:(MLMultiArray *)features atIndex:(int)index;

+ (cv::Mat)getImageOutput:(MLMultiArray *)features;

+ (cv::Mat) predictWithModel:(MLModel *)model image:(const cv::Mat &)image;

+ (cv::Mat) predictWithModel:(MLModel *)model images:(const std::map<std::string,cv::Mat> &)images;

@end

NS_ASSUME_NONNULL_END
