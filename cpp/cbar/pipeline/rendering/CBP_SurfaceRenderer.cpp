//
//  CBP_SurfaceRenderer.cpp
//  Cambrian
//
//  Created by Joel Teply on 11/13/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#include "CBP_SurfaceRenderer.hpp"
#include "CBP_RenderingEngine.hpp"

#include <cbar/scene/scene.h>
#include <cbar/pipeline/motion/motion.h>

#include <cbar/CBAR_View.hpp>
#include <imaging/ImageProcessing.h>
#include <imaging/Accelerated.h>
#include <utility/JSONHelper.h>
#include <imaging/Imaging.h>
#include <cbar/scene/CBAR_SurfaceAsset.hpp>
#include <thread>

#define MIN_PAINT_AREA 600
#define MAX_MASK_AGE 3

using namespace imaging;
using namespace cbar;

namespace cbpipe {
    
    static bool occupiedIndexes[] = {0,0,0,0};
    CBMutex occupiedIndexesMutex;
    static int getNextIndex() {
        std::lock_guard<CBMutex> lockGuard(occupiedIndexesMutex);
        for (int i=0; i<4; i++) {
            if (!occupiedIndexes[i]) {
                occupiedIndexes[i] = true;
                return i;
            }
        }
        return -1;
    }
    
    static void releaseOccupiedIndex(int index) {
        std::lock_guard<CBMutex> lockGuard(occupiedIndexesMutex);
        occupiedIndexes[index] = false;
    }
    
    struct CBP_SurfaceRenderer::Impl
    {
        Impl(CBP_SurfaceRenderer *surfaceRenderer)
            : m_parent(surfaceRenderer) {
            
            m_index = getNextIndex();
        }
        
        ~Impl() {
            m_parent = 0;
        }
        
        CBP_SurfaceRenderer *m_parent = 0;
        
        int m_index = 0;
        
        void _save_current_state() {
            
        }
        
        void _capture_state(std::shared_ptr<cbpipe::UndoState> state) {

        }
        
        void _reload_state(std::shared_ptr<cbpipe::UndoState> state) {
            
        }
       
        void _render(bool isStillMode) {
            
        }
        
        void _touched_at_point(TouchPoint &touch) {
            
        }
        
        bool _handle_frame(cbar::CBAR_VideoFramePtr frame) {
            return true;
        }
    };
    
    CBP_SurfaceRenderer::CBP_SurfaceRenderer(cbscene::CBAR_Asset *asset, std::string name) : CBP_Renderer(name, asset) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
        
        setPriority(CBThreadPriorityHigh);
    }
    
    CBP_SurfaceRenderer::~CBP_SurfaceRenderer() {
        releaseIndex();
    }
    
    bool CBP_SurfaceRenderer::handleFrame(cbar::CBAR_VideoFramePtr frame) {
        auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return false;
        
        bool isVideoMode = !renderer->isStillMode();
        if (isVideoMode != frame->isVideoFrame()) {
            return false;
        }
        
        return m_pImpl->_handle_frame(frame);
    }
    
    void CBP_SurfaceRenderer::touchedAtPoint(TouchPoint &touch) {
        m_pImpl->_touched_at_point(touch);
    }
    
    void CBP_SurfaceRenderer::clearAll() {
        
    }

    void CBP_SurfaceRenderer::captureState(std::shared_ptr<cbpipe::UndoState> state) {
        m_pImpl->_capture_state(state);
    }
    
    void CBP_SurfaceRenderer::reloadState(std::shared_ptr<cbpipe::UndoState> state) {
        CBP_Renderer::reloadState(state);
        
        m_pImpl->_reload_state(state);
    }
    
    int CBP_SurfaceRenderer::getIndex() {
        return m_pImpl->m_index;
    }
    
    void CBP_SurfaceRenderer::releaseIndex() {
        releaseOccupiedIndex(m_pImpl->m_index);
    }
    
    void CBP_SurfaceRenderer::render(bool isStillMode) {
        m_pImpl->_render(isStillMode);
    }

    cv::Mat CBP_SurfaceRenderer::getMask() {
        return cv::Mat();
    }
    
    void CBP_SurfaceRenderer::ambienceCalculationUpdated() {
        
    }
}

