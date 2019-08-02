//
//  CBP_PlaneAnalyzer.cpp
//  Cambrian
//
//  Created by Joel Teply on 2/19/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#include "CBP_PlaneAnalyzer.hpp"

#include <cbar/pipeline/rendering/rendering.h>
#include <cbcommon/CB_Config.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Coloring.h>

#define INITIAL_GROUND_HEIGHT -1.3
#define MIN_GROUND_HEIGHT 0.7
#define MAX_GROUND_HEIGHT 2.0
#define SMOOTHING_WEIGHT_INITIAL 1.0
#define SMOOTHING_WEIGHT_FINAL 0.3
#define VIBRATE_DEVICE 1

namespace cbpipe {
    
    struct CBP_PlaneAnalyzer::Impl
    {
        Impl(CBP_PlaneAnalyzer *parent) : m_parent(parent) {

        }
        
        ~Impl() {}
        
        CBP_PlaneAnalyzer *m_parent;
        std::map<std::string, PlaneAnchor> m_anchors;
        CBMutex m_anchorsMutex;
        
        Eigen::Vector3f m_groundCenter = Eigen::Vector3f(0,0,0);
        Eigen::Vector3f m_groundNormal = Eigen::Vector3f(0,1,0);
        
        Eigen::Vector3f m_newGroundCenter = Eigen::Vector3f(0,0,0);
        
        bool m_hasGround = false;
        bool m_hasAnchor = false;
        size_t m_numRealGrounds = 0;
        bool m_wasUpdated = false;
        int64_t m_groundFindTime = 0;
        
        bool handleFrame(cbar::CBAR_VideoFramePtr frame) {
            auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return false;
            auto callback = renderer->getCallback(); if (!callback) return false;
            
            if (m_wasUpdated || !m_numRealGrounds) {
                m_wasUpdated = false;
                getUpdatedPlane(frame);
            }
            
            if (!m_newGroundCenter.isZero() || !m_numRealGrounds) {
                
                if (m_groundCenter.isZero()) {
                    m_groundCenter = m_newGroundCenter;
                }
                else if (fabs(m_groundCenter.y() - m_newGroundCenter.y()) > 0.01) {
                    //smooth result
                    double smoothingWeight = m_numRealGrounds > 3 ? SMOOTHING_WEIGHT_INITIAL : SMOOTHING_WEIGHT_INITIAL;
                    m_groundCenter.y() = (1.0f - smoothingWeight) * m_groundCenter.y() + smoothingWeight * m_newGroundCenter.y();
                } else if (m_numRealGrounds) {
                    return true;
                }
            }
            
            return true;
        }
        
        void getUpdatedPlane(cbar::CBAR_VideoFramePtr frame) {
            auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return;
            auto tracker = renderer->getAnalyzerOfType<CBP_FeatureTracker>(); if (!tracker) return;
            Eigen::Matrix4f worldPosition = tracker->getWorldTransform(frame->frameIndex);
            auto camera = worldPosition.inverse(); //IMPORTANT: two steps required to prevent RELEASE ONLY crash
            auto cameraPos = camera.col(3);
            
            Eigen::Vector3f cameraPosition = Eigen::Vector3f(cameraPos.x(), cameraPos.y(), cameraPos.z());
            
            if (cameraPosition.isZero()) return;
            
            auto groundCenter = m_newGroundCenter;
            
            auto anchors = m_parent->getAnchors();
            
            for (const auto &itr : anchors) {
                const auto &anchor = itr.second;
                if (anchor.alignment == PlaneAnchorAlignmentHorizontal) {
                    updateGroundPosition(cameraPosition.y(), anchor.transform, groundCenter);
                    m_hasAnchor = true;
                }
            }
            bool foundGround = anchors.size();
            
//            if (!foundGround && !m_numRealGrounds) {
//                cv::Point2f normalizedPoint(0.5, 0.5);
//                if (auto callback = renderer->getCallback()) {
//                    auto hitResults = callback->hitTestAtPoint(normalizedPoint, HitTestResultTypeHorizontalPlane);
//                    if (hitResults.size()) {
//                        updateGroundPosition(cameraPosition.y(), hitResults[0].worldTransform, groundCenter);
//                    }
//                    foundGround = true;
//                }
//            }
            
            if (foundGround) {
                if (!m_numRealGrounds) {
                    
#if DEBUG & VIBRATE_DEVICE
                    renderer->getCallback()->vibrateDevice();
#endif
                    m_newGroundCenter = groundCenter;
                    m_hasGround = true;
                    m_numRealGrounds = 1;
                    
                    CBLog("REAL ground %f", groundCenter.y());

                    for (auto surfaceAnalyzer : renderer->getAnalyzersOfType<CBP_SurfaceAnalyzer>()) {
                        surfaceAnalyzer->reintegrationStarting(frame);
                    }
                    
                    //send out stabilization. Surfaces must be remapped before the ground center is set
                    for (auto areaAnalyzer : renderer->getAnalyzersOfType<CBP_AreaAnalyzer>()) {
                        areaAnalyzer->systemNowStable(frame, groundCenter);
                    }
                    
                    m_groundCenter = groundCenter;
                    
                    for (auto surfaceAnalyzer : renderer->getAnalyzersOfType<CBP_SurfaceAnalyzer>()) {
                        surfaceAnalyzer->reintegrationCompleted(frame);
                    }
                    
#if DEBUG & VIBRATE_DEVICE
                    renderer->getCallback()->vibrateDevice();
#endif
                    m_groundFindTime = sys_usec_time();
                } else {
                    m_numRealGrounds ++;
                }
                
            }

            //CBLog("Ground elevation: %.2f, camera ele: %.2f", groundCenter.y() - cameraPos.y(), cameraPos.y());
            
            if (m_newGroundCenter.isZero()) {
                //just use distance from camera 1 meter down from camera
                groundCenter = Eigen::Vector3f(cameraPosition.x(), INITIAL_GROUND_HEIGHT, cameraPosition.z());
            }
            
            if (groundCenter != m_newGroundCenter) {
                m_newGroundCenter = groundCenter;
                m_hasGround = true;
                //CBLog("New ground %f", groundCenter.y());
            }
        }
        
