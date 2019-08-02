//
//  CBP_Segmenter.hpp
//  Cambrian
//
//  Created by Joel Teply on 11/13/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#ifndef CBP_Segmenter_hpp
#define CBP_Segmenter_hpp

#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <memory>

#include <cbar/pipeline/analysis/CBP_Analyzer.hpp>

namespace cbpipe {
    class CBP_RenderingEngine;
    
    struct FillResult {
        cv::Mat overlay;
        cv::Mat reflections;
        int64_t frameIndex;
    };
    
    class DLL_LOCAL CBP_Segmenter {
    public:
        CBP_Segmenter(std::string name) { }
        
        virtual void handleFrame(cbar::CBAR_VideoFramePtr frame) {};
        virtual FillResult segmentAtPoint(const cv::Point2f &paintPoint, cbar::CBAR_VideoFramePtr frame) = 0;
        virtual void cancel() = 0;
    protected:
        ~CBP_Segmenter() { }
    };
}

#endif /* CBP_Segmenter_hpp */
