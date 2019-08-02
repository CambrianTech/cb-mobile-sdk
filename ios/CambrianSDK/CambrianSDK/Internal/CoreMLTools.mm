//
//  CoreMLTools.m
//  CambrianSDK
//
//  Created by Joel Teply on 1/9/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#import "CoreMLTools.h"
#import "ImageTranslation.h"

@interface CVMatFeatureProvider : NSObject<MLFeatureProvider>
    @property (nonatomic, strong) NSDictionary<NSString *, MLFeatureDescription *> *featureDescriptions;
@end

//Currently supports single image, very easy to add more
@implementation CVMatFeatureProvider {
    std::map<std::string,cv::Mat> m_images;
}

@dynamic featureNames;

//init with just one image
- (nullable instancetype)initWithModel:(MLModel *)model image:(cv::Mat)image {
    MLFeatureDescription * firstFeature = model.modelDescription.inputDescriptionsByName.allValues[0];
    std::map<std::string,cv::Mat> dict;
    std::string key = [firstFeature.name UTF8String];
    dict[key] = image;
    
    return [self initWithModel:model images:dict];
}

- (nullable instancetype)initWithModel:(MLModel *)model images:(const std::map<std::string,cv::Mat> &)images {
    for (const auto &itr : images) {
        cv::Mat imageResized = [CoreMLTools sizeImageForModel:model image:itr.second];
        m_images[itr.first] = imageResized;
    }
    
    self.featureDescriptions = model.modelDescription.inputDescriptionsByName;
    
    return self;
}

- (NSSet<NSString *> *)featureNames {
    return [NSSet setWithArray:self.featureDescriptions.allKeys];
}

/// Returns nil if the provided featureName is not in the set of featureNames
//- (nullable MLFeatureValue *)featureValueForName:(NSString *)featureName;
- (nullable MLFeatureValue *)featureValueForName:(nonnull NSString *)featureName {
    
    
    MLFeatureDescription * featureDesc = self.featureDescriptions[featureName];
    if (!featureDesc) return nil;
    
    cv::Mat image = m_images[[featureDesc.name UTF8String]];
    if (image.empty()) return nil;
    
    if (featureDesc.type == MLFeatureTypeMultiArray) {
        MLMultiArray *value = [CoreMLTools populateFeatures:image];
        return [MLFeatureValue featureValueWithMultiArray:value];
    } else {
        CVPixelBufferRef buffer = [ImageTranslation pixelBufferFromCVMat:image options:nil];
        return [MLFeatureValue featureValueWithPixelBuffer:buffer];
    }
}

@end

@implementation CoreMLTools

+ (int)getMaxClassForArray:(MLMultiArray *)features probability:(double *)prob {
    int value = 0;
    *prob = 0;
    
    for (int i=0; i<features.strides[0].intValue; i++) {
        const double * data = (const double *) features.dataPointer;
        double cur = data[i];
        if (cur > *prob) {
            *prob = cur;
            value = i;
        }
    }
    
    return value;
}

+ (int)getFeatureLength:(MLMultiArray *)features {
    return features.shape[0].intValue;
}

+ (cv::Mat)getSegmentationMask:(MLMultiArray *)features atIndex:(int)index {
    double *mlData = (double *) features.dataPointer;
    
    int width = features.shape[1].intValue;
    int height = features.shape[2].intValue;
    
    int classDataLength = width * height;
    
    long mlDataStart = classDataLength * index;
    
    cv::Mat mask = cv::Mat(height, width, CV_8UC1);
    for (int i=0; i<classDataLength; i++) {
        double value = mlData[mlDataStart + i];
        mask.data[i] = int(value * 255.0);
    }
    
    return mask;
}

+ (MLMultiArray *)populateFeatures:(const cv::Mat &)image {
    NSError *error;
    NSArray *shape = @[[NSNumber numberWithInt:image.channels()], [NSNumber numberWithInt:image.cols], [NSNumber numberWithInt:image.rows]];
    
    MLMultiArray *inputFeatures = [[MLMultiArray alloc] initWithShape:shape
                                                        dataType:MLMultiArrayDataTypeDouble
                                                           error:&error];
    double *mlData = (double *) inputFeatures.dataPointer;
    
    int width = image.cols;
    int height = image.rows;
    
    int numChannels = image.channels();
    int classDataLength = width * height;
    for (int channel=0; channel<numChannels; channel++) {
        int classDataStart = channel * classDataLength;
        for (int i=0; i<classDataLength; i++) {
            mlData[i + classDataStart] = double(image.data[i * numChannels + channel]) / 255.0;
        }
    }
    
    return inputFeatures;
}

