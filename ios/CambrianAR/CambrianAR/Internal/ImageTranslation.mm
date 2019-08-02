//
//  ImageTranslation.m
//  Wall Painter
//
//  Created by Joel Teply on 8/19/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "ImageTranslation.h"
#import <imaging/Drawing.h>
#import <imaging/Coloring.h>
#import <imaging/Imaging.h>

#define MIN_UNMASKED_VAL 70.0f

@interface ImageTranslation()

//+ (BOOL)color:(UIColor*)color 
//       getHue:(CGFloat *)hue 
//   saturation:(CGFloat *)saturation 
//   brightness:(CGFloat *)value 
//        alpha:(CGFloat *)alpha;

@end


@implementation ImageTranslation

#pragma mark -
#pragma mark OpenCV Support Methods

+ (double) getImageScale:(UIImageView *)imageView image:(const cv::Mat &)img {
	double imageAspectRatio = (double) img.cols / (double) img.rows;
	double frameAspectRatio = (double) imageView.frame.size.width / (double) imageView.frame.size.height;
	
	if (imageAspectRatio > frameAspectRatio) {
		return (double) img.cols / (double) imageView.frame.size.width;
	}
	else {
		return (double) img.rows / (double) imageView.frame.size.height;
	}
}

+ (cv::Point) translatePosition:(CGPoint)point
					imageView:(UIImageView *)imageView 
						image:(const cv::Mat &)img
{
	double xBarSize = 0;
	double yBarSize = 0;
	double imageAspectRatio = (double) img.cols / (double) img.rows;
	double frameAspectRatio = (double) imageView.frame.size.width / (double) imageView.frame.size.height;
	
	double scale = [self getImageScale:imageView image:img];
	CvPoint seedPoint;
	
	if (imageAspectRatio > frameAspectRatio) {
		yBarSize = round((scale * (double) imageView.frame.size.height - (double) img.rows) / 2.0);
	} 
	else {
		xBarSize = round((scale * (double) imageView.frame.size.width - (double) img.cols) / 2.0);
	}
	
	seedPoint.x = (int) round((scale * (float) point.x) - xBarSize);
	seedPoint.y = (int) round((scale * (double) point.y) - yBarSize);
	
	return seedPoint;
}

+ (UIImage *)scaleAndRotateImage:(UIImage *)image;
{
    return [self scaleAndRotateImage:image constrainedToSize:CGSizeMake(1024,1024)];
}

+ (UIImage *)scaleAndRotateImage:(UIImage *)image constrainedToSize:(CGSize)maxSize;
{
    UIImageOrientation orient = image.imageOrientation;
    
    return [self scaleAndRotateImage:image constrainedToSize:maxSize withOrientation:orient];
}

#ifndef MIN
#  define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

