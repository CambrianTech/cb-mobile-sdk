//
//  CBP_ColorSample.cpp
//  Cambrian
//
//  Created by Joel Teply on 11/12/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#include "CBP_AmbienceSampler.hpp"

#include <cbcommon/CB_Config.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Coloring.h>
#include <thread>

#include <cbar/pipeline/rendering/rendering.h>

#define clamp(value, minimum, maximum) fmax(fmin(value, maximum), minimum)

using namespace imaging;
using namespace cbscene;

namespace cbpipe {
    
    static CBMutex m_receiversMutex;
    static std::vector<CBP_AmbienceReceiver *>m_receivers;
    
    void registerReceiver(CBP_AmbienceReceiver* receiver) {
        std::lock_guard<CBMutex> lockGuard(m_receiversMutex);
        m_receivers.push_back(receiver);
    }
    
    void unregisterReceiver(CBP_AmbienceReceiver* receiver) {
        std::lock_guard<CBMutex> lockGuard(m_receiversMutex);
        auto result = std::find(m_receivers.begin(), m_receivers.end(), receiver);
        if (result != m_receivers.end()) {
            m_receivers.erase(result);
        }
    }
    
    struct CBP_AmbienceReceiver::Impl
    {
        Impl(CBP_AmbienceReceiver *parent) : m_parent(parent) {
            //
        }
        
        ~Impl() {
            
        }
        CBP_AmbienceReceiver *m_parent;
        
        CBMutex m_samplesMutex;
        std::vector<stat_sample>m_samples;
        ColorAdjustment m_colorAdjustment;
        stat_sample m_avgSample;
        
        void addSample(const stat_sample &sample, int max) {
            std::lock_guard<CBMutex> lockGuard(m_samplesMutex);
            m_samples.insert(m_samples.begin(), sample);
            
            if (m_samples.size() > max) {
                m_samples.pop_back();
            }
        }
        
        void calculate() {
            
            m_samplesMutex.lock();
            auto samples = m_samples;
            m_samplesMutex.unlock();
            
            std::vector<double>bgIntensities;
            std::vector<double>overlayIntensities;
            std::vector<cv::Scalar>ambientLevels;

            for (const auto &sample : samples) {
                bgIntensities.push_back(sample.bgIntensity);
                overlayIntensities.push_back(sample.overlayIntensity);
                ambientLevels.push_back(sample.ambientLevel);
            }

            stat_sample avgSample;
            avgSample.bgIntensity = cv::mean(bgIntensities)[0];
            avgSample.overlayIntensity = cv::mean(overlayIntensities)[0];
            avgSample.ambientLevel = cv::mean(ambientLevels);
            
            m_colorAdjustment = createColorAdjustment(avgSample.bgIntensity, avgSample.overlayIntensity);
            m_avgSample = avgSample;
        }
        
        ColorAdjustment createColorAdjustment(double backgroundIntensity, double overlayIntensity) {
            ColorAdjustment adjustment;
            
            adjustment.backgroundIntensity = backgroundIntensity;
            
            adjustment.alpha = clamp(backgroundIntensity / overlayIntensity, 1.0f, 1.4f);
            adjustment.beta = clamp(backgroundIntensity - adjustment.alpha * overlayIntensity, 0.0f, 1.0f);
            //*beta = MIN(*beta, 0.3f);
            
            adjustment.gamma = 1.0;
            //adjustment.gamma = clamp(powf(backgroundIntensity, 0.25), 0.75, 0.95);
            
            return adjustment;
        }
        
    };
    
    CBP_AmbienceReceiver::CBP_AmbienceReceiver() {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
        
        registerReceiver(this);
    }
    
    CBP_AmbienceReceiver::~CBP_AmbienceReceiver() {
        unregisterReceiver(this);
    }
    
    void CBP_AmbienceReceiver::addSample(const stat_sample &sample, int max) {
        m_pImpl->addSample(sample, max);
    }
    
    void CBP_AmbienceReceiver::clearSamples() {
        std::lock_guard<CBMutex> lockGuard(m_pImpl->m_samplesMutex);
        m_pImpl->m_samples.clear();
    }
    
