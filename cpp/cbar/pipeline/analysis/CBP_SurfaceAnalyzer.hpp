//
//  CBP_SurfaceAnalyzer.hpp
//  Cambrian
//
//  Created by Joel Teply on 7/12/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#ifndef CBP_SurfaceAnalyzer_hpp
#define CBP_SurfaceAnalyzer_hpp

#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <memory>

#include <cbar/pipeline/analysis/CBP_AreaAnalyzer.hpp>
#include <cbar/pipeline/CBP_Types.hpp>
#include <cbar/pipeline/analysis/CBP_AnalysisTypes.hpp>

#define SEMANTIC_NAME "semantic"
#define NORMALS_NAME "normals"
#define SHADOWS_NAME "shadows"

namespace cbscene {
    class CBAR_SurfaceAsset;
}

namespace cbpipe {
    
    class plane_contour;
    class CBP_SurfaceLightingAnalyzer;
    class CBP_SurfaceAccumulator;
    
    enum acc_index {
        acc_semantic = 0,
        acc_vert_lines,
        acc_horiz_lines,
        acc_normals,
        acc_shadows,
        acc_length,
    };
    
    class DLL_LOCAL CBP_SurfaceAnalyzer : public CBP_AreaAnalyzer {
        friend CBP_SurfaceLightingAnalyzer;
    public:
        CBP_SurfaceAnalyzer(cbscene::CBAR_SurfaceAsset *surfaceAsset);
        ~CBP_SurfaceAnalyzer();
        
        static std::shared_ptr<CBP_SurfaceAnalyzer> getGroundAnalyzer();
        
        virtual void initialize();
        virtual area_run_params getParams() const;
        virtual bool analyze(cbar::CBAR_VideoFramePtr frame);
        
        Eigen::Vector3f getPlaneNormal() const;
        void setPlaneNormal(const Eigen::Vector3f &);
        
        Eigen::Vector3f getPlaneCenter() const;
        void setPlaneCenter(const Eigen::Vector3f &);
        
        bool isGroundPlane() const;
        
        const std::map<acc_index, std::shared_ptr<CBP_SurfaceAccumulator>> getAccumulators() const;
        
        void semanticDataUpdated(const deep_result &result);
        void shadowsDataUpdated(const deep_result &result);
        void normalsDataUpdated(const deep_result &result);
        void linesFound(cbar::CBAR_VideoFramePtr frame, const std::vector<cv::Vec4f> &detectedLines);
        
        void clearAll();

        int64_t getLastUpdatedIndex();
        
        std::vector<cv::Point>getLastContour() const;
        
        virtual void reintegrationStarting(cbar::CBAR_VideoFramePtr frame);
        virtual void systemNowStable(cbar::CBAR_VideoFramePtr frame, const Eigen::Vector3f &groundCenter);
        virtual void reintegrationCompleted(cbar::CBAR_VideoFramePtr frame);
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif /* CBP_SurfaceAnalyzer_hpp */
