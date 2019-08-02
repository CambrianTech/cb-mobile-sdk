//
//  VideoThread.h
//  Cambrian
//
//  Created by Joel Teply on 3/31/15.
//  Copyright (c) 2015 Joel Teply. All rights reserved.
//

#ifndef __Cambrian__VideoThread__
#define __Cambrian__VideoThread__

#include <memory>
#include <queue>
#include <stdio.h>
#include <cambrian.h>
#include <utility/Threads.h>
#include <cbar/CBAR_VideoFrame.hpp>
#include <cbar/CBAR_Common.hpp>
#include <atomic>

namespace cbar {
    
    class DLL_LOCAL CBAR_VideoThread : public QueueThread<CBAR_VideoFramePtr> {
    public:
        CBAR_VideoThread(std::string name, int queueSize=1);
        
        float getFps() const;
        
        void executionCompleted(int64_t usecsTaken);
    protected:
        virtual ~CBAR_VideoThread();
        virtual bool handleFrame(CBAR_VideoFramePtr frame) { return false; };
        
        bool m_logFPS = false;
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
        virtual bool handleItem(CBAR_VideoFramePtr frame);
    };
}

#endif /* defined(__Cambrian__VideoThread__) */
