//
//  CBP_DeepAnalyzer.cpp
//  Cambrian
//
//  Created by Joel Teply on 9/7/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#include "CBP_AreaAnalyzer.hpp"

#include <cbar/pipeline/rendering/rendering.h>
#include <cbcommon/CB_Config.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Coloring.h>
#include <cbar/pipeline/util/util.h>

#include <cbar/pipeline/machine-learning/machine-learning.h>

using namespace imaging;

namespace cbpipe {
    
    struct CBP_AreaAnalyzer::Impl
    {
        Impl(CBP_AreaAnalyzer *parent) : m_parent(parent) {
            
        }
        
        ~Impl() {}
        
        CBP_AreaAnalyzer *m_parent;
        
        Eigen::Vector3f m_lastRunPosition;
        int64_t m_lastRunTime = 0;
        double m_lastRunDuration = 0;
        bool m_busy = false;
        bool m_isInitialized = false;
        int m_numRuns = 0;
        int64_t m_startTime = 0;
        double m_startupSeconds = 3.0;
        
        bool _handle_frame(cbar::CBAR_VideoFramePtr frame) {
            
            if (!m_startTime) {
                m_startTime = sys_usec_time();
            }
            
            if (seconds_elapsed(m_startTime) < m_startupSeconds) return false;
            
            auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return false;
            if (!m_isInitialized) {
                m_parent->initialize();
                m_isInitialized = true;
            }
            
            auto tracker = renderer->getAnalyzerOfType<CBP_FeatureTracker>(); if (!tracker) return false;
            
            area_run_params params = m_parent->getParams();
            float angularSpeed = renderer->getAngularSpeed(5);
            
            if (angularSpeed > params.max_rotation_velocity) return false;
            
            auto forward2 = renderer->unprojectPoint(cv::Point(frame->frameSize().width / 2,
                                                               frame->frameSize().height / 2), 2.0, frame->frameIndex);
            
            if (m_lastRunPosition.isZero()) {
                m_lastRunPosition = forward2;
            }
            
            auto distanceMoved = (forward2 - m_lastRunPosition).norm();
            
            bool movedFarEnough = distanceMoved > params.min_run_distance;
            double minDuration = params.min_run_seconds + m_lastRunDuration;
            bool beenLongEnough = seconds_elapsed(m_lastRunTime) > minDuration;

            if (!movedFarEnough && !beenLongEnough) return false;
            
            Eigen::Matrix4f worldPosition = tracker->getWorldTransform(frame->frameIndex);
            Eigen::Matrix3f worldRotation = worldPosition.block<3,3>(0,0);
            Eigen::Matrix3f cameraRotation = worldRotation.inverse();
            
            Eigen::Vector3f upVector(0,1,0);
            
            Eigen::Vector3f rotationVector = cameraRotation * upVector;
            double angleWithUp = CBP_MatrixUtil::angleBetweenVectors(upVector, rotationVector);
            
            if (angleWithUp > params.max_pitch_radians) return false;
            
            m_lastRunPosition = forward2;
            //auto startTime = sys_usec_time();
            
            //do stuff
#if LOG_DEEP_RUNS
            auto start = sys_usec_time();
#endif
            
#if LOG_DEEP_INFO
            if (movedFarEnough) {
                CBLog("%s network has moved enough to run, running...", m_parent->getThreadName().c_str());
            } else if (beenLongEnough) {
                CBLog("%s network hasn't been run within %.2f seconds, running...", m_parent->getThreadName().c_str(), params.min_run_seconds);
            }
#endif
            
            auto startTime = sys_usec_time();
            bool result = m_parent->analyze(frame);
            //CBLog("%s network took %.2f seconds", m_parent->getThreadName().c_str(), seconds_elapsed(startTime));
            
            if (result) {
                m_numRuns ++;
                m_lastRunDuration = seconds_elapsed(startTime);
                m_lastRunTime = sys_usec_time();
#if LOG_DEEP_RUNS
                CBLog("Analyzer %s took %f seconds, has run %d times", m_parent->getThreadName().c_str(), seconds_elapsed(start), m_numRuns);
#endif
            }
#if LOG_DEEP_NON_RUNS
            else {
                CBLog("Analyzer %s did not run. Has run %d times", m_parent->getThreadName().c_str(), m_numRuns);
            }
#endif
            
            return result;
        }
    };
    
    CBP_AreaAnalyzer::CBP_AreaAnalyzer(const std::string &name) : CBP_AnalyzerThread(name) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
        
        setPriority(CBThreadPriorityLow);
    }
    
    CBP_AreaAnalyzer::~CBP_AreaAnalyzer() {
        
    }
    
    bool CBP_AreaAnalyzer::handleFrame(cbar::CBAR_VideoFramePtr frame) {
        return m_pImpl->_handle_frame(frame);
    }
    
    void CBP_AreaAnalyzer::needsRefresh() {
        m_pImpl->m_lastRunTime = 0;
    }
};