+ (UIImage *)scaleAndRotateImage:(UIImage *)image constrainedToSize:(CGSize)max withOrientation:(UIImageOrientation)orient;
{
	CGImageRef imgRef = image.CGImage;
	CGFloat width = CGImageGetWidth(imgRef);
	CGFloat height = CGImageGetHeight(imgRef);
    
    CGSize maxSize = max;
    
    if (!maxSize.width) {
        maxSize = CGSizeMake(image.size.width, image.size.height);
    }
    
    if (orient == UIImageOrientationLeft || orient == UIImageOrientationRight
        || orient == UIImageOrientationLeftMirrored || orient == UIImageOrientationRightMirrored) {
        maxSize.width = max.height;
        maxSize.height = max.width;
    }
	
	CGAffineTransform transform = CGAffineTransformIdentity;
	CGRect bounds = CGRectMake(0, 0, width, height);
	if (width > maxSize.width || height > maxSize.height) {
		CGFloat ratio = width/height;
		if (ratio > 1) {
            bounds.size.width = maxSize.width;
			bounds.size.height = bounds.size.width / ratio;
		} else {
			bounds.size.height = maxSize.height;
			bounds.size.width = bounds.size.height * ratio;
		}
	}
    
    if (bounds.size.width > width) {
        bounds.size.height = bounds.size.height * (width / bounds.size.width);
        bounds.size.width = width;
    }
    
    if (bounds.size.height > height) {
        bounds.size.width = bounds.size.width * (height / bounds.size.height);
        bounds.size.height = height;
    }
	
	CGFloat scaleRatio = bounds.size.width / width;
	CGSize imageSize = CGSizeMake(CGImageGetWidth(imgRef), CGImageGetHeight(imgRef));
	CGFloat boundHeight;
	
	switch(orient) {
		case UIImageOrientationUp:
			transform = CGAffineTransformIdentity;
			break;
		case UIImageOrientationUpMirrored:
			transform = CGAffineTransformMakeTranslation(imageSize.width, 0.0);
			transform = CGAffineTransformScale(transform, -1.0, 1.0);
			break;
		case UIImageOrientationDown:
			transform = CGAffineTransformMakeTranslation(imageSize.width, imageSize.height);
			transform = CGAffineTransformRotate(transform, M_PI);
			break;
		case UIImageOrientationDownMirrored:
			transform = CGAffineTransformMakeTranslation(0.0, imageSize.height);
			transform = CGAffineTransformScale(transform, 1.0, -1.0);
			break;
		case UIImageOrientationLeftMirrored:
			boundHeight = bounds.size.height;
			bounds.size.height = bounds.size.width;
			bounds.size.width = boundHeight;
			transform = CGAffineTransformMakeTranslation(imageSize.height, imageSize.width);
			transform = CGAffineTransformScale(transform, -1.0, 1.0);
			transform = CGAffineTransformRotate(transform, 3.0 * M_PI / 2.0);
			break;
		case UIImageOrientationLeft:
			boundHeight = bounds.size.height;
			bounds.size.height = bounds.size.width;
			bounds.size.width = boundHeight;
			transform = CGAffineTransformMakeTranslation(0.0, imageSize.width);
			transform = CGAffineTransformRotate(transform, 3.0 * M_PI / 2.0);
			break;
		case UIImageOrientationRightMirrored:
			boundHeight = bounds.size.height;
			bounds.size.height = bounds.size.width;
			bounds.size.width = boundHeight;
			transform = CGAffineTransformMakeScale(-1.0, 1.0);
			transform = CGAffineTransformRotate(transform, M_PI / 2.0);
			break;
		case UIImageOrientationRight:
			boundHeight = bounds.size.height;
			bounds.size.height = bounds.size.width;
			bounds.size.width = boundHeight;
			transform = CGAffineTransformMakeTranslation(imageSize.height, 0.0);
			transform = CGAffineTransformRotate(transform, M_PI / 2.0);
			break;
		default:
			[NSException raise:NSInternalInconsistencyException format:@"Invalid image orientation"];
	}
	
	UIGraphicsBeginImageContext(bounds.size);
	CGContextRef context = UIGraphicsGetCurrentContext();
	if (orient == UIImageOrientationRight || orient == UIImageOrientationLeft) {
		CGContextScaleCTM(context, -scaleRatio, scaleRatio);
		CGContextTranslateCTM(context, -height, 0);
	} else {
		CGContextScaleCTM(context, scaleRatio, -scaleRatio);
		CGContextTranslateCTM(context, 0, -height);
	}
	CGContextConcatCTM(context, transform);
	CGContextDrawImage(UIGraphicsGetCurrentContext(), CGRectMake(0, 0, width, height), imgRef);
	UIImage *imageCopy = UIGraphicsGetImageFromCurrentImageContext();
	UIGraphicsEndImageContext();
	
	return imageCopy;
}

