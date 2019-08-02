//
//  CBP_Renderer.hpp
//  Cambrian
//
//  Created by Joel Teply on 3/21/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBP_Renderer_hpp
#define CBP_Renderer_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>

#include <cambrian.h>

#include <cbar/pipeline/motion/motion.h>
#include <cbar/pipeline/analysis/analysis.h>

namespace cbscene {
    class CBAR_Asset;
}

namespace cbpipe {
    class CBP_RenderingEngine;
    
    class DLL_LOCAL CBP_Renderer : public CBP_AnalyzerThread {
    public:
        CBP_Renderer(std::string name, cbscene::CBAR_Asset *asset=nullptr);
        
        const cbscene::CBAR_Asset* getAsset() const;
        
        //must implement
        virtual PaintPointType getType() = 0;
        
        virtual void render(bool isStillMode) = 0;
        
        std::shared_ptr<CBP_Analyzer> getAnalyzer();
        
        //optional
        virtual void clearAll();
        
        virtual void touchedAtPoint(TouchPoint &touch);
                
        virtual void rotatedBy(cbpipe::TouchPoint &touch);
        
        virtual void rotatedBy(float amount);
        
        virtual void captureToStill() {};
        
        virtual void goingLive() {}; //we'll do it LIVE!
        
        virtual bool isUtilized() { return true;};
        
        virtual void commitChanges() {};
        
        virtual bool saveJSONState(cbscene::CBAR_Scene *scene, Json::Value &assetNode);
        virtual void loadJSONState(cbscene::CBAR_Scene *scene, const Json::Value &assetNode);
        
        virtual bool saveToDirectory(const std::string &location);
        virtual void loadFromDirectory(const std::string &location, int applyRotation);
        
        virtual void captureState(std::shared_ptr<cbpipe::UndoState> state);
        virtual void reloadState(std::shared_ptr<cbpipe::UndoState> state);
        
    protected:
        ~CBP_Renderer();
        
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif /* CBP_Renderer_hpp */
