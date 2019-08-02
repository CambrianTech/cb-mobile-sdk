//
//  CBP_ShadowsAnalyzer.hpp
//  Cambrian
//
//  Created by Joel Teply on 3/7/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#ifndef CBP_ShadowsAnalyzer_hpp
#define CBP_ShadowsAnalyzer_hpp

#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <memory>

#include <cbar/pipeline/analysis/CBP_AreaAnalyzer.hpp>
#include <cbar/pipeline/CBP_Types.hpp>

namespace cbpipe {
    
    class DLL_LOCAL CBP_ShadowsAnalyzer : public CBP_AreaAnalyzer {
    public:
        CBP_ShadowsAnalyzer();
        ~CBP_ShadowsAnalyzer();
        
    protected:
        virtual void initialize();
        virtual bool analyze(cbar::CBAR_VideoFramePtr frame);
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif /* CBP_ShadowsAnalyzer_hpp */