+ (UIColor *) UIColorForScalar:(cv::Scalar)color {
	return [UIColor colorWithRed:(double)color.val[0]/255.0f green:(double)color.val[1]/255.0f blue:(double)color.val[2]/255.0f alpha:color.val[3]/255.0f];
}

+ (cv::Scalar) scalarForUIColor:(UIColor *)color {
	cv::Scalar scalar;
	
    if (color) {
        if (color == [UIColor whiteColor]) {
            return cv::Scalar(255, 255, 255, 255);
        }
        else if (color == [UIColor clearColor]) {
            return cv::Scalar(0, 0, 0, 0);
        }
        else if (color == [UIColor blackColor]) {
            return cv::Scalar(0, 0, 0, 255);
        }
        
        const CGFloat *colors = CGColorGetComponents(color.CGColor);
        
        scalar.val[0] = 255.0f * colors[0]; 
        scalar.val[1] = 255.0f * colors[1]; 
        scalar.val[2] = 255.0f * colors[2]; 
        
        scalar.val[3] = 255.0f * colors[3];
    }
	
	return scalar;
}

+ (UIImage *)imageWithImage:(UIImage *)image scaledToSize:(CGSize)newSize {
    if (!newSize.width) {
        return image;
    }
    UIGraphicsBeginImageContext(newSize);
    [image drawInRect:CGRectMake(0, 0, newSize.width, newSize.height)];
    UIImage *newImage = UIGraphicsGetImageFromCurrentImageContext();    
    UIGraphicsEndImageContext();
    return newImage;
}

+ (UIImage *)resizeImage:(UIImage *)image width:(int)width height:(int)height;
{
	
	CGImageRef imageRef = [image CGImage];
	CGImageAlphaInfo alphaInfo = CGImageGetAlphaInfo(imageRef);
	CGColorSpaceRef colorSpaceInfo = CGImageGetColorSpace(imageRef);
	
	if (alphaInfo == kCGImageAlphaNone)
        alphaInfo = kCGImageAlphaNoneSkipLast;
	
	CGContextRef bitmap = CGBitmapContextCreate(NULL, width, height, CGImageGetBitsPerComponent(imageRef), CGImageGetBytesPerRow(imageRef), colorSpaceInfo, alphaInfo);
	CGContextDrawImage(bitmap, CGRectMake(0, 0, width, height), imageRef);
	CGImageRef ref = CGBitmapContextCreateImage(bitmap);
	UIImage *result = [UIImage imageWithCGImage:ref];
	
	CGContextRelease(bitmap);
	CGImageRelease(ref);
	
	return result;	
}

+ (double) maskImage:(const cv::Mat &)img
           destImage:(cv::Mat &)dest
            belowHSV:(cv::Scalar)startHSV
            aboveHSV:(cv::Scalar)endHSV;
{
    //hue 0-180
    //sat 0-255
    //val 0-255
    
    //return;
    int numMasks = 0;
    
    cv::Mat hsvImg;
    cv::cvtColor(img, hsvImg, cv::COLOR_RGB2HSV_FULL);
    
    for (int x=0; x<img.cols; x++) {
        for (int y=0; y<img.rows; y++) {
            cv::Point position = cv::Point(x, y);
            cv::Scalar currentHsv = hsvImg.at<cv::Scalar>(position);
            if (currentHsv.val[0] < startHSV.val[0] || currentHsv.val[0] > endHSV.val[0] ||
                currentHsv.val[1] < startHSV.val[1] || currentHsv.val[1] > endHSV.val[1] ||
                currentHsv.val[2] < startHSV.val[2] || currentHsv.val[2] > endHSV.val[2]) {
                //mask
                dest.at<cv::Vec3b>(position)[0] = 0;
                dest.at<cv::Vec3b>(position)[1] = 0;
                dest.at<cv::Vec3b>(position)[2] = 0;
                //(Image.data + Image.step *i)[j] = ncolor;
                numMasks ++;
            }
        }
    }
    
    double totalPoints = (double) img.cols * (double) img.rows;
    
    return ((double) numMasks) / totalPoints;
}

