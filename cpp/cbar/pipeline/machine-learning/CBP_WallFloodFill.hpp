//
//  FloodFillAlgorithm.hpp
//  Cambrian
//
//  Created by Joel Teply on 10/23/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#ifndef CBP_WallFloodFill_hpp
#define CBP_WallFloodFill_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <cbar/CBAR_VideoFrame.hpp>

#include "CBP_FloodFillAlgorithm.hpp"


namespace cbpipe {
    
    class CBP_RenderingEngine;
    
    class DLL_LOCAL CBP_WallFloodFill : public CBP_FloodFillAlgorithm {
        
    public:
        CBP_WallFloodFill(const cv::Size &gridSize, double constantMean=2.0f, double constantStdDev=5.0f);
        ~CBP_WallFloodFill();
        
    protected:
        
        virtual cv::Mat runGrid(cv::Ptr<texture_grid> grid, const std::vector<cv::Point2f> &paintPoints, bool isVideo);
        
        virtual void prepare(cv::Ptr<texture_grid> grid, bool isVideo);
        
        virtual void getReplicatedSeedValue(cv::Ptr<texture_grid> grid, std::vector<texture_sample*>&seeds, texture_sample &replicatedSample);
        
        virtual bool isMatch(cv::Ptr<texture_grid> grid, texture_sample &elementA, texture_sample &elementB);
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
};

#endif /* CBP_PaintFloodFill_hpp */
