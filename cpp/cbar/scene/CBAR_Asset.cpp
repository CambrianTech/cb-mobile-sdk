//
//  CB_ARAsset.cpp
//  Cambrian
//
//  Created by Joel Teply on 3/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#include "CBAR_Asset.hpp"

#include <cbar/pipeline/pipeline.h>
#include <utility/Directory.h>
#include <utility/CommonUtility.h>

namespace cbscene {
    
    struct CBAR_Asset::Impl
    {
        Impl(CBAR_Asset *asset, const std::string &assetID, std::shared_ptr<CBAR_AssetCallbackI>callback)
        : m_asset(asset), m_assetID(assetID), m_callback(callback)
        {
            if (!m_assetID.length()) m_assetID = newUUID();
            
            CBLog("CREATED ASSET %s", m_assetID.c_str());
        }
        ~Impl() {
            CBLog("DESTROYED ASSET %s", m_assetID.c_str());
        }
        
        CBAR_Asset *m_asset;
        std::string m_assetID;
        std::shared_ptr<CBAR_AssetCallbackI> m_callback;
        bool m_isSelected = false;
        
        CBAR_Scene *m_scene = nullptr;
        bool m_isAttached = false;
        Eigen::Matrix4f m_worldTransform = Eigen::Matrix4f::Identity();
    };
    
    CBAR_Asset::~CBAR_Asset() {
        if (auto renderer = cbpipe::CBP_RenderingEngine::sharedInstance()) {
            detach(renderer.get());
        }
    }
    