    void CBP_AmbienceReceiver::calculate() {
        std::thread([&](){
            m_pImpl->calculate();
            ambienceCalculationUpdated();
        }).detach();
    }
    
    ColorAdjustment CBP_AmbienceReceiver::getColorAdjustment() {
        return m_pImpl->m_colorAdjustment;
    }
    
    bool CBP_AmbienceReceiver::saveJSONState(cbscene::CBAR_Scene *scene, Json::Value& ambienceNode) {
        
        return true;
//        if (!numElements) {
//            CBLog("No samples to save");
//            return true;//nothing to do
//        }
//
//        stat_sample avgSample, totalSample;
//        for (const auto &sample: samples) {
//            totalSample.bgIntensity += sample.bgIntensity;
//            totalSample.ambientLevel += sample.ambientLevel;
//
//            for (int i=0; i<sample.overlayIntensities.size(); i++) {
//                totalSample.overlayIntensities.resize(i+1);
//                totalSample.overlayIntensities[i] += sample.overlayIntensities[i];
//            }
//        }
//
//        //average
//        avgSample.bgIntensity = totalSample.bgIntensity / double(numElements);
//        avgSample.ambientLevel = totalSample.ambientLevel / double(numElements);
//
//        for (int i=0; i<totalSample.overlayIntensities.size(); i++) {
//            avgSample.overlayIntensities.resize(i+1);
//            avgSample.overlayIntensities[i] = totalSample.overlayIntensities[i] / double(numElements);
//        }
//
//        ambienceNode["bgIntensity"] = avgSample.bgIntensity;
//        ambienceNode["ambientLevel"] = JSONHelper::createScalarJSONValue(avgSample.ambientLevel);
//
//        Json::Value overlayIntensitiesNode;
//        for (int i=0; i<avgSample.overlayIntensities.size(); i++) {
//            overlayIntensitiesNode.append(avgSample.overlayIntensities[i]);
//        }
//        ambienceNode["overlayIntensities"] = overlayIntensitiesNode;
//
//        auto adjustments = m_parent->getColorAdjustments();
//        Json::Value colorAdjustmentsNode;
//        for (int i=0; i<adjustments.size(); i++) {
//            auto adjustment = adjustments[i];
//            Json::Value adjustmentNode;
//
//            adjustmentNode["alpha"] = adjustment.alpha;
//            adjustmentNode["beta"] = adjustment.beta;
//            adjustmentNode["gamma"] = adjustment.gamma;
//            adjustmentNode["backgroundIntensity"] = adjustment.backgroundIntensity;
//
//            colorAdjustmentsNode.append(adjustmentNode);
//        }
        
//        ambienceNode["colorAdjustments"] = colorAdjustmentsNode;
//        analysisNode["ambience"] = ambienceNode;
        
    }
    
    void CBP_AmbienceReceiver::loadJSONState(cbscene::CBAR_Scene *scene, const Json::Value& analysisNode) {

//
//        stat_sample avgSample;
//        
//        const Json::Value& ambienceNode = analysisNode["ambience"];
//        
//        if (ambienceNode.isMember("bgIntensity")) {
//            avgSample.bgIntensity = ambienceNode["bgIntensity"].asDouble();
//        }
//        
//        if (ambienceNode.isMember("ambientLevel")) {
//            avgSample.ambientLevel = JSONHelper::getScalarJSONValue(ambienceNode["ambientLevel"]);
//        }
//        
//        if (ambienceNode.isMember("overlayIntensities")) {
//            const Json::Value& overlayIntensitiesNode = ambienceNode["overlayIntensities"];
//            int numIntensities = overlayIntensitiesNode.size();
//            avgSample.overlayIntensities.resize(numIntensities);
//            for (int i = 0; i < numIntensities; i++) {
//                const Json::Value& intensityNode = overlayIntensitiesNode[i];
//                avgSample.overlayIntensities[i] = intensityNode.asDouble();
//            }
//        }
//        
//        m_samplesMutex.lock();
//        m_samples.push_back(avgSample);
//        m_samplesMutex.unlock();
//        
//        if (ambienceNode.isMember("colorAdjustments")) {
//            const Json::Value& adjustmentsNode = ambienceNode["colorAdjustments"];
//            int numAdjustments = adjustmentsNode.size();
//            m_colorAdjustmentsMutex.lock();
//            m_colorAdjustments.clear();
//            m_colorAdjustments.resize(numAdjustments);
//            for (int i = 0; i < numAdjustments; i++) {
//                const Json::Value& adjustmentNode = adjustmentsNode[i];
//                ColorAdjustment adjustment;
//                if (adjustmentNode.isMember("alpha")) {
//                    adjustment.alpha = adjustmentNode["alpha"].asDouble();
//                }
//                if (adjustmentNode.isMember("beta")) {
//                    adjustment.beta = adjustmentNode["beta"].asDouble();
//                }
//                if (adjustmentNode.isMember("gamma")) {
//                    adjustment.gamma = adjustmentNode["gamma"].asDouble();
//                }
//                if (adjustmentNode.isMember("backgroundIntensity")) {
//                    adjustment.backgroundIntensity = adjustmentNode["backgroundIntensity"].asDouble();
//                }
//                m_colorAdjustments[i] = adjustment;
//            }
//            m_colorAdjustmentsMutex.unlock();
//        }
    }
    
