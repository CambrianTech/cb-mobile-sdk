//
//  CBP_AnalysisUtil.hpp
//  Cambrian
//
//  Created by Joel Teply on 4/21/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#ifndef CBP_AnalysisUtil_hpp
#define CBP_AnalysisUtil_hpp

#include <opencv2/opencv.hpp>
#include <memory>

#include <cbar/pipeline/analysis/CBP_AreaAnalyzer.hpp>
#include <cbar/pipeline/CBP_Types.hpp>
#include <cbar/pipeline/analysis/CBP_AnalysisTypes.hpp>

namespace cbpipe {
    
    class DLL_LOCAL CBP_AnalysisUtil {
    public:
        static bool kmeansNormals(const cv::Mat &normals,
                                  const cv::Size &workingSize,
                                  std::vector<cv::Mat>&surfaces,
                                  std::vector<int>&counts,
                                  std::vector<cv::Point2f>&directions,
                                  std::vector<cv::Scalar>&colors,
                                  cv::Mat *debug=0);
        
        static cv::Point pointInMask(const Eigen::Vector3f &point3d, const cv::Rect2f &extents3d, const cv::Size &maskSize);
        
        static cv::Vec3b getMostCommonColor(const cv::Mat &src, const cv::Mat &mask, int K);
        
        static Eigen::Vector3f colorToDirection(const cv::Scalar &color);
        
        static cv::Scalar directionToColor(const Eigen::Vector3f &direction);
        
        static std::vector<cv::Point2f> getLineContourIntersections(const cv::Vec4f &line, const std::vector<cv::Point> &hull);
    };
}

#endif /* CBP_AnalysisUtil_hpp */