    CBAR_Asset::CBAR_Asset(const std::string &assetID, std::shared_ptr<CBAR_AssetCallbackI>callback)  {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, assetID, callback));
    }
    
    std::string CBAR_Asset::getAssetID() const {
        return m_pImpl->m_assetID;
    }
    
    void CBAR_Asset::setAssetID(const std::string &assetID) {
        CBLog("Changed asset ID from '%s' to '%s'", m_pImpl->m_assetID.c_str(), trim_copy(assetID).c_str());
        m_pImpl->m_assetID = trim_copy(assetID);
    }
    
    const std::shared_ptr<CBAR_AssetCallbackI> CBAR_Asset::getCallback() const {
        return m_pImpl->m_callback;
    }
    
    bool CBAR_Asset::saveJSONState(cbscene::CBAR_Scene *scene, Json::Value &assetNode) {
        
        assetNode["assetID"] = m_pImpl->m_assetID;
        assetNode["type"] = getType();
        
        Json::Value worldTransform;
        JSONHelper::fillJSONArray(worldTransform, m_pImpl->m_worldTransform.data(), m_pImpl->m_worldTransform.size());
        assetNode["worldTransform"] = worldTransform;
        
        Json::Value userDataValue;
        for (auto &iter : userData) {
            userDataValue[iter.first] = iter.second;
        }
        assetNode["userData"] = userDataValue;
        
        //save renderer
        bool appearanceChanged = false;
        if (auto renderer = getRenderer()) {
            Json::Value rendererNode;
            appearanceChanged = renderer->saveJSONState(scene, rendererNode);
            assetNode["renderer"] = rendererNode;
        }
        
        return appearanceChanged;//appearance changed since load
    }
    
    void CBAR_Asset::loadJSONState(cbscene::CBAR_Scene *scene, const Json::Value &assetNode) {
        m_pImpl->m_assetID = assetNode["assetID"].asString();
        
        if (assetNode.isMember("worldTransform")) {
            const Json::Value& wtValue = assetNode["worldTransform"];
            if (!wtValue.isNull() && wtValue.isArray()) {
                JSONHelper::parseJSONArray(wtValue, m_pImpl->m_worldTransform.data());
            }
        }
        
        const Json::Value& userDataValues = assetNode["userData"];
        for (Json::Value::const_iterator itr = userDataValues.begin(); itr != userDataValues.end(); ++itr) {
            userData[itr.key().asString()] = itr->asString();
        }
        
        //load renderer
        const Json::Value& rendererNode = assetNode["renderer"];
        getRenderer()->loadJSONState(scene, rendererNode);
    }
    
    std::shared_ptr<cbpipe::CBP_Renderer> CBAR_Asset::getRenderer() {
        return std::shared_ptr<cbpipe::CBP_Renderer>();
    }
    
    void CBAR_Asset::captureState(std::shared_ptr<cbpipe::UndoState> state) {
        if (state->target == cbpipe::undo_target_renderer) {
            getRenderer()->captureState(state);
        }
    }
    
    void CBAR_Asset::reloadState(std::shared_ptr<cbpipe::UndoState> state) {
        if (state->target == cbpipe::undo_target_renderer) {
            getRenderer()->reloadState(state);
        }
    }
    
    CBAR_Scene * CBAR_Asset::getScene() {
        return m_pImpl->m_scene;
    }
    
    void CBAR_Asset::setScene(CBAR_Scene *scene) {
        m_pImpl->m_scene = scene;
    }
    
    bool CBAR_Asset::isSelected() const {
        return m_pImpl && m_pImpl->m_isSelected;
    }
    
    void CBAR_Asset::setIsSelected(bool value) {
        m_pImpl->m_isSelected = value;
    }
    
    void CBAR_Asset::captureToStill() {
        if (auto renderer = getRenderer()) {
            renderer->captureToStill();
        }
    }
    
    void CBAR_Asset::goingLive() {
        if (auto renderer = getRenderer()) {
            renderer->goingLive();
        }
    }
    
    void CBAR_Asset::touchedAtPoint(cbpipe::TouchPoint &touch) {
        if (auto renderer = getRenderer()) {
            if (touch.step == TouchStepTapped || touch.step == TouchStepBegan) {
                renderer->wakeup();
            }
            renderer->touchedAtPoint(touch);
        }
        
        if (auto callback = getCallback()) {
            std::thread([callback, touch](){
                callback->touchedAtPoint(touch);
            }).detach();
        }
    }
    
    void CBAR_Asset::rotatedBy(cbpipe::TouchPoint &touch) {
        if (auto renderer = getRenderer()) {
            if (touch.step == TouchStepTapped || touch.step == TouchStepBegan) {
                renderer->wakeup();
            }
            renderer->rotatedBy(touch);
        }
        
        if (auto callback = getCallback()) {
            std::thread([callback, touch](){
                callback->rotatedBy(touch);
            }).detach();
        }
    }

    void CBAR_Asset::rotatedBy(float amount) {
        if (auto renderer = getRenderer()) {
            renderer->wakeup();
            renderer->rotatedBy(amount);
        }

        if (auto callback = getCallback()) {
            std::thread([callback, amount](){
                callback->rotatedBy(amount);
            }).detach();
        }
    }
    
    bool CBAR_Asset::isAttached() const {
        return m_pImpl->m_isAttached;
    }
    
    void CBAR_Asset::attach(cbpipe::CBP_RenderingEngine *engine) {
        m_pImpl->m_isAttached = true;
        m_pImpl->m_callback->attachedToScene();
    }
    
    void CBAR_Asset::detach(cbpipe::CBP_RenderingEngine *engine) {
        m_pImpl->m_isAttached = false;
        m_pImpl->m_callback->detachedFromScene();
    }
    
    bool CBAR_Asset::isUtilized() {
        if (auto renderer = getRenderer()) {
            return renderer->isUtilized();
        }
        return true;
    }
    
    void CBAR_Asset::commitChanges() {
        if (auto renderer = getRenderer()) {
            renderer->commitChanges();
        }
    }
    
    Eigen::Matrix4f CBAR_Asset::getWorldTransform() const {
        return m_pImpl->m_worldTransform;
    }
    
    void CBAR_Asset::setWorldTransform(const Eigen::Matrix4f &transform) {
        m_pImpl->m_worldTransform = transform;
    }
};