    struct CBP_AmbienceSampler::Impl
    {
        Impl(CBP_AmbienceSampler *parent) : m_parent(parent) {
            
        }
        
        ~Impl() {
            
        }
        
        CBP_AmbienceSampler *m_parent;
        
        bool m_needsRecalculation = false;
        
        bool handleFrame(cbar::CBAR_VideoFramePtr frame) {
            
            auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return false;
            
            if (!renderer->getARView()
                || (frame->isVideoFrame() && frame->frameIndex < 60)) return false;
            
            if (m_needsRecalculation) {
                m_needsRecalculation = false;
                m_parent->flush();
                
                m_receiversMutex.lock();
                auto receivers = m_receivers;
                m_receiversMutex.unlock();
                
                for (CBP_AmbienceReceiver *receiver : receivers) {
                    receiver->clearSamples();
                }
            }
            
            calculateColorAdjustments(frame);
            
            return true;
        }
        
        void calculateColorAdjustments(cbar::CBAR_VideoFramePtr frame) {
            
            m_receiversMutex.lock();
            auto receivers = m_receivers;
            m_receiversMutex.unlock();
            
            if (!receivers.size()) return;
            
            //calculate overlay's intensity
            cv::Size sampleSize = cv::Size(200,200);
            cv::Mat canny, reducedBW, reducedRGB;
            
            cv::resize(frame->getBWImage(), reducedBW, sampleSize);
            cv::resize(frame->getRGBImage(), reducedRGB, sampleSize);
            cv::Canny(reducedBW, canny, 45, 250);
            
            double bgIntensity = ImageProcessing::getBackgroundIntensity(reducedBW, canny);
            cv::Scalar ambientLevel  = ImageProcessing::getWhiteBalanceGainAdjustment(reducedRGB, canny);
            
            int maxSampleSize = frame->isVideoFrame() ? INTENSITY_CALC_SAMPLE_SIZE : 1;
            
            for (CBP_AmbienceReceiver *receiver : receivers) {
                cv::Mat reducedMask;
                cv::Mat mask = receiver->getMask();
                if (mask.empty()) continue;
                
                cv::resize(mask, reducedMask, sampleSize);
                
                stat_sample sample;
                sample.bgIntensity = bgIntensity;
                sample.ambientLevel = ambientLevel;
                sample.overlayIntensity = ImageProcessing::getOverlayIntensity(reducedBW, reducedMask);
                
                receiver->addSample(sample, maxSampleSize);
                receiver->calculate();
            }
        }
    };
    
    CBP_AmbienceSampler::CBP_AmbienceSampler() : CBP_AnalyzerThread("CBP_AmbienceSampler") {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
        setPriority(CBThreadPriorityLowest);
    }
    
    CBP_AmbienceSampler::~CBP_AmbienceSampler() {
        
    }
    
    void CBP_AmbienceSampler::recalculate() {
        m_pImpl->m_needsRecalculation = true;
        wakeup();
    }
    
    bool CBP_AmbienceSampler::handleFrame(cbar::CBAR_VideoFramePtr frame) {
        return m_pImpl->handleFrame(frame);
    }
};

