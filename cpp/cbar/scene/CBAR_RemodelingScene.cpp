//
//  CBAR_RemodelingScene.cpp
//  Cambrian
//
//  Created by Joel Teply on 3/23/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#include "CBAR_RemodelingScene.hpp"

#include <cbar/CBAR_View.hpp>
#include <imaging/ImageProcessing.h>
#include <imaging/Accelerated.h>
#include <utility/JSONHelper.h>
#include <imaging/Imaging.h>

using namespace imaging;
using namespace cbpipe;

namespace cbscene {
    
    struct CBAR_RemodelingScene::Impl
    {
        Impl(CBAR_RemodelingScene *scene)
        : m_scene(scene)        {
            CBLog("Created CBAR_RemodelingScene");
        }
        ~Impl() {
            
        }
        
        CBAR_RemodelingScene *m_scene;
        
        cv::Mat m_positiveMask;
        cv::Mat m_negativeMask;
        
        void save_to_directory(const std::string &location) {
//            std::string maskPath = string_sprintf("%s/mask.png", location.c_str());
//
//            m_positiveMask = cv::imread(maskPath.c_str(), cv::IMREAD_GRAYSCALE);
//            if (!m_positiveMask.empty()) {
//                ImageProcessing::rotate_image_90n(m_positiveMask, m_positiveMask, outputRotation);
//                m_negativeMask = cv::Mat::ones(m_positiveMask.size(), m_positiveMask.type()) * 255 - m_positiveMask;
//            }
        }
        
        void load_from_directory(const std::string &location, int outputRotation) {
            
            if (!m_positiveMask.empty()) {
//                auto renderer = cbpipe::CBP_RenderingEngine::sharedInstance(); if (!renderer) return;
//
//                cv::Mat mask;
//                renderer->fitImageToOutput(m_positiveMask, mask);
//                std::string maskPath = string_sprintf("%s/mask.png", location.c_str());
//                cv::imwrite(maskPath.c_str(), mask);
            }
        }
    };
    
    CBAR_RemodelingScene::CBAR_RemodelingScene(std::shared_ptr<cbscene::CBAR_SceneCallbackI> callback, const std::string &sceneID, const void *boundObject)
        : CBAR_Scene(callback, sceneID, boundObject)  {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }
    
    CBAR_RemodelingScene::CBAR_RemodelingScene(std::shared_ptr<cbscene::CBAR_SceneCallbackI> callback, const cv::Mat &rgbaImage, const std::string &sceneID, const void *context)
        : CBAR_Scene(callback, rgbaImage, sceneID, context)  {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }
    
    CBAR_RemodelingScene::~CBAR_RemodelingScene() {
        CBLog("DESTROYED CBAR_RemodelingScene");
    }
    
    bool CBAR_RemodelingScene::appendAsset(std::shared_ptr<cbscene::CBAR_Asset> asset) {
        int numPaints = 0;
        int numFloors = 0;
        auto assets = getAssets();
        
        if (!assets.empty()) {
            for (const auto &it: getAssets()) {
                if (it.second->getType() == CBAR_Asset::asset_type_paint) {
                    numPaints ++;
                } else if (it.second->getType() == CBAR_Asset::asset_type_floor) {
                    numFloors ++;
                }
            }
        }
        
        if (asset->getType() == CBAR_Asset::asset_type_model
            || (asset->getType() == CBAR_Asset::asset_type_paint && numPaints < 4)
            || (asset->getType() == CBAR_Asset::asset_type_floor && numFloors == 0)) {
            CBAR_Scene::appendAsset(asset);
            return true;
        }
        
        return false;
    }
    
    void CBAR_RemodelingScene::saveToDirectory(const std::string &location, bool compressed, std::string &finalPath, std::string &projectJSON) {
        m_pImpl->save_to_directory(location);
        CBAR_Scene::saveToDirectory(location, compressed, finalPath, projectJSON);
    }
    
    std::string CBAR_RemodelingScene::loadFromDirectory(const std::string &location, int outputRotation, const Json::Value* sceneNode) {
        m_pImpl->load_from_directory(location, outputRotation);
        return CBAR_Scene::loadFromDirectory(location, outputRotation, sceneNode);
    }

    const cv::Mat& CBAR_RemodelingScene::getPositiveMask() {
        return m_pImpl->m_positiveMask;
    }
    
    const cv::Mat& CBAR_RemodelingScene::getNegativeMask() {
        return m_pImpl->m_negativeMask;
    }
    
    bool CBAR_RemodelingScene::isMasked() const {
        return !m_pImpl->m_positiveMask.empty();
    }
}
