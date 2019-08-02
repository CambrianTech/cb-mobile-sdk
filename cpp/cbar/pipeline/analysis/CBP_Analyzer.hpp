//
//  CBP_Analyzer.hpp
//  Cambrian
//
//  Created by Joel Teply on 10/12/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#ifndef CBP_Analyzer_hpp
#define CBP_Analyzer_hpp

#include <stdio.h>

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <cbar/CBAR_VideoThread.hpp>
#include <utility/JSONHelper.h>

namespace cbscene {
    class CBAR_Scene;
}

namespace cbpipe {
    
    class CBP_RenderingEngine;
    class CBP_Analyzer;
    
    class DLL_LOCAL CBP_AnalyzerThread : public cbar::CBAR_VideoThread {
        friend class CBP_Analyzer;
    public:
        CBP_AnalyzerThread(std::string name) : cbar::CBAR_VideoThread(name) {
            CBLogAllocation("Allocating CBP_AnalyzerThread %s", getThreadName().c_str());
        };
        ~CBP_AnalyzerThread() {
            CBLogAllocation("Deallocating CBP_AnalyzerThread %s", getThreadName().c_str());
        };
        
        virtual bool needsColorFrames() const { return true;};
        virtual bool needsRealTime() const { return false;};
        virtual bool videoOnly() const { return false;};
        virtual bool handleFrame(cbar::CBAR_VideoFramePtr frame) { return false; };
        
        virtual void captureCurrentState() {};
        
        virtual void loadJSONState(cbscene::CBAR_Scene *scene, const Json::Value& analysisNode) {}
        virtual bool saveJSONState(cbscene::CBAR_Scene *scene, Json::Value& analysisNode) { return true;};
        
    };
    
    class DLL_LOCAL CBP_Analyzer : public cbar::CBAR_VideoThread {
    public:
        CBP_Analyzer();
        ~CBP_Analyzer();
                
        virtual void addItem(cbar::CBAR_VideoFramePtr item);
        
        virtual void start();
        virtual bool isRunning();
        
        void analyzeFrame(cbar::CBAR_VideoFramePtr frame);
        
        std::vector<std::shared_ptr<CBP_AnalyzerThread>> getAnalyzers();
        
        template<class T> std::vector<std::shared_ptr<T>> getAnalyzersOfType() {
            
            std::vector<std::shared_ptr<T>>analyzers;
            
            for (auto &analyzer : getAnalyzers()) {
                
                if (const auto castedAnalyzer = std::dynamic_pointer_cast<T>(analyzer)) {
                    analyzers.push_back(castedAnalyzer);
                }
            }
            
            return analyzers;
        }
        
        template<class T> std::shared_ptr<T> ofType() {
            
            for (const auto &analyzer : getAnalyzers()) {
                
                if (auto castedAnalyzer = std::dynamic_pointer_cast<T>(analyzer)) {
                    return castedAnalyzer;
                }
            }
            
            return std::shared_ptr<T>();
        }
        
        bool appendAnalyzer(std::shared_ptr<CBP_AnalyzerThread> analyzer);
        bool removeAnalyzer(std::shared_ptr<CBP_AnalyzerThread> analyzer);
        
        void captureCurrentState();
        
        void loadJSONState(cbscene::CBAR_Scene *scene, const Json::Value& analysisNode);
        bool saveJSONState(cbscene::CBAR_Scene *scene, Json::Value& analysisNode);
        
        virtual void flush();
        
    private:
        
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
        
    };
}

#endif /* CBP_Analyzer_hpp */
