//
//  FloodFillAlgorithm.hpp
//  Cambrian
//
//  Created by Joel Teply on 10/23/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#ifndef CBP_FloodFillAlgorithm_hpp
#define CBP_FloodFillAlgorithm_hpp

#define INLINE_ALGORITHM(x) #x

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <cbar/CBAR_VideoFrame.hpp>
#include "CBP_MLTypes.hpp"

namespace cbpipe {
    
    class CBP_RenderingEngine;
    
    class DLL_LOCAL CBP_FloodFillAlgorithm {
        
    public:
        CBP_FloodFillAlgorithm(PaintPointType type,
                               const cv::Size &gridUnitSize=cv::Size(10,10),
                               const cv::Size &maxImageSize=cv::Size(640,640));
        ~CBP_FloodFillAlgorithm();
        
        
        void setFrame(cbar::CBAR_VideoFramePtr frame, const std::vector<cv::Point2f> &paintPoints);
        
        cv::Mat run(cbar::CBAR_VideoFramePtr frame, const cv::Point2f &paintPoint, bool isVideo=true);
        cv::Mat segmentImage(const cv::Point2f &paintPoint, cbar::CBAR_VideoFramePtr frame=cbar::CBAR_VideoFramePtr());
        
        virtual void cancel();
    protected:

        virtual cv::Mat runGrid(cv::Ptr<texture_grid> grid, const std::vector<cv::Point2f> &paintPoints, bool isVideo);
        
        virtual void prepare(cv::Ptr<texture_grid> grid, bool isVideo) {};
        
        virtual void getFFMatches(cv::Ptr<texture_grid> grid, std::vector<texture_sample*>sampleStack, std::vector<texture_sample*>&matches);
        
        cv::Point pointForIndex(cv::Ptr<texture_grid> grid, const cv::Point &index);
        cv::Point indexForPoint(cv::Ptr<texture_grid> grid, const cv::Point &point);
        
        virtual void getReplicatedSeedValue(cv::Ptr<texture_grid> grid, std::vector<texture_sample*>&seeds, texture_sample &replicatedSample);

        virtual bool isMatch(cv::Ptr<texture_grid> grid, texture_sample &elementA, texture_sample &elementB) = 0;
        
        const cv::Size& getGridUnitSize();
        const cv::Size& getMaxImageSize();
        
    protected:
        cv::Ptr<texture_grid> _retainedGrid;
        
        bool m_hasInitialized = false;
        bool m_isCanceled = false;
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
};

#endif /* CBP_FloodFillAlgorithm_hpp */
