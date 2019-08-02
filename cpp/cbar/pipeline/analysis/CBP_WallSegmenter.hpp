//
//  CBP_WallSegmenter.hpp
//  Cambrian
//
//  Created by Joel Teply on 10/12/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#ifndef CBP_WallSegmenter_hpp
#define CBP_WallSegmenter_hpp

#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <memory>

#include <cbar/pipeline/analysis/CBP_Segmenter.hpp>

namespace cbpipe {
    class CBP_RenderingEngine;
    
    class DLL_LOCAL CBP_WallSegmenter : public CBP_Segmenter {
    public:
        CBP_WallSegmenter(const cv::Size &gridUnitSize);
        ~CBP_WallSegmenter();
        
        virtual FillResult segmentAtPoint(const cv::Point2f &paintPoint, cbar::CBAR_VideoFramePtr frame);
        
        virtual void cancel();
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif /* CBP_WallSegmenter_hpp */