+ (cv::Mat)getImageOutput:(MLMultiArray *)features {
    
    int numChannels = features.shape[0].intValue;
    int width = features.shape[1].intValue;
    int height = features.shape[2].intValue;
    
    cv::Mat img = cv::Mat(height, width, CV_MAKETYPE(features.dataType == MLMultiArrayDataTypeFloat32 ? CV_32F : CV_8U, numChannels));

    int classDataLength = features.strides[0].intValue;
    
    for (int channel=0; channel<numChannels; channel++) {
        int classDataStart = channel * classDataLength;
        if (features.dataType == MLMultiArrayDataTypeDouble) {
            double *mlData = (double *) features.dataPointer;
            for (int i=0; i<classDataLength; i++) {
                double value = mlData[i + classDataStart];
                img.data[i * numChannels + channel] = int(value * 255.0f);
            }
        } else if (features.dataType == MLMultiArrayDataTypeFloat32) {
            float *mlData = (float *) features.dataPointer;
            for (int i=0; i<classDataLength; i++) {
                img.data[i * numChannels + channel] = 255.0f * mlData[i + classDataStart];
            }
        } else if (features.dataType == MLMultiArrayDataTypeInt32) {
            int *mlData = (int *) features.dataPointer;
            for (int i=0; i<classDataLength; i++) {
                img.data[i * numChannels + channel] = mlData[i + classDataStart];
            }
        }
    }
    
    if (features.dataType == MLMultiArrayDataTypeFloat32) {
        img.convertTo(img, CV_MAKETYPE(CV_8U, numChannels));
    }
    
    return img;
}

+ (cv::Mat)sizeImageForModel:(MLModel *)model image:(const cv::Mat &)image {
    MLFeatureDescription *feature = model.modelDescription.inputDescriptionsByName.allValues[0];
    
    int numChannels = image.channels();
    int width = image.cols;
    int height = image.rows;
    
    if (feature.type == MLFeatureTypeMultiArray) {
        numChannels = feature.multiArrayConstraint.shape[0].intValue;
        width = feature.multiArrayConstraint.shape[1].intValue;
        height = feature.multiArrayConstraint.shape[2].intValue;
    } else {
        width = int(feature.imageConstraint.pixelsWide);
        height = int(feature.imageConstraint.pixelsHigh);
    }

    cv::Mat dest = image;
    if (image.cols != width || image.rows != height) {
        cv::resize(image, dest, cv::Size(width, height), 0,0, cv::INTER_AREA);
    }
    
    if (dest.channels() != numChannels) {
        if (dest.channels() == 3) {
            if (numChannels == 1) cv::cvtColor(dest, dest, CV_RGB2GRAY);
            if (numChannels == 4) cv::cvtColor(dest, dest, CV_RGB2RGBA);
        }
        else if (dest.channels() == 4) {
            if (numChannels == 1) cv::cvtColor(dest, dest, CV_RGBA2GRAY);
            if (numChannels == 3) cv::cvtColor(dest, dest, CV_RGBA2RGB);
        }
    }
    
    return dest;
}

+ (cv::Mat) predictWithModel:(MLModel *)model image:(const cv::Mat &)image {
    CVMatFeatureProvider * inputs = [[CVMatFeatureProvider alloc] initWithModel:model image:image];

    NSError *error;
    id<MLFeatureProvider> output = [model predictionFromFeatures:inputs error:&error];
    
    NSString * outputName = [output.featureNames allObjects][0];
    MLMultiArray *value = [output featureValueForName:outputName].multiArrayValue;
    
    return [self getImageOutput:value];
}

+ (cv::Mat) predictWithModel:(MLModel *)model images:(const std::map<std::string,cv::Mat> &)images {
    CVMatFeatureProvider * inputs = [[CVMatFeatureProvider alloc] initWithModel:model images:images];
    
    NSError *error;
    id<MLFeatureProvider> output = [model predictionFromFeatures:inputs error:&error];
    
    NSString * outputName = [output.featureNames allObjects][0];
    MLMultiArray *value = [output featureValueForName:outputName].multiArrayValue;
    
    return [self getImageOutput:value];
}

@end
