//
//  CBP_FeatureTracker.cpp
//  Cambrian
//
//  Created by Joel Teply on 1/18/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#include "CBP_FeatureTracker.hpp"
#include <cbar/pipeline/rendering/rendering.h>
#include <thread>
#define CACHE_SIZE 30000

using namespace imaging;

namespace cbpipe {
    struct CBP_FeatureTracker::Impl
    {
        Impl(CBP_FeatureTracker *parent) : m_parent(parent),
            m_locationCache(CACHE_SIZE), m_projectionCache(CACHE_SIZE), m_headingCache(CACHE_SIZE) {
                
        }
        
        ~Impl() {
            
        }

        CBP_FeatureTracker *m_parent;
        
        std::map<uint64_t, Eigen::Vector3f> m_points;
        std::vector<Eigen::Matrix4f> m_locationCache;
        std::vector<Eigen::Matrix4f> m_projectionCache;
        std::vector<Eigen::Vector3f> m_headingCache;
        uint64_t m_currentFrameIndex;
        
        bool handleFrame(cbar::CBAR_VideoFramePtr frame) {
            
            if (auto renderer = CBP_RenderingEngine::sharedInstance()) {
                if (!renderer->getScene()->hasWorldTransform()) {
                    return false;
                }
                size_t index = frame->frameIndex % CACHE_SIZE;
                m_locationCache[index] = renderer->getScene()->getWorldTransform();
                m_projectionCache[index] = renderer->getScene()->getCameraProjection();
                m_headingCache[index] = renderer->getLastHeading();
                m_currentFrameIndex = frame->frameIndex;
            }
            return true;
        }
        
        void setPointCloudData(const std::map<uint64_t, Eigen::Vector3f> &cloudPoints) {
            for (const auto &itr : cloudPoints) {
                m_points[itr.first] = itr.second;
            }
        }
        
        Eigen::Matrix4f getCameraProjection(int64_t frameIndex) {
            if (frameIndex > m_currentFrameIndex) {
                return  m_projectionCache[m_currentFrameIndex % CACHE_SIZE];
            }
            return m_projectionCache[frameIndex % CACHE_SIZE];
        }
        
        Eigen::Matrix4f getWorldTransform(int64_t frameIndex) {
            if (frameIndex > m_currentFrameIndex) {
                return  m_locationCache[m_currentFrameIndex % CACHE_SIZE];
            }
            return m_locationCache[frameIndex % CACHE_SIZE];
        }
        
        Eigen::Vector3f getHeading(int64_t frameIndex) {
            if (frameIndex > m_currentFrameIndex) {
                return  m_headingCache[m_currentFrameIndex % CACHE_SIZE];
            }
            return m_headingCache[frameIndex % CACHE_SIZE];
        }
    };
    
    CBP_FeatureTracker::CBP_FeatureTracker() : CBP_AnalyzerThread("CBP_FeatureTracker") {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }
    
    CBP_FeatureTracker::~CBP_FeatureTracker() {
        
    }
    
    std::map<uint64_t, Eigen::Vector3f> CBP_FeatureTracker::getPointCloudData() {
        std::map<uint64_t, Eigen::Vector3f> copy;
        std::copy(m_pImpl->m_points.begin(), m_pImpl->m_points.end(), std::inserter(copy, copy.end()) );
        return copy;
    }
    
    void CBP_FeatureTracker::setPointCloudData(const std::map<uint64_t, Eigen::Vector3f> &points) {
        m_pImpl->setPointCloudData(points);
    }
    
    Eigen::Matrix4f CBP_FeatureTracker::getCameraProjection(int64_t frameIndex) {
        return m_pImpl->getCameraProjection(frameIndex);
    }
    
    Eigen::Matrix4f CBP_FeatureTracker::getWorldTransform(int64_t frameIndex) {
        return m_pImpl->getWorldTransform(frameIndex);
    }
    
    Eigen::Vector3f CBP_FeatureTracker::getHeading(int64_t frameIndex) {
        return m_pImpl->getHeading(frameIndex);
    }
    
    bool CBP_FeatureTracker::handleFrame(cbar::CBAR_VideoFramePtr frame) {
        return m_pImpl->handleFrame(frame);
    }
}

