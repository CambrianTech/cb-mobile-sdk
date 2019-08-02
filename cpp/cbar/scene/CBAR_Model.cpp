//
//  CB_ARModel.cpp
//  Cambrian
//
//  Created by Joel Teply on 3/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#include "CBAR_Model.hpp"

namespace cbscene {
    
    struct CBAR_Model::Impl
    {
        Impl(CBAR_Model *)
        {
            
        }
        ~Impl() {
            
        }
    };
    
    CBAR_Model::~CBAR_Model() {
        
    }
    
    CBAR_Model::CBAR_Model(const std::string &assetID, const std::shared_ptr<CBAR_AssetCallbackI>callback) : cbscene::CBAR_Asset(assetID, callback)  {
        CBLog("new CBAR_Model");
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }
    
    bool CBAR_Model::saveJSONState(cbscene::CBAR_Scene *scene, Json::Value &assetNode) {
        
        CBAR_Asset::saveJSONState(scene, assetNode);
        
        return true;//appearance changed since load
    }
    
    void CBAR_Model::loadJSONState(cbscene::CBAR_Scene *scene, const Json::Value &assetNode) {
        CBAR_Asset::loadJSONState(scene, assetNode);
    }
    
    void CBAR_Model::reloadState(std::shared_ptr<cbpipe::UndoState> state) {
        CBAR_Asset::reloadState(state);
    }
    
    bool CBAR_Model::saveToDirectory(const std::string &location) {
        //save model
        return true;
    }
    
    void CBAR_Model::loadFromDirectory(const std::string &location, int applyRotation) {
        //load model
    }
};
