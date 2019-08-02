//
//  CBP_MeshUtil.hpp
//  Cambrian
//
//  Created by Joel Teply on 1/29/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#ifndef CBP_MeshUtil_hpp
#define CBP_MeshUtil_hpp

#include <stdio.h>

#include <Eigen/Geometry>
#include <Eigen/QR>
#include <Eigen/LU>
#include <opencv2/opencv.hpp>

namespace cbpipe {
    
    class CBP_MeshUtil {
        
    public:
        
        static cv::Rect2f createPolygonsForMask(const cv::Mat &mask,
                                                std::vector<Eigen::Vector3f> &vertices,
                                                std::vector<int> &indices,
                                                std::vector<std::vector<cv::Point>> &boundaries,
                                                float scale,
                                                cv::Point2f offet,
                                                double polyEpsilon,
                                                double minHoleArea = FLT_MAX,
                                                double minOuterArea = 6000.0f);
        
        static void drawPolygon(cv::Mat &dest,
                                const std::vector<Eigen::Vector3f> &vertices,
                                const std::vector<int> &indices,
                                const cv::Rect2f &roi, float scale = 300.0f, cv::Scalar color=cv::Scalar(0,0,255));
    };
    
};

#endif /* CBP_MeshUtil_hpp */
