//
//  Accelerated.h
//  
//
//  Created by Joel Teply on 7/9/12.
//
//

#ifndef __Accelerated__
#define __Accelerated__

#if __APPLE__
#import <Accelerate/Accelerate.h>
#endif

#include <opencv2/core/core.hpp>
#include <cambrian.h>

class DLL_LOCAL Accelerated
{
public:
    static void dilate(const cv::Mat &src, cv::Mat &dest, int size, bool isElipse=true);
    static void erode(const cv::Mat &src, cv::Mat &dest, int size, bool isElipse=true);
    
    static void roughErode(const cv::Mat &image, cv::Mat &dest, cv::Size size, bool ellipse = true, double minScale=0.5);
    static void roughDilate(const cv::Mat &image, cv::Mat &dest, cv::Size size, bool ellipse = true, double minScale=0.5);
    
    static void
    goodFeaturesToTrack(const cv::Mat &imagePrev,
                        std::vector<cv::Point2f> &corners,
                        int maxCorners,
                        double qualityLevel,
                        double minDistance,
                        cv::InputArray mask = cv::noArray(),
                        int blockSize = 3,
                        bool useHarrisDetector = false,
                        double k = 0.04);
    
    static void
    calcOpticalFlowPyrLK(const cv::Mat &imagePrev, const cv::Mat &imageNext,
                         const std::vector<cv::Point2f> &features_current,
                         std::vector<cv::Point2f> &features_next,
                         std::vector<uchar> &status,
                         std::vector<float> &err,
                         cv::Size winSize = cv::Size(21,21),
                         int maxLevel = 3,
                         cv::TermCriteria criteria = cv::TermCriteria(cv::TermCriteria::COUNT+cv::TermCriteria::EPS, 30, 0.01),
                         int flags = 0,
                         double minEigThreshold = 1e-4);
    
    static int
    floodFillMask(const cv::Mat &image, cv::Mat &mask,
                  cv::Point seedPoint, cv::Scalar newVal,
                  cv::Scalar loDiff = cv::Scalar(), cv::Scalar upDiff = cv::Scalar());
};

#endif /* defined(__Accelerated__) */
