//
//  CBP_AreaAnalyzer.hpp
//  Cambrian
//
//  Created by Joel Teply on 9/7/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#ifndef CBP_AreaAnalyzer_hpp
#define CBP_AreaAnalyzer_hpp

#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <memory>

#include <cbar/pipeline/analysis/CBP_Analyzer.hpp>
#include <cbar/pipeline/CBP_Types.hpp>

namespace cbpipe {
    
    //defaults:
    struct area_run_params {
        double max_rotation_velocity = 0.15f;
        double min_run_seconds = 5.0f; //required to run at least this often
        double min_run_distance = 5.0f; // for a point projected out 2m, how far must it move to trigger a run
        double max_pitch_radians = M_PI_4; //45 degrees
    };

    class DLL_LOCAL CBP_AreaAnalyzer : public CBP_AnalyzerThread {
    public:
        CBP_AreaAnalyzer(const std::string &name);
        
        virtual bool needsColorFrames() const { return true;};
        virtual bool videoOnly() const { return true;};
        virtual bool handleFrame(cbar::CBAR_VideoFramePtr frame);
        
        void needsRefresh();
        virtual void systemNowStable(cbar::CBAR_VideoFramePtr frame, const Eigen::Vector3f &groundCenter) {};
    protected:
        ~CBP_AreaAnalyzer();
        virtual void initialize() {};
        virtual area_run_params getParams() const { return area_run_params(); };
        virtual bool analyze(cbar::CBAR_VideoFramePtr frame) = 0;
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif /* CBP_AreaAnalyzer */
