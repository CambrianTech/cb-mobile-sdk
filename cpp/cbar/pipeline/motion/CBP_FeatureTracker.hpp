//
//  CBP_FeatureTracker.hpp
//  Cambrian
//
//  Created by Joel Teply on 1/18/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#ifndef CBP_FeatureTracker_hpp
#define CBP_FeatureTracker_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <Eigen/Geometry>
#include <cambrian.h>
#include <cbar/pipeline/analysis/CBP_Analyzer.hpp>

namespace cbpipe {
    
    class DLL_LOCAL CBP_FeatureTracker : public CBP_AnalyzerThread {
    public:
        CBP_FeatureTracker();
        ~CBP_FeatureTracker();
        
        Eigen::Matrix4f getCameraProjection(int64_t frameIndex=cbar::CBAR_VideoFrame::lastFrameIndex());
        Eigen::Matrix4f getWorldTransform(int64_t frameIndex=cbar::CBAR_VideoFrame::lastFrameIndex());
        Eigen::Vector3f getHeading(int64_t frameIndex=cbar::CBAR_VideoFrame::lastFrameIndex());
        
        std::map<uint64_t, Eigen::Vector3f> getPointCloudData();
        void setPointCloudData(const std::map<uint64_t, Eigen::Vector3f> &points);
        
        virtual bool needsColorFrames() const { return false;};
        virtual bool needsRealTime() const { return true;};
        virtual bool videoOnly() const { return true;};
        
        virtual bool handleFrame(cbar::CBAR_VideoFramePtr frame);
        
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}


#endif /* CBP_FeatureTracker_hpp */
