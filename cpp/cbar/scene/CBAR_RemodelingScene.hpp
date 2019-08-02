//
//  CBAR_RemodelingScene.hpp
//  Cambrian
//
//  Created by Joel Teply on 3/23/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBAR_RemodelingScene_hpp
#define CBAR_RemodelingScene_hpp

#include "CBAR_Scene.hpp"
#include <utility/JSONHelper.h>

namespace cbscene {
    
    class DLL_PUBLIC CBAR_RemodelingScene : public CBAR_Scene {
    public:
        CBAR_RemodelingScene(std::shared_ptr<cbscene::CBAR_SceneCallbackI> callback, const std::string &sceneID = newUUID(), const void *context=nullptr);
        CBAR_RemodelingScene(std::shared_ptr<cbscene::CBAR_SceneCallbackI> callback, const cv::Mat &rgbaImage, const std::string &sceneID = newUUID(), const void *context=nullptr);
        
        ~CBAR_RemodelingScene();
        
        virtual bool appendAsset(std::shared_ptr<cbscene::CBAR_Asset> asset);
        
        virtual void saveToDirectory(const std::string &location, bool compressed, std::string &finalPath, std::string &projectJSON);
        virtual std::string loadFromDirectory(const std::string &location, int outputRotation, const Json::Value* sceneNode=nullptr);
        
        const cv::Mat& getPositiveMask();
        const cv::Mat& getNegativeMask();
        
        bool isMasked() const;
        
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif /* CBAR_RemodelingScene_hpp */
