//
//  Imaging.h
//
//  Created by Joel Teply on 6/14/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef __Imaging__
#define __Imaging__

#include <opencv2/opencv.hpp>
#include <cambrian.h>
#include "Geometry.h"

namespace imaging {

#define ASPECT_RATIO(img) (((double) img.cols) / ((double) img.rows))

class DLL_LOCAL Imaging
{
public:
    
    static void meanStdDevForPolygon(const cv::Mat &image, std::vector<cv::Point2f> points, cv::OutputArray mean, cv::OutputArray stddev);
    
    static cv::Mat rotateImage(const cv::Mat &source, double radians);
    static cv::Mat rotateImage(const cv::Mat &source, double radians, cv::Point2f aroundPoint);
    
    static cv::Scalar imageDifferencePerPixel(const cv::Mat &imageA, const cv::Mat &imageB, float angleBetween = 0);
    static cv::Scalar meanAtPoint(const cv::Mat &img, cv::Point point, float radius = 5);
    static cv::Rect meanStdDevAtPoint(const cv::Mat &img, cv::Scalar &mean, cv::Scalar &stddev, cv::Point point,
                                 float outerRadius = 5, float innerRadius=0, cv::Rect *destRoi=0);
    static void meanStdDevForPoints(const cv::Mat &srcImage, const std::vector<cv::Point>&points,
                                    cv::Scalar &sampleMean, cv::Scalar &sampleStdDev);
    
    static cv::Mat cropImage(const cv::Mat &img, cv::Rect toRect);
    static cv::Mat cropImage(const cv::Mat &img, double desiredAspectRatio, bool isRotated90degrees);
    
    static cv::Mat scaleImage(const cv::Mat &img, double scale, int type = CV_INTER_NN);
    static void restrictImageToSize(const cv::Mat &source, cv::Mat &dest,
                                       cv::Size size, int scaleType = CV_INTER_NN);
    
    static void getCvMat(cv::Mat& mat, uint8_t* rawData, cv::Size size, int numChannels);
    static uchar* getMatData(const cv::Mat& mat);
    
    static bool importMaskedPNG(cv::Mat &largeImage, cv::Mat &alphaMask);
    
    static cv::Mat blend_multiply(const cv::Mat& level1, const cv::Mat& level2, float fopacity=1.0f);
};

};
#endif