+ (double) maskImage:(const cv::Mat&)img
              hsvImg:(const cv::Mat&)hsvImg
           destImage:(cv::Mat&)dest
      staticHSVImage:(const cv::Mat&)staticHSVImage
     staticThreshold:(cv::Scalar)staticThreshold
            belowHSV:(cv::Scalar)startHSV
            aboveHSV:(cv::Scalar)endHSV;
{
    //hue 0-180
    //sat 0-255
    //val 0-255
    int numMasks = 0;
    //int destChannels = dest.channels();
    
    for (int y=0; y<img.rows; y++) {
        //uchar* imgRow = ROW_PTR(img, y);
        uchar* hsvRow = ROW_PTR(hsvImg, y);
        uchar* staticHsvRow = ROW_PTR(staticHSVImage, y);
        //uchar* destRow = ROW_PTR(dest, y);
    for (int x=0; x<img.cols; x++) {
        
            CvPoint position = cvPoint(x, y);
            CvScalar currentHsv = getPixel3(x, hsvRow);
            
            BOOL applyMask = false;
            
            if (staticHSVImage.rows) {
                CvScalar staticHsv = getPixel3(x, staticHsvRow);
                double distance = imaging::Coloring::euclideanDistanceSq(currentHsv, staticHsv, YES);
                applyMask = applyMask || fabs(currentHsv.val[0] - staticHsv.val[0]) < staticThreshold.val[0]
                || fabs(currentHsv.val[1] - staticHsv.val[1]) < staticThreshold.val[1]
                || fabs(currentHsv.val[2] - staticHsv.val[2]) < staticThreshold.val[2]
                || distance < staticThreshold.val[3];
            }
            
            applyMask = applyMask || currentHsv.val[0] < startHSV.val[0] || currentHsv.val[0] > endHSV.val[0]
            || currentHsv.val[1] < startHSV.val[1] || currentHsv.val[1] > endHSV.val[1]
            || currentHsv.val[2] < startHSV.val[2] || currentHsv.val[2] > endHSV.val[2];
            
            if (applyMask) {
                //mask
                imaging::Drawing::setPixelAt(position, dest, cv::Scalar::all(0));
                numMasks ++;
            } else if (currentHsv.val[2] < MIN_UNMASKED_VAL) {
                currentHsv.val[2] = MIN_UNMASKED_VAL;
                cv::Scalar color = imaging::Coloring::hsvToRGB(currentHsv);
                imaging::Drawing::setPixelAt(position, dest, color);
            }
        }
    }
    
    double totalPoints = (double) img.cols * (double) img.rows;
    
    return ((double) numMasks) / totalPoints;
}

+ (UIImage *)imageWithCVMat:(const cv::Mat&)cvMat;
{
    return [self imageWithCVMat:cvMat isBGR:NO];
}

+ (UIImage *)imageWithCVMat:(const cv::Mat&)cvMat isBGR:(BOOL)bgr;
{
    return [self imageWithBytes:cvMat.data
                      imageSize:CGSizeMake(cvMat.cols, cvMat.rows)
                       channels:cvMat.channels()
                          isBGR:bgr];
}

+ (CIImage *)ciimageWithBytes:(uchar*)bytes
                    imageSize:(CGSize)imageSize
                     channels:(size_t)channels
                        isBGR:(BOOL)bgr;
{
    
    @autoreleasepool {
        size_t totalPixels = imageSize.width * imageSize.height * 4;
        NSData *data = [NSData dataWithBytes:bytes length:totalPixels * channels];

        size_t bytesPerRow = channels * imageSize.width;
        
        CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
        CIFormat format = bgr ? kCIFormatBGRA8 : kCIFormatRGBA8;
        
        CIImage *ciimage = [[CIImage alloc] initWithBitmapData:data
                                                   bytesPerRow:bytesPerRow
                                                          size:imageSize
                                                        format:format
                                                    colorSpace:colorSpace];
        
        CGColorSpaceRelease(colorSpace);

        return ciimage;
    }
}

