//
//  CBP_PlaneRansac.hpp
//  Cambrian
//
//  Created by Joel Teply on 3/18/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#ifndef CBP_PlaneRansac_hpp
#define CBP_PlaneRansac_hpp

#include <stdio.h>

#include <stdio.h>
#include <cbar/pipeline/CBP_Types.hpp>
#include <opencv2/opencv.hpp>
#include <memory>

namespace cbpipe {
    
    class DLL_LOCAL CBP_PlaneRansac {
        
    public:
        CBP_PlaneRansac(const cv::Size &workingSize,
                        const std::vector<cv::Vec4f>&lines3d,
                        std::vector<cv::Mat> &surfaces,
                        std::vector<cv::Point2f> &directions,
                        std::vector<cv::Scalar> &colors);
        
        ~CBP_PlaneRansac();
        
        void solve(int numIterations, cv::Mat &debug, std::vector<cv::Vec4f> &baselines);
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif /* CBP_PlaneRansac_hpp */
