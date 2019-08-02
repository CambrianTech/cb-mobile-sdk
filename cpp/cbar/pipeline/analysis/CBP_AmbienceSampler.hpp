//
//  CBP_ColorSample.hpp
//  Cambrian
//
//  Created by Joel Teply on 11/12/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBP_AmbienceSampler_hpp
#define CBP_AmbienceSampler_hpp

#include <stdio.h>
#include <utility/JSONHelper.h>

#include <cbar/pipeline/CBP_Types.hpp>
#include <cbar/pipeline/analysis/CBP_Analyzer.hpp>

namespace cbpipe {
    
    class CBP_AmbienceSampler;
    
    struct stat_sample {
        double overlayIntensity;
        double bgIntensity = 0;
        cv::Scalar ambientLevel;
    };
    
    class DLL_LOCAL CBP_AmbienceReceiver {
        friend class CBP_AmbienceSampler;
        
    public:
        CBP_AmbienceReceiver();
        ColorAdjustment getColorAdjustment();
    protected:
        ~CBP_AmbienceReceiver();
        
        void loadJSONState(cbscene::CBAR_Scene *scene, const Json::Value& analysisNode);
        bool saveJSONState(cbscene::CBAR_Scene *scene, Json::Value& analysisNode);
        
        virtual cv::Mat getMask()=0;
        virtual void ambienceCalculationUpdated()=0;
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
        
        void addSample(const stat_sample &sample, int max);
        void clearSamples();
        void calculate();
    };
    
    class DLL_LOCAL CBP_AmbienceSampler : public CBP_AnalyzerThread {
        friend CBP_AmbienceReceiver;
    public:
        CBP_AmbienceSampler();
        ~CBP_AmbienceSampler();
        
        virtual bool handleFrame(cbar::CBAR_VideoFramePtr frame);
        
        void recalculate();
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
    
};

#endif /* CBP_AmbienceSampler_hpp */
