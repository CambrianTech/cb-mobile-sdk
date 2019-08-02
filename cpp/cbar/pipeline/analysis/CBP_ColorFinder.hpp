//
//  CBP_ColorFinder.hpp
//  Cambrian
//
//  Created by Joel Teply on 10/12/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#ifndef CBP_ColorFinder_hpp
#define CBP_ColorFinder_hpp

#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <memory>

#include <cbar/pipeline/analysis/CBP_Analyzer.hpp>

#include <cbar/pipeline/CBP_Types.hpp>

namespace cbpipe {
    
    class CBP_RenderingEngine;
    
    class DLL_LOCAL CBP_ColorFinder : public CBP_AnalyzerThread {
    public:
        CBP_ColorFinder();
        ~CBP_ColorFinder();
        
        virtual bool handleFrame(cbar::CBAR_VideoFramePtr frame);
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}


#endif /* CBP_ColorFinder_hpp */
