//
//  CBP_DeepModel.cpp
//  Cambrian
//
//  Created by Joel Teply on 9/5/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#include "CBP_DeepModel.hpp"

#include <cbar/pipeline/pipeline.h>
#include <imaging/ImageProcessing.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Accelerated.h>
#include <utility/Directory.h>
#include <imaging/Imaging.h>
#include <thread>

#include <cbar/pipeline/CBP_Types.hpp>
#include <cbar/pipeline/motion/motion.h>
#include <cbar/pipeline/machine-learning/machine-learning.h>

using namespace imaging;

namespace cbpipe {
      
    struct CBP_DeepModel::Impl
    {
        Impl(CBP_DeepModel *parent) : m_parent(parent) {
            
        }
        
        ~Impl() {}
        
        CBP_DeepModel *m_parent;
        
        //CBMutex m_loaderMutex;
        CBMutex m_tfSessionMutex; //one at a time
        bool m_tfSession = false;
        
        CBP_DeepModelInfo m_modelInfo;
        
        float m_benchmarkedDuration = 0;
        
        void _load_deep_network() {
            auto renderer = CBP_RenderingEngine::sharedInstance();
            if (!renderer) return;
            
            auto callback = renderer->getCallback();
            const auto modelInfo = m_parent->getModelInfo();
            
            callback->loadDeepModel(modelInfo);
            m_tfSession = true;
            CBLog("Deep model '%s' loaded", modelInfo.name.c_str());
        }
        
        void _inference(const cv::Mat &srcImage, const std::map<std::string, cv::Mat> &images, cv::Mat &result) {

            const auto modelInfo = m_parent->getModelInfo();
            
            m_tfSessionMutex.lock();
            if (!m_tfSession) {
                _load_deep_network();
            }
            
            auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) {
                m_tfSessionMutex.unlock();
                return;
            }
            auto callback = renderer->getCallback();
            
            result = callback->predict(modelInfo, images);
            
            //CBP_TensorflowUtils::feedImage(m_tfSession, inputImage, result, modelInfo.input, modelInfo.output, labelCounts);
            m_tfSessionMutex.unlock();
            
            if (result.channels() == 3 && modelInfo.output.flipBGR) {
                cv::cvtColor(result, result, CV_BGR2RGB);
            }

        }
        
    };
    
    CBP_DeepModel::CBP_DeepModel() {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }
    
    CBP_DeepModel::~CBP_DeepModel() {
        
    }
    
    void CBP_DeepModel::loadDeepNetwork() {
        std::thread([&](){
            m_pImpl->m_tfSessionMutex.lock();
            m_pImpl->_load_deep_network();
            m_pImpl->m_tfSessionMutex.unlock();
        }).detach();
    }
    
    float CBP_DeepModel::getBenchmarkedDuration() const {
        return m_pImpl->m_benchmarkedDuration;
    }
    
    void CBP_DeepModel::inference(const cv::Mat &srcImage, const std::map<std::string, cv::Mat> &images, cv::Mat &result) {
        m_pImpl->_inference(srcImage, images, result);
    }
};

