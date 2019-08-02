//
//  CBP_WallSegmenter.hpp
//  Cambrian
//
//  Created by Joel Teply on 10/12/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#ifndef CBP_FloorSegmenter_hpp
#define CBP_FloorSegmenter_hpp

#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <memory>

#include <cbar/pipeline/analysis/CBP_Segmenter.hpp>

namespace cbpipe {
    class CBP_RenderingEngine;
    
    class DLL_LOCAL CBP_FloorSegmenter : public CBP_Segmenter {
    public:
        CBP_FloorSegmenter(const cv::Size &gridUnitSize);
        ~CBP_FloorSegmenter();

        virtual FillResult segmentAtPoint(const cv::Point2f &paintPoint, cbar::CBAR_VideoFramePtr frame);
        
        virtual void cancel();
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif /* CBP_FloorSegmenter_hpp */