+ (cv::Mat) CVMatForCoreImage:(CIImage *)ciImage {
    
    @autoreleasepool {
        CIContext *ciContext = [CIContext contextWithCGContext:UIGraphicsGetCurrentContext() options:nil];
        CGImageRef img = [ciContext createCGImage:ciImage fromRect:[ciImage extent]];
        auto result = [self CVMatForCGImage:img];
        CGImageRelease(img);
        return result;
    }
}

+ (UIImage *)imageWithBytes:(uchar*)bytes
                  imageSize:(CGSize)imageSize
                   channels:(size_t)channels
                      isBGR:(BOOL)bgr;
{
    size_t totalPixels = imageSize.width * imageSize.height;
    NSData *data = [NSData dataWithBytes:bytes length:totalPixels * channels];
    size_t bytesPerRow = channels * imageSize.width;
    
    CGColorSpaceRef colorSpace;
    int alphaSetting = kCGImageAlphaNone;
    int byteOrderSetting = kCGBitmapByteOrderDefault;
    
    if (channels == 1)
    {
        colorSpace = CGColorSpaceCreateDeviceGray();
    }
    else
    {
        colorSpace = CGColorSpaceCreateDeviceRGB();
        if (channels == 4) {
            if (bgr) {
                alphaSetting = kCGImageAlphaPremultipliedFirst;
                byteOrderSetting = kCGBitmapByteOrder32Little;
            } else {
                alphaSetting = kCGImageAlphaPremultipliedLast;
            }
        }
    }
    
    CGDataProviderRef provider = CGDataProviderCreateWithCFData((__bridge CFDataRef)data);
    
    CGImageRef imageRef = CGImageCreate(imageSize.width,                                     // Width
                                        imageSize.height,                                     // Height
                                        8,                                              // Bits per component
                                        8 * channels,                                   // Bits per pixel
                                        bytesPerRow,                                  // Bytes per row
                                        colorSpace,                                     // Colorspace
                                        alphaSetting | byteOrderSetting,                // Bitmap info flags
                                        provider,                                       // CGDataProviderRef
                                        NULL,                                           // Decode
                                        false,                                          // Should interpolate
                                        kCGRenderingIntentDefault);                     // Intent
    
    UIImage *image = [[UIImage alloc] initWithCGImage:imageRef];
    CGImageRelease(imageRef);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(colorSpace);
    
    return image;
}

+ (cv::Mat)CVMatForImage:(UIImage *)image;
{
    return [self CVMatForCGImage:image.CGImage];
}

+ (cv::Mat)CVMatForCGImage:(CGImageRef)image;
{
    
    bool isGreyScale = CGImageGetBitsPerPixel(image) == 8;
    
    CGColorSpaceRef colorSpace = isGreyScale ? CGColorSpaceCreateDeviceGray() : CGImageGetColorSpace(image);
    CGFloat cols = CGImageGetWidth(image);
    CGFloat rows = CGImageGetHeight(image);

    cv::Mat cvMat = cv::Mat::zeros(rows, cols, isGreyScale ? CV_8UC1 : CV_8UC4); // 8 bits per component, 4 channels
    
    CGContextRef contextRef = CGBitmapContextCreate(cvMat.data,                 // Pointer to backing data
                                                    cols,                      // Width of bitmap
                                                    rows,                     // Height of bitmap
                                                    8,                          // Bits per component
                                                    cvMat.step[0],              // Bytes per row
                                                    colorSpace,                 // Colorspace
                                                    isGreyScale ? kCGImageAlphaNone : (kCGImageAlphaPremultipliedLast | kCGBitmapByteOrderDefault)); // Bitmap info flags
    
    CGContextDrawImage(contextRef, CGRectMake(0, 0, cols, rows), image);
    CGContextRelease(contextRef);
    
    return cvMat;
}

