//
//  Coloring.h
//
//  Created by Joel Teply on 6/14/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef __Coloring__
#define __Coloring__

#include <opencv2/opencv.hpp>
#include <cambrian.h>
#include <cbcommon/CB_Types.h>
#include <Eigen/Geometry>

namespace imaging {
    class DLL_LOCAL Coloring
    {
    public:
        
        static double euclideanDistance(cv::Scalar colorA, cv::Scalar colorB,
                                        bool isHSV=false, cv::Scalar coefficient=cv::Scalar::all(1.0f));
        static double euclideanDistanceSq(cv::Scalar colorA, cv::Scalar colorB,
                                          bool isHSV=false, cv::Scalar coefficient=cv::Scalar::all(1.0f));
        static double manhattanDistance(cv::Scalar colorA, cv::Scalar colorB,
                                        bool isHSV=false, cv::Scalar coefficient=cv::Scalar::all(1.0f));
        static double manhattanDistanceSq(cv::Scalar colorA, cv::Scalar colorB,
                                          bool isHSV=false, cv::Scalar coefficient=cv::Scalar::all(1.0f));
        
        static double perceptiveDistance(cv::Scalar colorA, cv::Scalar colorB);
        
        static ColorClass getColorClassification(const cv::Scalar &color);
        static ColorClass getColorClassification(double hue, double saturation, double brightness);
        
        static cv::Scalar rgbToBGR(cv::Scalar rgb);
        static cv::Scalar bgrToRGB(cv::Scalar bgr);
        
        static cv::Scalar rgbToHSV(cv::Scalar rgb, bool isHSV255 = false);
        static cv::Scalar hsvToRGB(cv::Scalar hsv, bool isHSV255 = false);
        
        static cv::Scalar rgbToHSL(cv::Scalar rgb);
        static cv::Scalar hslToRGB(cv::Scalar rgb);
        
        static cv::Scalar bgrToHSV(cv::Scalar bgr, bool isHSV255 = false);
        static cv::Scalar hsvToBGR(cv::Scalar hsv, bool isHSV255 = false);
        
        
        static cv::Scalar rotateRGB(cv::Scalar rgb, double degrees);
        static cv::Scalar rotateHSV(const cv::Scalar hsv, double degrees, bool isHSV255 = false);
        
        static std::vector<cv::Scalar> triadicColors(cv::Scalar rgb);
        static std::vector<cv::Scalar> analogousColors(cv::Scalar rgb);
        static std::vector<cv::Scalar> complementary(cv::Scalar rgb, int imageCount=1, double angleRange=0);
        
        static std::vector<cv::Scalar> sampleColors(cv::Mat img, int xSampleCount = 25, int ySampleCount = 25);
        
        static std::vector<cv::Scalar> getMostCommonColors(const cv::Mat &rgb, cv::Mat &reducedRGB, int count);
        
        static std::vector<cv::Scalar> mostCommonHSVColors(const cv::Mat &imgHsv, int numColors = 10,
                                                           bool sample = false, //sample actual colors
                                                           cv::Scalar lowerBound = cv::Scalar(0, 30, 70),
                                                           cv::Scalar upperBound = cv::Scalar(180, 255, 255),
                                                           bool boostBrights=true);
        
        static std::vector<cv::Point> getColorPositions(const cv::Mat &rgb, cv::Mat &reducedRGB,
                                                        const std::vector<cv::Scalar> &colors, cv::Scalar coefficient=cv::Scalar(5.0f, 2.0, 1.0));
        
        static cv::Scalar randomColor(int minValue=0, int maxValue=255);
        
    private:
        static cv::Scalar _subtractColors(cv::Scalar colorA, cv::Scalar colorB, bool isHSV, cv::Scalar coefficient);
        
    };
};
#endif
