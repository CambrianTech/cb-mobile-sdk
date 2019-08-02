//
//  CBP_Renderer.cpp
//  Cambrian
//
//  Created by Joel Teply on 3/21/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#include "CBP_Renderer.hpp"

#include <cbar/CBAR_View.hpp>

namespace cbpipe {
    
    struct CBP_Renderer::Impl
    {
        Impl(CBP_Renderer *renderer, cbscene::CBAR_Asset *asset)
        : m_renderer(renderer),  m_asset(asset) {
            
        }
        
        ~Impl() {
            m_asset = nullptr;
        }
        
        CBP_Renderer *m_renderer;
        cbscene::CBAR_Asset *m_asset = nullptr;
    };
    
    CBP_Renderer::CBP_Renderer(std::string name, cbscene::CBAR_Asset *asset) : CBP_AnalyzerThread(name) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, asset));
    }
    
    CBP_Renderer::~CBP_Renderer() {
        
    }
    
    const cbscene::CBAR_Asset* CBP_Renderer::getAsset() const {
        return m_pImpl->m_asset;
    }
    
    void CBP_Renderer::touchedAtPoint(TouchPoint &touch) {
        
    }
    
    void CBP_Renderer::rotatedBy(TouchPoint &touch) {
        
    }

    void CBP_Renderer::rotatedBy(float amount) {

    }
    
    void CBP_Renderer::clearAll() {
        
    }
    
    bool CBP_Renderer::saveJSONState(cbscene::CBAR_Scene *scene, Json::Value &assetNode) {
        return false;
    }
    
    void CBP_Renderer::loadJSONState(cbscene::CBAR_Scene *scene, const Json::Value &assetNode) {
        
    }
    
    bool CBP_Renderer::saveToDirectory(const std::string &location) {
        return false;
    }
    
    void CBP_Renderer::loadFromDirectory(const std::string &location, int applyRotation) {
        
    }
    
    void CBP_Renderer::captureState(std::shared_ptr<cbpipe::UndoState> state) {
        
    }
    
    void CBP_Renderer::reloadState(std::shared_ptr<cbpipe::UndoState> state) {
        
    }
}
