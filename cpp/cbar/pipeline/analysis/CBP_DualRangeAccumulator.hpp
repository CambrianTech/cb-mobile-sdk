//
//  CBP_DualRangeAccumulator.hpp
//  Cambrian
//
//  Created by Joel Teply on 3/10/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#ifndef CBP_DualRangeAccumulator_hpp
#define CBP_DualRangeAccumulator_hpp

#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <memory>

#include <cbar/pipeline/analysis/CBP_SurfaceAccumulator.hpp>
#include <cbar/pipeline/CBP_Types.hpp>
#include <cbar/pipeline/analysis/CBP_AnalysisTypes.hpp>
#include <cbar/pipeline/analysis/CBP_LargeImage.hpp>

namespace cbpipe {
    
    class DLL_LOCAL CBP_DualRangeAccumulator : public CBP_SurfaceAccumulator {
        
    public:
        CBP_DualRangeAccumulator(const std::string &name, float nearRange, float nearPPM, float farRange, float farPPM);
        ~CBP_DualRangeAccumulator();
        
        virtual const std::string& getName() const;
        
        virtual float getPixelsPerMeter() const;
        virtual void setAlphaAtIndex(int index, float alpha);
        virtual void setDebugEnabled(int index, bool debug);
        
        virtual void setPlaneNormal(const Eigen::Vector3f &center);        
        virtual void setPlaneCenter(const Eigen::Vector3f &center);
        
        virtual void addSurfaceData(cbar::CBAR_VideoFramePtr frame, const std::vector<cv::Mat> &data,
                                    const std::vector<cv::Vec4f> *lines=0, int surfaceRotation=0);
        virtual void clear();

        virtual void systemNowStable(cbar::CBAR_VideoFramePtr frame, const Eigen::Vector3f &groundCenter);
        
        virtual void lock();
        virtual void unlock();
                
        virtual void getSurfaceData(std::vector<cv::Mat> &mergedImages, std::vector<cv::Vec4f> &mergedLines, cv::Mat &bounds, cv::Rect &extents3d);

        virtual cv::Rect get2DExtents(cv::Rect2f roi3d=cv::Rect2f(), cv::Rect roi2d=cv::Rect()) const;
        virtual cv::Rect2f get3DExtents(cv::Rect roi=cv::Rect()) const;
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif /* CBP_DualRangeAccumulator_hpp */
