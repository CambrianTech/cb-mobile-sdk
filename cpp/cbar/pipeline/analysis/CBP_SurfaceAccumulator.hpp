//
//  CBP_SurfaceAccumulator.hpp
//  Cambrian
//
//  Created by Joel Teply on 2/1/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#ifndef CBP_SurfaceAccumulator_hpp
#define CBP_SurfaceAccumulator_hpp

#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <memory>

#include <cbar/pipeline/analysis/CBP_Analyzer.hpp>
#include <cbar/pipeline/CBP_Types.hpp>
#include <cbar/pipeline/analysis/CBP_AnalysisTypes.hpp>
#include <cbar/pipeline/analysis/CBP_LargeImage.hpp>

namespace cbpipe {
    
    class DLL_LOCAL CBP_SurfaceAccumulator {

    public:
        CBP_SurfaceAccumulator(const std::string &name, float range, float ppm);
        ~CBP_SurfaceAccumulator();
        
        virtual const std::string& getName() const;
        
        virtual float getRangeMeters() const;
        virtual float getPixelsPerMeter() const;
        
        virtual float getAlphaAtIndex(int index) const;
        virtual void setAlphaAtIndex(int index, float alpha);
        
        virtual void setDebugEnabled(int index, bool debug);
        
        virtual Eigen::Vector3f getPlaneNormal() const;
        virtual void setPlaneNormal(const Eigen::Vector3f &center);
        
        virtual Eigen::Vector3f getPlaneCenter() const;
        virtual void setPlaneCenter(const Eigen::Vector3f &center);
        
        virtual void addSurfaceData(cbar::CBAR_VideoFramePtr frame, const std::vector<cv::Mat> &data,
                                    const std::vector<cv::Vec4f> *lines=0, int surfaceRotation=0);
        virtual void clear();
        
        virtual void getSurfaceData(std::vector<cv::Mat> &mergedImages, std::vector<cv::Vec4f> &mergedLines, cv::Mat &bounds, cv::Rect &extents3d);
        virtual void setSurfaceData(int index, cv::Mat data, cv::Mat bounds = cv::Mat(), cv::Rect roi=cv::Rect());
        
        virtual void systemNowStable(cbar::CBAR_VideoFramePtr frame, const Eigen::Vector3f &groundCenter);
        
        virtual cv::Rect get2DExtents(cv::Rect2f roi3d=cv::Rect2f(), cv::Rect roi2d=cv::Rect()) const;
        virtual cv::Rect2f get3DExtents(cv::Rect roi2d=cv::Rect()) const;
        
        virtual void lock();
        virtual void unlock();
        
        virtual int64_t getFrameIndex() const;
    protected:
        std::vector<CBP_LargeImage>& mergedImages();
        CBP_LargeImage& mergedBounds();
        bool getDebugEnabled(int index) const;
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif /* CBP_SurfaceAccumulator_hpp */
