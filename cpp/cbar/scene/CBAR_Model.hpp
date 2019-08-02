//
//  CB_ARModel.hpp
//  Cambrian
//
//  Created by Joel Teply on 3/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBAR_Model_hpp
#define CBAR_Model_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <utility/JSONHelper.h>

#include "CBAR_Asset.hpp"

namespace cbscene {
    
    class DLL_PUBLIC CBAR_Model : public CBAR_Asset {
    public:
        CBAR_Model(const std::string &assetID, const std::shared_ptr<CBAR_AssetCallbackI>callback);
        ~CBAR_Model();
        
        virtual asset_type getType() {return asset_type_model;};
        
        virtual void destroyRenderer() {};
        
        virtual void reloadState(std::shared_ptr<cbpipe::UndoState> state);
        
        virtual bool saveJSONState(cbscene::CBAR_Scene *scene, Json::Value &assetNode);
        virtual void loadJSONState(cbscene::CBAR_Scene *scene, const Json::Value &assetNode);
        
        virtual bool saveToDirectory(const std::string &location);
        virtual void loadFromDirectory(const std::string &location, int applyRotation);
        
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}


#endif /* CB_ARModel_hpp */
