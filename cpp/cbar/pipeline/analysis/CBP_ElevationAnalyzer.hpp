//
//  CBP_ShadowsAnalyzer.hpp
//  Cambrian
//
//  Created by Joel Teply on 4/2/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#ifndef CBP_ElevationAnalyzer_hpp
#define CBP_ElevationAnalyzer_hpp

#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <memory>

#include <cbar/pipeline/analysis/CBP_AreaAnalyzer.hpp>
#include <cbar/pipeline/CBP_Types.hpp>

namespace cbpipe {
    
    class DLL_LOCAL CBP_ElevationAnalyzer : public CBP_AreaAnalyzer {
    public:
        CBP_ElevationAnalyzer();
        ~CBP_ElevationAnalyzer();
        
    protected:
        virtual void initialize();
        virtual bool analyze(cbar::CBAR_VideoFramePtr frame);
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif /* CBP_ShadowsAnalyzer_hpp */
