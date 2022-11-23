//
//  CBAR_MaskedAsset.cpp
//  Cambrian
//
//  Created by Joel Teply on 3/15/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#include "CBAR_SurfaceAsset.hpp"
#include <cbar/pipeline/pipeline.h>
#include <cbar/pipeline/rendering/rendering.h>

using namespace cbpipe;

namespace cbscene {
            
    struct CBAR_SurfaceAsset::Impl
    {
        Impl(CBAR_SurfaceAsset *parent, cv::Size textureSize, HitTestResultType planeType, const std::vector<int> &labelTypes)
            : m_parent(parent), m_textureSize(textureSize), m_planeType(planeType), m_labelTypes(labelTypes)
        {
            if (auto renderer = CBP_RenderingEngine::sharedInstance()) {
                m_surfaceAnalyzer = std::shared_ptr<CBP_SurfaceAnalyzer>(new CBP_SurfaceAnalyzer(parent));
                renderer->appendAnalyzer(m_surfaceAnalyzer);
            }
        }
        ~Impl() {
            if (auto renderer = CBP_RenderingEngine::sharedInstance()) {
                renderer->removeAnalyzer(m_surfaceAnalyzer);
            }
        }
        
        CBAR_SurfaceAsset *m_parent;
        
        HitTestResultType m_planeType;
        std::vector<int> m_labelTypes;
        cv::Size m_textureSize;
        
        std::shared_ptr<cbpipe::CBP_SurfaceRenderer> m_renderer;
        std::shared_ptr<cbpipe::CBP_SurfaceAnalyzer> m_surfaceAnalyzer;
                           
        CBMutex m_rendererMutex;
        
        float m_assetScale = 1.0f;
        bool m_needsUpload = false;
        
        float m_rotation = 0.0f;
        Eigen::Vector3f m_position = Eigen::Vector3f(0, -1.0, 0);
        
        std::shared_ptr<cbpipe::CBP_SurfaceRenderer> _get_renderer() {
            std::lock_guard<CBMutex> lockGuard(m_rendererMutex);
            if (!m_renderer) {
                auto *renderer = new cbpipe::CBP_SurfaceRenderer(m_parent, "CBP_SurfaceRenderer");
                m_renderer = std::shared_ptr<cbpipe::CBP_SurfaceRenderer>(renderer);
            }
            return m_renderer;
        }
        
        void _destroy_renderer() {
            if (m_renderer) {
                m_renderer->releaseIndex();
                m_renderer.reset();
                m_renderer = nullptr;
            }
        }
        
        void _attach(cbpipe::CBP_RenderingEngine *engine) {
            if (auto renderer = CBP_RenderingEngine::sharedInstance()) {
                renderer->appendAnalyzer(_get_renderer());
            }
        }
        
        void _detach(cbpipe::CBP_RenderingEngine *engine) {
            if (auto renderer = CBP_RenderingEngine::sharedInstance()) {
                renderer->removeAnalyzer(_get_renderer());
            }
        }
        
        CBAR_SurfaceAsset *m_asset;
    };
    
    CBAR_SurfaceAsset::CBAR_SurfaceAsset(const std::string &assetID,
                                         std::shared_ptr<CBAR_SurfaceAssetCallbackI>callback,
                                         cv::Size textureSize,
                                         HitTestResultType planeType,
                                         const std::vector<int> &labelTypes)
        : CBAR_Asset(assetID, callback) {
            m_pImpl = std::unique_ptr<Impl>(new Impl(this, textureSize, planeType, labelTypes));
    }
    
    CBAR_SurfaceAsset::~CBAR_SurfaceAsset() {
        auto renderer = CBP_RenderingEngine::sharedInstance();
        if (m_pImpl->m_renderer && renderer) {
            renderer->removeAnalyzer(m_pImpl->m_renderer);
        }
    }
    
    std::shared_ptr<cbpipe::CBP_Renderer> CBAR_SurfaceAsset::getRenderer() {
        return getSurfaceRenderer();
    }
    
    std::shared_ptr<cbpipe::CBP_SurfaceRenderer> CBAR_SurfaceAsset::getSurfaceRenderer() {
        return m_pImpl->_get_renderer();
    }
    
    void CBAR_SurfaceAsset::destroyRenderer() {
        m_pImpl->_destroy_renderer();
    }
    
    int CBAR_SurfaceAsset::getIndex() {
        return getSurfaceRenderer()->getIndex();
    }

    std::shared_ptr<CBAR_SurfaceAssetCallbackI> CBAR_SurfaceAsset::getSurfaceCallback() const {
        return std::dynamic_pointer_cast<cbscene::CBAR_SurfaceAssetCallbackI>(getCallback());
    }
    
    bool CBAR_SurfaceAsset::saveJSONState(cbscene::CBAR_Scene *scene, Json::Value &assetNode) {
        
        assetNode["assetScale"] = m_pImpl->m_assetScale;
        assetNode["assetRotation"] = m_pImpl->m_rotation;
        
        return CBAR_Asset::saveJSONState(scene, assetNode);
    }
    
    void CBAR_SurfaceAsset::loadJSONState(cbscene::CBAR_Scene *scene, const Json::Value &assetNode) {
        CBAR_Asset::loadJSONState(scene, assetNode);
        
        if (assetNode.isMember("assetRotation")) {
            setPlaneRotation(assetNode["assetRotation"].asDouble());
        }
    }
    
    void CBAR_SurfaceAsset::reloadState(std::shared_ptr<cbpipe::UndoState> state) {
        CBAR_Asset::reloadState(state);
    }
    
    void CBAR_SurfaceAsset::attach(cbpipe::CBP_RenderingEngine *engine) {
        m_pImpl->_attach(engine);
    }
    
    void CBAR_SurfaceAsset::detach(cbpipe::CBP_RenderingEngine *engine) {
        m_pImpl->_detach(engine);
    }
    
    float CBAR_SurfaceAsset::getScale() const {
        return m_pImpl->m_assetScale;
    }
    
    void CBAR_SurfaceAsset::setScale(float scale) {
        m_pImpl->m_assetScale = scale;
    }
    
    float CBAR_SurfaceAsset::getPlaneRotation() const {
        return m_pImpl->m_rotation;
    }
    
    void CBAR_SurfaceAsset::setPlaneRotation(float value) {
        m_pImpl->m_rotation = value;
    }
    
    Eigen::Vector3f CBAR_SurfaceAsset::getPlanePosition() const {
        return m_pImpl->m_position;
    }
    
    void CBAR_SurfaceAsset::setPlanePosition(const Eigen::Vector3f &value) {
        m_pImpl->m_position = value;
    }
    
    bool CBAR_SurfaceAsset::saveToDirectory(const std::string &location) {
        
        return true;
    }
    
    void CBAR_SurfaceAsset::loadFromDirectory(const std::string &location, int applyRotation) {
        
    }
    
    HitTestResultType CBAR_SurfaceAsset::getHitTestType() const {
        return m_pImpl->m_planeType;
    }
    
    std::vector<int> CBAR_SurfaceAsset::getLabelTypes() const {
        return m_pImpl->m_labelTypes;
    }
    
    cv::Size CBAR_SurfaceAsset::getMaskSize() const {
        return cv::Size(2048, 2048);
    }
    
    cv::Size CBAR_SurfaceAsset::getShadowsSize() const {
        return cv::Size(512, 512);
    }
}
