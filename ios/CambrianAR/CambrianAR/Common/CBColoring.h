//
//  CBColoring.h
//  Cambrian
//
//  Created by Joel Teply on 11/4/12.
//
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

__attribute__((visibility("default")))
@interface CBColoring : NSObject

+ (NSArray *)complementsForColor:(UIColor *)color count:(int)count angle:(double)angleSpan;

+ (NSArray *)adjacentColors:(UIColor *)color count:(int)count angle:(double)angleSpan;

+ (NSArray *)shadesOfColor:(UIColor *)color count:(int)count;

+ (double)euclideanDistance:(UIColor *)colorA
                  fromColor:(UIColor *)colorB;

+ (double)euclideanDistance:(UIColor *)colorA
                  fromColor:(UIColor *)colorB
                      asHSV:(BOOL)asHSV
                coefficient:(CGFloat[3])coefficient;

//http://www.compuphase.com/cmetric.htm
+ (double)humanPerceptiveDistance:(UIColor *)colorA
                        fromColor:(UIColor *)colorB;
@end