        void updateGroundPosition(float cameraElevation,
                                  const Eigen::Matrix4f &transform,
                                  Eigen::Vector3f &groundCenter) {
            
            Eigen::Vector3f position = CBP_MatrixUtil::getPosition(transform);
            float elevation = position.y();
            float totalElevation = cameraElevation - elevation;
            
            if ((!m_hasAnchor || (elevation < groundCenter.y() || abs(elevation - groundCenter.y()) < 0.5))
                && totalElevation > MIN_GROUND_HEIGHT && totalElevation < MAX_GROUND_HEIGHT) {
                groundCenter.y() = position.y();
            }
        }
        
        std::map<std::string, PlaneAnchor> getAnchors() {
            std::lock_guard<CBMutex> lockGuard(m_anchorsMutex);
            return m_anchors;
        }
        
        void anchorAdded(const PlaneAnchor &anchordata) {
            std::lock_guard<CBMutex> lockGuard(m_anchorsMutex);
            m_wasUpdated = true;
            m_anchors[anchordata.anchorIdentifier] = anchordata;
            m_parent->wakeup();
        }
        
        void anchorUpdated(const PlaneAnchor &anchordata) {
            std::lock_guard<CBMutex> lockGuard(m_anchorsMutex);
            m_wasUpdated = true;
            m_anchors[anchordata.anchorIdentifier] = anchordata;
        }
        
        void anchorRemoved(const PlaneAnchor &anchordata) {
            std::lock_guard<CBMutex> lockGuard(m_anchorsMutex);
            auto it = m_anchors.find(anchordata.anchorIdentifier);
            if (it != m_anchors.end()) {
                m_anchors.erase(it);
            }
        }
        
        std::vector<PlaneAnchor> getPlanes(int mask) {
            std::vector<PlaneAnchor> anchors;
            std::lock_guard<CBMutex> lockGuard(m_anchorsMutex);
            for (auto it : m_anchors) {
                if (it.second.alignment & mask) {
                    anchors.push_back(it.second);
                }
            }
            return anchors;
        }
    };
    
    CBP_PlaneAnalyzer::CBP_PlaneAnalyzer() : CBP_AnalyzerThread("CBP_PlaneAnalyzer") {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
        setPriority(CBThreadPriorityHighest);
    }
    
    CBP_PlaneAnalyzer::~CBP_PlaneAnalyzer() {
        
    }
    
    bool CBP_PlaneAnalyzer::handleFrame(cbar::CBAR_VideoFramePtr frame) {
        return m_pImpl->handleFrame(frame);
    }
    
    void CBP_PlaneAnalyzer::anchorAdded(const PlaneAnchor &anchordata) {
        m_pImpl->anchorAdded(anchordata);
    }
    
    void CBP_PlaneAnalyzer::anchorUpdated(const PlaneAnchor &anchordata) {
        m_pImpl->anchorUpdated(anchordata);
    }
    
    void CBP_PlaneAnalyzer::anchorRemoved(const PlaneAnchor &anchordata) {
        m_pImpl->anchorRemoved(anchordata);
    }
    
    void CBP_PlaneAnalyzer::getGroundPlane(Eigen::Vector3f &center, Eigen::Vector3f &normal) {
        center = m_pImpl->m_groundCenter;
        normal = m_pImpl->m_groundNormal;
    }
    
    std::map<std::string, PlaneAnchor> CBP_PlaneAnalyzer::getAnchors() {
        return m_pImpl->getAnchors();
    }
    
    bool CBP_PlaneAnalyzer::hasGroundPlane() const {
        return m_pImpl->m_hasGround;
    }
    
    float CBP_PlaneAnalyzer::secondsStable() const {
        return m_pImpl->m_numRealGrounds ? seconds_elapsed(m_pImpl->m_groundFindTime) : 0.0f;
    }
    
    std::vector<PlaneAnchor> CBP_PlaneAnalyzer::getPlanes(int mask) {
        return m_pImpl->getPlanes(mask);
    }
};
