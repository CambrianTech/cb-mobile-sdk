//
//  CBP_ShadowsAnalyzer.cpp
//  Cambrian
//
//  Created by Joel Teply on 3/7/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#include "CBP_ShadowsAnalyzer.hpp"

#include <cbar/pipeline/rendering/rendering.h>
#include <cbcommon/CB_Config.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Coloring.h>
#include <cbar/pipeline/util/util.h>

#include <cbar/pipeline/machine-learning/machine-learning.h>

namespace cbpipe {
    
    struct CBP_ShadowsAnalyzer::Impl
    {
        Impl(CBP_ShadowsAnalyzer *parent) : m_parent(parent) {
            
        }
        
        ~Impl() {}
        
        CBP_ShadowsAnalyzer *m_parent;
        CBP_ShadowsEstimator m_estimator;
        
        void _initialize() {
            m_estimator.loadDeepNetwork();
        }
        
        bool _analyze(cbar::CBAR_VideoFramePtr frame) {
            
            auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return false;
            auto tracker = renderer->getAnalyzerOfType<CBP_FeatureTracker>(); if (!tracker) return false;
            
            std::map<std::string, cv::Mat> pix2PixInput;
            pix2PixInput["Placeholder__0"] = frame->uprightRGBImage();
            
            cv::Mat shadowsResult;
            m_estimator.inference(frame->uprightRGBImage(), pix2PixInput, shadowsResult);
            if (shadowsResult.empty()) return false;
            
            cv::cvtColor(shadowsResult, shadowsResult, CV_RGB2GRAY);
            
            deep_result result;
            result.frame = frame;
            result.outputs.push_back(shadowsResult);
                                    
            for (auto surfaceAnalyzer : renderer->getAnalyzersOfType<CBP_SurfaceAnalyzer>()) {
                std::thread([surfaceAnalyzer, result](){
                    surfaceAnalyzer->shadowsDataUpdated(result);
                }).detach();
            }
            
            return true;
        }
    };
    
    CBP_ShadowsAnalyzer::CBP_ShadowsAnalyzer() : CBP_AreaAnalyzer("CBP_ShadowsAnalyzer") {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
        
        setPriority(CBThreadPriorityLow);
    }
    
    CBP_ShadowsAnalyzer::~CBP_ShadowsAnalyzer() {
        
    }
    
    void CBP_ShadowsAnalyzer::initialize() {
        m_pImpl->_initialize();
    }
    
    bool CBP_ShadowsAnalyzer::analyze(cbar::CBAR_VideoFramePtr frame) {
        return m_pImpl->_analyze(frame);
    }
};
