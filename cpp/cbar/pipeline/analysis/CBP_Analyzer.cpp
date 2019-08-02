//
//  CBP_Analyzer.cpp
//  Cambrian
//
//  Created by Joel Teply on 10/12/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#include "CBP_Analyzer.hpp"

#include <cbar/pipeline/motion/motion.h>
#include <cbar/pipeline/rendering/rendering.h>
#include <utility/Directory.h>

using namespace cbar;

namespace cbpipe {
    
    struct CBP_Analyzer::Impl
    {        
        Impl(CBP_Analyzer *)
        {
            if (getCBConfig().mode != CBOperationModeServer) {
                appendAnalyzer(std::shared_ptr<CBP_AnalyzerThread>(new CBP_FeatureTracker));
                
                //deep networks:
                appendAnalyzer(std::shared_ptr<CBP_AnalyzerThread>(new CBP_SemanticAnalyzer));
                appendAnalyzer(std::shared_ptr<CBP_AnalyzerThread>(new CBP_ShadowsAnalyzer));
                
                //structural analysis
                appendAnalyzer(std::shared_ptr<CBP_AnalyzerThread>(new CBP_WallFinder));
                appendAnalyzer(std::shared_ptr<CBP_AnalyzerThread>(new CBP_GroundSurfaceAnalyzer));
                appendAnalyzer(std::shared_ptr<CBP_AnalyzerThread>(new CBP_PlaneAnalyzer));
                appendAnalyzer(std::shared_ptr<CBP_AnalyzerThread>(new CBP_LineFinder));
            }
        }
        ~Impl() {
            
        }
        
        CBMutex m_analyzersMutex;
        bool m_isRunning = false;
       
        std::vector<std::shared_ptr<CBP_AnalyzerThread>> m_analyzers;
        
        size_t appendAnalyzer(std::shared_ptr<CBP_AnalyzerThread> analyzer) {            
            size_t index;
            { // scope the lock_guard
                std::lock_guard<CBMutex> lockGuard(m_analyzersMutex);
                index = m_analyzers.size();
                m_analyzers.push_back(analyzer);
            }
            if (m_isRunning) {
                analyzer->start();
            }
            return index;
        }
        
        bool removeAnalyzer(std::shared_ptr<CBP_AnalyzerThread> analyzer) {
            if (!analyzer.get()) return false;
            
            analyzer->abort();
            bool success = false;
            
            std::lock_guard<CBMutex> lockGuard(m_analyzersMutex);
            
            auto result = std::find(m_analyzers.begin(), m_analyzers.end(), analyzer);
            if (result != m_analyzers.end()) {
                m_analyzers.erase(result);
                success = true;
            }
            
            return success;
        }
        
        void analyzeFrame(cbar::CBAR_VideoFramePtr frame) {
            for (auto &analyzer : getAnalyzers()) {
                
                if (!analyzer->videoOnly() && !frame.empty()) {
                    //CBLog("analyzeFrame::%s", analyzer->getThreadName().c_str());
                    analyzer->handleFrame(frame);
                }
            }
        }
        
        void addItem(cbar::CBAR_VideoFramePtr frame) {
            
            //handle real time first
            for (auto &analyzer : getAnalyzers()) {
                if (!analyzer || (!frame->isVideoFrame() && analyzer->videoOnly()) || !analyzer->needsRealTime()) continue;
                analyzer->handleFrame(frame);
            }
            
            //handle delayed
            for (auto &analyzer : getAnalyzers()) {
                if (!analyzer || (!frame->isVideoFrame() && analyzer->videoOnly())
                    || analyzer->needsRealTime() || frame->frameIndex % 3 != 0) continue;
                analyzer->addItem(frame);
            }
        }
        
        std::vector<std::shared_ptr<CBP_AnalyzerThread>> getAnalyzers() {
            std::lock_guard<CBMutex> lockGuard(m_analyzersMutex);
            auto analyzers = m_analyzers;
            return analyzers;
        }
        
        void startRunning() {
            if (m_isRunning) {
                return;
            }
            
            for (auto &analyzer : getAnalyzers()) {
                analyzer->start();
            }
            
            m_isRunning = true;
        }
        
        void stopRunning() {
            std::vector<std::shared_ptr<CBP_AnalyzerThread>> analyzers = getAnalyzers();
            
            for (auto &analyzer : analyzers) {
                analyzer->abort();
            }
            
            for (auto &analyzer : analyzers) {
                analyzer->join();
            }
            
            m_isRunning = false;
        }
        
        void captureCurrentState() {
            for (auto &analyzer : getAnalyzers()) {
                analyzer->captureCurrentState();
                analyzer->flush();
            }
        }
        
        void loadJSONState(cbscene::CBAR_Scene *scene, const Json::Value& analysisNode) {
            for (auto &analyzer : getAnalyzers()) {
                analyzer->loadJSONState(scene, analysisNode);
            }
        }
        
        bool saveJSONState(cbscene::CBAR_Scene *scene, Json::Value& analysisNode) {
            for (auto &analyzer : getAnalyzers()) {
                
                bool success = analyzer->saveJSONState(scene, analysisNode);
                if (!success) {
                    CBError("saveJSONState::%s failed", analyzer->getThreadName().c_str());
                    return false;
                }
            }
            
            return true;
        }
    };
    
    CBP_Analyzer::CBP_Analyzer() : cbar::CBAR_VideoThread("CBP_Analyzer")  {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }
    
    CBP_Analyzer::~CBP_Analyzer() {
        m_pImpl->stopRunning();
    }
    
    void CBP_Analyzer::flush() {
        cbar::CBAR_VideoThread::flush();
        
        for ( auto &analyzer : getAnalyzers()) {
            analyzer->flush();
        }
    }
    
    void CBP_Analyzer::addItem(cbar::CBAR_VideoFramePtr item) {
        cbar::CBAR_VideoThread::addItem(item);
        
        m_pImpl->addItem(item);
    }
    
    void CBP_Analyzer::start() {
        cbar::CBAR_VideoThread::start();
        m_pImpl->startRunning();
    }

    bool CBP_Analyzer::isRunning() {
        return m_pImpl->m_isRunning;
    }
    
    bool CBP_Analyzer::appendAnalyzer(std::shared_ptr<CBP_AnalyzerThread> analyzer) {
        return m_pImpl->appendAnalyzer(analyzer);
    }
    
    bool CBP_Analyzer::removeAnalyzer(std::shared_ptr<CBP_AnalyzerThread> analyzer) {
        return m_pImpl->removeAnalyzer(analyzer);
    }
    
    void CBP_Analyzer::captureCurrentState() {
        m_pImpl->captureCurrentState();
    }
    
    std::vector<std::shared_ptr<CBP_AnalyzerThread>> CBP_Analyzer::getAnalyzers() {
        return m_pImpl->getAnalyzers();
    }
    
    void CBP_Analyzer::loadJSONState(cbscene::CBAR_Scene *scene, const Json::Value& analysisNode) {
        m_pImpl->loadJSONState(scene, analysisNode);
    }
    
    bool CBP_Analyzer::saveJSONState(cbscene::CBAR_Scene *scene, Json::Value& analysisNode) {
        return m_pImpl->saveJSONState(scene, analysisNode);
    }
    
    void CBP_Analyzer::analyzeFrame(cbar::CBAR_VideoFramePtr frame) {
        m_pImpl->analyzeFrame(frame);
    }
};
