//
//  CBP_WallFinder.cpp
//  Cambrian
//
//  Created by Joel Teply on 4/17/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#include "CBP_WallFinder.hpp"

#include <cbar/pipeline/rendering/rendering.h>
#include <cbcommon/CB_Config.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Coloring.h>
#include <cbar/pipeline/util/util.h>
#include <imaging/Imaging.h>

#include <cbar/pipeline/machine-learning/machine-learning.h>

#define MAX_NORMALS_SIZE cv::Size(512,512)

using namespace imaging;

namespace cbpipe {
    
    struct CBP_WallFinder::Impl
    {
        Impl(CBP_WallFinder *parent) : m_parent(parent) {
            
        }
        
        ~Impl() {}
        
        CBP_WallFinder *m_parent;
        int64_t m_lastUpdateFrame = -1;
        
        CBMutex m_lastSemanticResultMutex;
        semantic_data m_lastSemanticResult;
        
        CBMutex m_lastWallsResultMutex;
        std::vector<candidate_wall> m_lastWallsResult;
        int64_t m_lastWallsFrameIndex;
        
        int64_t m_lastWallsProcessedFrameIndex = -1;
        
        void _semantic_data_updated(const semantic_data &data) {
            
            m_lastSemanticResultMutex.lock();
            m_lastSemanticResult = data;
            m_lastSemanticResultMutex.unlock();
        }
        
        void _candidate_walls_found(const std::vector<candidate_wall> &walls, int64_t frameIndex) {
            m_lastWallsResultMutex.lock();
            m_lastWallsResult = walls;
            m_lastWallsFrameIndex = frameIndex;
            m_lastWallsResultMutex.unlock();
            
            m_parent->wakeup();
        }
        
        bool _analyze(cbar::CBAR_VideoFramePtr frame) {
            
            auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return false;
            auto surfaceAnalyzers = renderer->getAnalyzersOfType<CBP_SurfaceAnalyzer>(); if (!surfaceAnalyzers.size()) return false;
            auto tracker = renderer->getAnalyzerOfType<CBP_FeatureTracker>(); if (!tracker) return false;
            
            std::vector<cv::Mat> surfaces;
            std::vector<int> counts;
            std::vector<cv::Point2f> directions;
            std::vector<cv::Scalar> colors;
            
            m_lastSemanticResultMutex.lock();
            auto result = m_lastSemanticResult;
            m_lastSemanticResultMutex.unlock();
            
            m_lastWallsResultMutex.lock();
            auto walls = m_lastWallsResult;
            int64_t wallsFrameIndex = m_lastWallsFrameIndex;
            m_lastWallsResultMutex.unlock();
            
            if (result.normals.empty() || walls.empty() || m_lastWallsProcessedFrameIndex == wallsFrameIndex) return false;
            
            m_lastWallsProcessedFrameIndex = wallsFrameIndex;
            
            Diagnostics::SaveDiagnosticMask(result.semantic, result.rgb, "walls.png");
            
            
            
            //            if (surfaces.normalsImage.size().area() > MAX_NORMALS_SIZE.area()) {
            //                cv::resize(surfaces.normalsImage, surfaces.normalsImage, MAX_NORMALS_SIZE);
            //            }
            
            //auto start = sys_usec_time();
            //ImageProcessing::kmeansColor(surfaces.normalsImage, surfaces.normalsImage);
            //CBLog("kmeans took %f seconds", seconds_elapsed(start));
            
            //            cv::Mat debug;
            //            cv::resize(surfaces.normalsImage, surfaces.normalsImage, surfaces.colorImage.size());
            //            cv::addWeighted(surfaces.normalsImage, 0.5, surfaces.colorImage, 0.5, 0.0, debug);
            //            Diagnostics::SaveDiagnosticImage(false, debug, "planes");
            
            return true;
        }
    };
    
    CBP_WallFinder::CBP_WallFinder() : CBP_AreaAnalyzer("CBP_WallFinder") {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
        
        setPriority(CBThreadPriorityLowest);
    }
    
    CBP_WallFinder::~CBP_WallFinder() {
        
    }
    
    bool CBP_WallFinder::analyze(cbar::CBAR_VideoFramePtr frame) {
        return m_pImpl->_analyze(frame);
    }
    
    void CBP_WallFinder::semanticDataUpdated(const semantic_data &semantic) {
        m_pImpl->_semantic_data_updated(semantic);
    }
    
    void CBP_WallFinder::candidateWallsFound(const std::vector<candidate_wall> &walls, int64_t frameIndex) {
        m_pImpl->_candidate_walls_found(walls, frameIndex);
    }
};
