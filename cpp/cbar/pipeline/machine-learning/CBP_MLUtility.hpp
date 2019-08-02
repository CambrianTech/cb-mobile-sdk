//
//  CBP_MLUtility.hpp
//  Cambrian
//
//  Created by Joel Teply on 1/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBP_MLUtility_hpp
#define CBP_MLUtility_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <cbar/CBAR_VideoFrame.hpp>
#include "CBP_MLTypes.hpp"

namespace cbpipe {
    
    class DLL_LOCAL CBP_MLUtility {
    public:
        static cbar::CBAR_VideoFramePtr generateFrame(const std::string &imagePath, bool augmentImage);
        
        static cv::Ptr<texture_grid> createGrid(cbar::CBAR_VideoFramePtr frame,
                                                const cv::Size &gridUnitSize, const cv::Size &maxImageSize,
                                                const std::vector<cv::Point2f> &paintPoints);
        
        static void setGridSeeds(cv::Ptr<texture_grid> grid, const std::vector<cv::Point2f> &paintPoints);
        
        static void makePositiveNegativeMask(cv::Ptr<texture_grid> grid);
        static void makePositiveNegativeMask(const cv::Ptr<texture_grid> grid, cv::Mat &positivesMask, cv::Mat &negativesMask);
        
        static cv::Point getTrainingPaintPoint(const cv::Mat &mask);
        
        static void debugResults(cv::Ptr<texture_grid> grid, cv::Mat &debugOutput, const cv::Mat &results, const cv::Scalar &color, bool filled);
        static cv::Mat getDebugImage(cv::Ptr<texture_grid> grid, std::vector<cv::Mat> &matches);
        static std::vector<std::string> getTrainingImages(const std::string &trainingDirectory, std::string filter="");
        
        static cv::Point indexForPoint(cv::Ptr<texture_grid> grid, const cv::Point &point);
        static cv::Point pointForIndex(cv::Ptr<texture_grid> grid, const cv::Point &index);
        
        static void getMaxClass(const cv::Mat &probBlob, int *classId, double *classProb);
    };
};

#endif /* CBP_MLUtility_hpp */
