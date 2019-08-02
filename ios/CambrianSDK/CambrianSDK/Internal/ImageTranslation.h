//
//  ImageTranslation.h
//  Wall Painter
//
//  Created by Joel Teply on 8/19/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKIt/UIKit.h>
#import "CBARInternal.h"

@interface ImageTranslation : NSObject {

}

+ (cv::Point) translatePosition:(CGPoint)point
                      imageView:(UIImageView *)imageView
                          image:(const cv::Mat &)img;

+ (UIImage *)imageWithImage:(UIImage *)image scaledToSize:(CGSize)newSize;
+ (UIImage *)resizeImage:(UIImage *)image width:(int)width height:(int)height;

+ (UIColor *) UIColorForScalar:(cv::Scalar)color;
+ (cv::Scalar) scalarForUIColor:(UIColor *)color;

+ (double) getImageScale:(UIImageView *)imageView image:(const cv::Mat &)img;

+ (UIImage *)scaleAndRotateImage:(UIImage *)image;
+ (UIImage *)scaleAndRotateImage:(UIImage *)image constrainedToSize:(CGSize)maxSize;
+ (UIImage *)scaleAndRotateImage:(UIImage *)image constrainedToSize:(CGSize)maxSize withOrientation:(UIImageOrientation)orient;

+ (double) maskImage:(const cv::Mat&)img
              hsvImg:(const cv::Mat&)hsvImg
           destImage:(cv::Mat&)dest
      staticHSVImage:(const cv::Mat&)staticHSVImage
     staticThreshold:(cv::Scalar)staticThreshold
            belowHSV:(cv::Scalar)startHSV 
            aboveHSV:(cv::Scalar)endHSV;

+ (UIImage *)imageWithCVMat:(const cv::Mat&)cvMat;
+ (UIImage *)imageWithCVMat:(const cv::Mat&)cvMat isBGR:(BOOL)bgr;
+ (UIImage *)imageWithBytes:(uchar*)bytes
                  imageSize:(CGSize)imageSize
                   channels:(size_t)channels
                      isBGR:(BOOL)bgr;

+ (CVPixelBufferRef) pixelBufferFromCGImage: (CGImageRef) image options:(NSDictionary *)options;
+ (CVPixelBufferRef) pixelBufferFromCoreImage: (CIImage *) image options:(NSDictionary *)options;
+ (CVPixelBufferRef) pixelBufferFromCVMat:(const cv::Mat &) image options:(NSDictionary *)options;

+ (CIImage *)ciimageWithBytes:(uchar*)bytes
                    imageSize:(CGSize)imageSize
                     channels:(size_t)channels
                        isBGR:(BOOL)bgr;

+ (cv::Mat) CVMatForCoreImage:(CIImage *)image;
+ (CIImage *) CoreImageForCVMat:(const cv::Mat&)image isBGR:(BOOL)isBGR;

+ (cv::Mat) CVMatForImage:(UIImage *)image;

+ (cv::Rect) cvRectForCGRect:(CGRect) rect;

+ (std::vector<CGPoint>)cgPointsForCVPoints:(std::vector<cv::Point>)points;

+ (std::vector<cv::Point>)cvPointsForCGPoints:(std::vector<CGPoint>)points;

+ (void)drawPolygon:(CGContextRef)context
             points:(const std::vector<cv::Point>)points
        strokeWidth:(CGFloat)strokeWidth
        strokeColor:(CGColorRef)strokeColor
           isDashed:(BOOL)dashed
          fillColor:(CGColorRef)fillColor;

+ (void)drawPath:(CGContextRef)context points:(std::vector<cv::Point>)points;

+ (int)degreeRotationForDeviceOrientation:(UIDeviceOrientation)orientation;

+ (int)degreeRotationForInterfaceOrientation:(UIInterfaceOrientation)orientation;

+ (CGAffineTransform)imageTransformForRotation:(int)degrees
                                     imageSize:(CGSize)imageSize
                                  viewportSize:(CGSize)viewportSize
                                     cropImage:(BOOL)cropImage;

@end