+ (std::vector<CGPoint>)cgPointsForCVPoints:(std::vector<cv::Point>)points;
{
    std::vector<CGPoint> outPoints;
    long inPointsSize = points.size();
    for (int i=0; i<inPointsSize; i++) {
        cv::Point2f inPoint = points[i];
        outPoints.push_back(CGPointMake(inPoint.x, inPoint.y));
    }
    return outPoints;
}

+ (std::vector<cv::Point>)cvPointsForCGPoints:(std::vector<CGPoint>)points;
{
    std::vector<cv::Point> outPoints;
    long inPointsSize = points.size();
    for (int i=0; i<inPointsSize; i++) {
        CGPoint inPoint = points[i];
        outPoints.push_back(cv::Point(inPoint.x, inPoint.y));
    }
    return outPoints;
}

+ (CVPixelBufferRef) pixelBufferFromCGImage: (CGImageRef) image options:(NSDictionary *)options
{
    CIImage *ciImage = [CIImage imageWithCGImage:image];
    
    return [self pixelBufferFromCoreImage:ciImage options:options];
}

+ (CVPixelBufferRef) pixelBufferFromCoreImage: (CIImage *) image options:(NSDictionary *)options {
    CVPixelBufferRef pxbuffer = NULL;
    
    size_t width = image.extent.size.width;
    size_t height = image.extent.size.height;
    CVReturn status = CVPixelBufferCreate(kCFAllocatorDefault, width, height,
                                          kCVPixelFormatType_32ARGB, (__bridge CFDictionaryRef) options,
                                          &pxbuffer);
    
    NSParameterAssert(status == kCVReturnSuccess && pxbuffer != NULL);
    
    @autoreleasepool {
        CIContext *ciContext = [CIContext contextWithCGContext:UIGraphicsGetCurrentContext() options:nil];
        [ciContext render:image toCVPixelBuffer:pxbuffer];
    }
    
    return pxbuffer;
}

+ (cv::Rect) cvRectForCGRect:(CGRect) rect;
{
    return cv::Rect(rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
}

+ (void)drawPolygon:(CGContextRef)context
             points:(const std::vector<cv::Point>)points
        strokeWidth:(CGFloat)strokeWidth
        strokeColor:(CGColorRef)strokeColor
           isDashed:(BOOL)dashed
          fillColor:(CGColorRef)fillColor
{
    CGContextSetLineCap(context, kCGLineCapSquare);
    
    //connect first to last
    std::vector<cv::Point> pointsConnected = points;
    pointsConnected.push_back(points[0]);
    
    //fill first
    CGContextSetFillColorWithColor(context, fillColor);
    [self drawPath:context points:pointsConnected];
    CGContextFillPath(context);
    
    //stroke
    CGContextSetStrokeColorWithColor(context, strokeColor);
    
    if (dashed) {
        const CGFloat dashLengths[] = {5, 10};
        CGContextSetLineDash(context, 0, dashLengths, 2);
    }
    
    [self drawPath:context points:pointsConnected];
    CGContextStrokePath(context);
    
    if (dashed) {
        //reset
        CGContextSetLineDash(context, 0, 0, 0);
    }
}

+ (void)drawPath:(CGContextRef)context points:(std::vector<cv::Point>)points {
    for (int i=0; i<points.size(); i++) {
        CGPoint p = CGPointMake(points[i].x, points[i].y);
        if (i==0) {
            CGContextMoveToPoint(context, p.x, p.y);
        } else {
            CGContextAddLineToPoint(context, p.x, p.y);
        }
    }
}

#if COREML_ENABLED
+ (int)getMaxClassForArray:(MLMultiArray *)features probability:(double *)prob API_AVAILABLE(ios(11.0)) {
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
#endif

@end
