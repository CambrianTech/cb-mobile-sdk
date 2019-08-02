//
//  CB_ARFloor.cpp
//  Cambrian
//
//  Created by Joel Teply on 3/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#include "CBAR_Floor.hpp"

#include <cbar/pipeline/analysis/analysis.h>
#include <cbar/pipeline/rendering/rendering.h>

namespace cbscene {
    
    struct CBAR_Floor::Impl
    {
        Impl(CBAR_Floor *parent) {
              m_parent = parent;
        }

       ~Impl() { }
        
        cbscene::CBAR_Floor* m_parent;
        CBMutex m_rendererMutex;
        
        float m_assetScale = 1.0f;
        bool m_needsUpload = false;
        
        float m_rotation = 0.0f;
        Eigen::Vector3f m_position = Eigen::Vector3f(0, -1.0, 0);
    };
    
    CBAR_Floor::~CBAR_Floor() {
        
    }
    
    CBAR_Floor::CBAR_Floor(const std::string &assetID, const std::shared_ptr<CBAR_SurfaceAssetCallbackI>callback)
        : cbscene::CBAR_SurfaceAsset(assetID, callback,
                                     cv::Size(1024, 1024),
                                     HitTestResultTypeHorizontalPlane,
                                     std::vector<int>(FLOOR_VALUES))  {
        //CBLog("new CBAR_Floor");
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }
    
    void CBAR_Floor::reloadState(std::shared_ptr<cbpipe::UndoState> state) {
        CBAR_SurfaceAsset::reloadState(state);
    }
    
    bool CBAR_Floor::saveJSONState(cbscene::CBAR_Scene *scene, Json::Value &assetNode) {
        CBAR_SurfaceAsset::saveJSONState(scene, assetNode);
        return true;//appearance changed since load
    }
    
    void CBAR_Floor::loadJSONState(cbscene::CBAR_Scene *scene, const Json::Value &assetNode) {
        CBAR_SurfaceAsset::loadJSONState(scene, assetNode);
    }
};
