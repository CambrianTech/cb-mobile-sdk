//
//  CBColoring.m
//  Cambrian
//
//  Created by Joel Teply on 11/4/12.
//
//

#import "CBColoring.h"
#import "ImageTranslation.h"
#import <imaging/Coloring.h>

@implementation CBColoring

+ (NSArray *)complementsForColor:(UIColor *)color count:(int)count angle:(double)angleSpan;
{
    //color = [UIColor redColor];
    cv::Scalar cvColor = [ImageTranslation scalarForUIColor:color];
    
    std::vector<cv::Scalar> complements = imaging::Coloring::complementary(cvColor, count, angleSpan);
    
    NSMutableArray *colors = [NSMutableArray array];
    
    for (int i=0;i<complements.size(); i++) {
        cv::Scalar cvComplement = complements[i];
        [colors addObject:[ImageTranslation UIColorForScalar:cvComplement]];
    }
    
    return colors;
}

+ (NSArray *)adjacentColors:(UIColor *)color count:(int)count angle:(double)angleSpan;
{
    cv::Scalar cvColor = [ImageTranslation scalarForUIColor:color];
    
    NSMutableArray *colors = [NSMutableArray array];
    
    for (int i=0; i<count; i++) {
        double angle = angleSpan * double(i) / count - angleSpan / 2.0f;
        cv::Scalar rotatedColor = imaging::Coloring::rotateRGB(cvColor, angle);
        
        [colors addObject:[ImageTranslation UIColorForScalar:rotatedColor]];
    }
    
    return colors;
}

+ (NSArray *)shadesOfColor:(UIColor *)color count:(int)count;
{
    cv::Scalar cvColor = [ImageTranslation scalarForUIColor:color];
    cv::Scalar hsvColor = imaging::Coloring::rgbToHSV(cvColor);
    
    NSMutableArray *colors = [NSMutableArray array];
    
    if (hsvColor.val[2] < 50) hsvColor.val[2] = 50;
    for (int i=0; i<count; i++) {
        hsvColor.val[2] = 50 + 205 * (i + 1) / (count + 1);
        //hsvColor.val[2] = 55 + 200 * (i + 1) / (count + 1);
        cv::Scalar rgbColor = imaging::Coloring::hsvToRGB(hsvColor);
        [colors addObject:[ImageTranslation UIColorForScalar:rgbColor]];
    }
    
    return colors;
}

+ (double)euclideanDistance:(UIColor *)colorA
                  fromColor:(UIColor *)colorB;
{
    CGFloat coefficient[3] = {1.0f, 1.0f, 1.0f};
    return [self euclideanDistance:colorA fromColor:colorB asHSV:NO coefficient:coefficient];
}

+ (double)euclideanDistance:(UIColor *)colorA
                  fromColor:(UIColor *)colorB
                      asHSV:(BOOL)asHSV
                coefficient:(CGFloat[3])coefficient;
{
    cv::Scalar cvColorA = [ImageTranslation scalarForUIColor:colorA];
    cv::Scalar cvColorB = [ImageTranslation scalarForUIColor:colorB];
    cv::Scalar cvCoefficient = cv::Scalar(coefficient[0], coefficient[1], coefficient[2]);
    if (asHSV) {
        cv::Scalar cvColorAHSV = imaging::Coloring::rgbToHSV(cvColorA);
        cv::Scalar cvColorBHSV = imaging::Coloring::rgbToHSV(cvColorB);
        return imaging::Coloring::euclideanDistance(cvColorAHSV, cvColorBHSV, NO, cvCoefficient);
    } else {
        return imaging::Coloring::euclideanDistance(cvColorA, cvColorB, NO, cvCoefficient);
    }
}

+ (double)humanPerceptiveDistance:(UIColor *)colorA
                        fromColor:(UIColor *)colorB;
{
    cv::Scalar cvColorA = [ImageTranslation scalarForUIColor:colorA];
    cv::Scalar cvColorB = [ImageTranslation scalarForUIColor:colorB];
    
    return imaging::Coloring::perceptiveDistance(cvColorA, cvColorB);
}



@end
