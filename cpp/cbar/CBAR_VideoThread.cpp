//
//  CBAR_VideoThread.cpp
//  Cambrian
//
//  Created by Joel Teply on 3/31/15.
//  Copyright (c) 2015 Joel Teply. All rights reserved.
//

#include "CBAR_VideoThread.hpp"
#include "CBAR_VideoFrame.hpp"
#include <imaging/ImageProcessing.h>

#include <utility/CommonUtility.h>
#include <imaging/Accelerated.h>

namespace cbar {
    
    struct CBAR_VideoThread::Impl
    {
        Impl(CBAR_VideoThread *parent) : m_parent(parent)
        {
            
        }
        ~Impl() {
            
        }
        
        void tick() {
            m_fpsTimer.tick();
            
            if (m_parent->m_logFPS && m_fpsTimer.logFPS()) {
                CBWrite("%s fps: %.2f", m_parent->getThreadName().c_str(), m_parent->getFps());
            }
        }
        
        CBAR_VideoThread *m_parent;
        FPSTimer m_fpsTimer;
    };
    
    CBAR_VideoThread::CBAR_VideoThread(std::string name, int queueSize)
    : QueueThread<CBAR_VideoFramePtr>(name, queueSize)
    {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }
    
    CBAR_VideoThread::~CBAR_VideoThread() {
        
    }
    
    float CBAR_VideoThread::getFps() const {
        return m_pImpl->m_fpsTimer.getFPS();
    }
    
    bool CBAR_VideoThread::handleItem(CBAR_VideoFramePtr frame) {
        if (!frame.empty()) {
            bool didRun = handleFrame(frame);
            return didRun;
        }
        return false;
    };
}
