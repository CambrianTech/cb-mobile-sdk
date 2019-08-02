//
//  CB_ARPaint.cpp
//  Cambrian
//
//  Created by Joel Teply on 3/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#include "CBAR_Paint.hpp"
#include <utility/JSONHelper.h>
#include <cbar/pipeline/rendering/rendering.h>

#include <cbar/pipeline/analysis/analysis.h>
#include <cbar/pipeline/rendering/rendering.h>

namespace cbscene {
    
    struct CBAR_Paint::Impl
    {
        Impl(CBAR_Paint *parent) : m_parent(parent) {
              
        }

       ~Impl() {}
        
        cbscene::CBAR_Paint *m_parent;
        cv::Scalar m_color = cv::Scalar::all(0);//defaults to transparent
        Sheen m_sheen = SheenMatte;//defaults to charlie
        
        Transparency m_transparency = TransparencyNone;
        
        void reloadState(std::shared_ptr<cbpipe::UndoState> state) {
            if (state->data.count("sheen")) {
                m_sheen = (Sheen) state->data["sheen"].at<int>(0);
            }
            
            if (state->data.count("color")) {
                cv::Mat color = state->data["color"];
                for (int i=0; i<color.rows; i++) {
                    m_color[i] = color.at<int>(i);
                }
            }
        }
        
        
    };
    
    CBAR_Paint::~CBAR_Paint() {
        
    }
    
    CBAR_Paint::CBAR_Paint(const std::string &assetID, const std::shared_ptr<CBAR_SurfaceAssetCallbackI>callback)
    : cbscene::CBAR_SurfaceAsset(assetID, callback, cv::Size(1024,1024), HitTestResultTypeVerticalPlane, std::vector<int>(WALL_VALUES))  {
        //CBLog("new CBAR_Floor");
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }
    
    Sheen CBAR_Paint::getSheen() const {
        return m_pImpl->m_sheen;
    }
    
    void CBAR_Paint::setSheen(Sheen sheen) {
        if (getScene()) {
            std::shared_ptr<cbpipe::UndoState> undoStep = std::shared_ptr<cbpipe::UndoState>(new cbpipe::UndoState(cbpipe::undo_target_asset, cbpipe::undo_change_paint_sheen, getAssetID()));
            undoStep->stateInfo = userData;
            std::string sheenKey = "sheen";//android causing crash without this
            undoStep->data[sheenKey] = cv::Mat(cv::Scalar(sheen));
            getScene()->appendUndoData(undoStep);
        }
        
        m_pImpl->m_sheen = sheen;
    }
    
    cv::Scalar CBAR_Paint::getColor() const {
        return m_pImpl->m_color;
    }
    
    void CBAR_Paint::setColor(const cv::Scalar &color) {
        bool saveColorHistory = false;
        if (saveColorHistory && getScene()) {
            //save last color
            std::shared_ptr<cbpipe::UndoState> undoStep = std::shared_ptr<cbpipe::UndoState>(new cbpipe::UndoState(cbpipe::undo_target_asset, cbpipe::undo_change_paint_color, getAssetID()));
            undoStep->stateInfo = userData;
            
            cv::Mat mat = cv::Mat(4,1,CV_8UC1);
            for (int i=0; i<4; i++) {
                mat.at<int>(i) = m_pImpl->m_color[i];
            }
            std::string colorKey = "color";//android causing crash without this
            undoStep->data[colorKey] = mat;
            getScene()->appendUndoData(undoStep);
        }
        
        m_pImpl->m_color = color;
    }
    
    Transparency CBAR_Paint::getTransparency() const {
        return m_pImpl->m_transparency;
    }
    
    void CBAR_Paint::setTransparency(Transparency transparency) {
        m_pImpl->m_transparency = transparency;
    }
    
    double CBAR_Paint::getOpacity() {
        switch(m_pImpl->m_transparency) {
            case TransparencyUltraSolid:
            return 0.9;
            case TransparencySolid:
            return 0.8;
            case TransparencySemiTransparent:
            return 0.7;
            case TransparencyTransparent:
            return 0.5;
            case TransparencyTranslucent:
            return 0.3;
            case TransparencyClear:
            return 0.1;
            default:
            return 1.0;
        }
    }
        
    bool CBAR_Paint::saveJSONState(cbscene::CBAR_Scene *scene, Json::Value &assetNode) {
        
        assetNode["sheen"] = getSheen();
        assetNode["color"] = JSONHelper::createScalarJSONValue(getColor());
        
        CBAR_SurfaceAsset::saveJSONState(scene, assetNode);
        
        return true;//appearance changed since load
    }
    
    void CBAR_Paint::loadJSONState(cbscene::CBAR_Scene *scene, const Json::Value &assetNode) {
        CBAR_SurfaceAsset::loadJSONState(scene, assetNode);
        const Json::Value& sheenValue = assetNode["sheen"];
        if (!sheenValue.isNull()) {
            m_pImpl->m_sheen = static_cast<Sheen>(sheenValue.asInt());
        }
        m_pImpl->m_color = JSONHelper::getScalarJSONValue(assetNode["color"]);
    }
    
    void CBAR_Paint::reloadState(std::shared_ptr<cbpipe::UndoState> state) {
        CBAR_SurfaceAsset::reloadState(state);
        
        if (state->target == cbpipe::undo_target_asset) {
            m_pImpl->reloadState(state);
        }
    }
};
