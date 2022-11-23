//
//  CBP_NormalsAnalyzer.cpp
//  Cambrian
//
//  Created by Joel Teply on 4/2/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#include "CBP_ElevationAnalyzer.hpp"

#include <cbar/pipeline/rendering/rendering.h>
#include <cbcommon/CB_Config.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Coloring.h>
#include <cbar/pipeline/util/util.h>

#include <cbar/pipeline/machine-learning/machine-learning.h>

using namespace imaging;

namespace cbpipe {
    
    struct CBP_ElevationAnalyzer::Impl
    {
        Impl(CBP_ElevationAnalyzer *parent) : m_parent(parent) {
            
        }
        
        ~Impl() {}
        
        CBP_ElevationAnalyzer *m_parent;
        CBP_ElevationEstimator m_estimator;
        
        void _initialize() {
            m_estimator.loadDeepNetwork();
        }
        
        bool _analyze(cbar::CBAR_VideoFramePtr frame) {
            
            auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return false;
            auto tracker = renderer->getAnalyzerOfType<CBP_FeatureTracker>(); if (!tracker) return false;

            deep_result result;
            result.frame = frame;
            
//            result.output.resize(1);
//            result.inputs["Placeholder__0"] = frame->uprightRGBImage();
//            m_estimator.inference(frame->uprightRGBImage(), result.inputs, result.output[0]);
  
            return true;
        }
    };
    
    CBP_ElevationAnalyzer::CBP_ElevationAnalyzer() : CBP_AreaAnalyzer("CBP_ElevationAnalyzer") {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
        
        setPriority(CBThreadPriorityLow);
    }
    
    CBP_ElevationAnalyzer::~CBP_ElevationAnalyzer() {
        
    }
    
    void CBP_ElevationAnalyzer::initialize() {
        m_pImpl->_initialize();
    }
    
    bool CBP_ElevationAnalyzer::analyze(cbar::CBAR_VideoFramePtr frame) {
        return m_pImpl->_analyze(frame);
    }
};
