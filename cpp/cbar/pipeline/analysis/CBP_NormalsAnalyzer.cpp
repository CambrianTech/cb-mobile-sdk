//
//  CBP_NormalsAnalyzer.cpp
//  Cambrian
//
//  Created by Joel Teply on 3/7/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#include "CBP_NormalsAnalyzer.hpp"

#include <cbar/pipeline/rendering/rendering.h>
#include <cbcommon/CB_Config.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Coloring.h>
#include <cbar/pipeline/util/util.h>
#include <opencv2/core/eigen.hpp>

#include <cbar/pipeline/machine-learning/machine-learning.h>

using namespace imaging;

namespace cbpipe {
    
    struct CBP_NormalsAnalyzer::Impl
    {
        Impl(CBP_NormalsAnalyzer *parent) : m_parent(parent) {
            m_params.max_pitch_radians = M_PI_4;
        }
        
        ~Impl() {}
        
        area_run_params m_params;
        CBP_NormalsAnalyzer *m_parent;
        CBP_SurfaceNormalsEstimator m_estimator;
        
        void _initialize() {
            m_estimator.loadDeepNetwork();
        }
        
        bool _analyze(cbar::CBAR_VideoFramePtr frame) {
            
            auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return false;
            auto tracker = renderer->getAnalyzerOfType<CBP_FeatureTracker>(); if (!tracker) return false;
            
            deep_result result;
            result.frame = frame;
            
            //Normals
            cv::Mat normalsResult;
            
            std::map<std::string, cv::Mat> pix2PixInput;
            pix2PixInput["Placeholder__0"] = frame->uprightRGBImage();
            m_estimator.inference(frame->uprightRGBImage(), pix2PixInput, normalsResult);
            
            if (normalsResult.empty()) return false;
            
            //convert normals to world space
            Eigen::Matrix4f worldPosition = tracker->getWorldTransform(frame->frameIndex);
            Eigen::Matrix3f worldRotation = worldPosition.block<3,3>(0,0);
            //Eigen::Matrix3f cameraRotation = worldRotation.inverse();
            
            //convert to Y up: (x,y,z) --> (x,z,y)
            std::vector<cv::Mat>planes;
            cv::split(normalsResult, planes);
            planes = {planes[0], planes[2], planes[1]};
            cv::merge(planes, normalsResult);
            normalsResult.convertTo(normalsResult, CV_32FC3, 2.0f / 255.0, -1.0f);//image is now -1 to 1, Y up
            
            //per element multiply of 3x3 rotation
            cv::Mat rotation;
            eigen2cv(worldRotation, rotation);//convert eigen matrix to CV
            cv::Mat normalsFlattened = normalsResult.reshape(1, normalsResult.rows * normalsResult.cols);
            cv::Mat product = normalsFlattened * rotation;
            normalsResult = product.reshape(3, normalsResult.rows);
            //back to 0-255 byte 3 channel image
            normalsResult.convertTo(normalsResult, CV_8UC3, 255.0 / 2.0f, 255.0 / 2.0f);
            //End world space conversion
            
            result.outputs.push_back(normalsResult);
            
            for (auto surfaceAnalyzer : renderer->getAnalyzersOfType<CBP_SurfaceAnalyzer>()) {
                std::thread([surfaceAnalyzer, result](){
                    surfaceAnalyzer->normalsDataUpdated(result);
                }).detach();
            }
            
            return true;
        }
    };
    
    CBP_NormalsAnalyzer::CBP_NormalsAnalyzer() : CBP_AreaAnalyzer("CBP_NormalsAnalyzer") {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
        
        setPriority(CBThreadPriorityLow);
    }
    
    CBP_NormalsAnalyzer::~CBP_NormalsAnalyzer() {
        
    }
    
    area_run_params CBP_NormalsAnalyzer::getParams() const {
        return m_pImpl->m_params;
    }
    
    void CBP_NormalsAnalyzer::initialize() {
        m_pImpl->_initialize();
    }
    
    bool CBP_NormalsAnalyzer::analyze(cbar::CBAR_VideoFramePtr frame) {
        return m_pImpl->_analyze(frame);
    }
};
