//
//  CB_ARAsset.hpp
//  Cambrian
//
//  Created by Joel Teply on 3/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBAR_Asset_hpp
#define CBAR_Asset_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <utility/CommonUtility.h>

#include <cbar/pipeline/CBP_Types.hpp>
#include <utility/JSONHelper.h>
#include <Eigen/Geometry>

namespace cbpipe {
    class CBP_Renderer;
    class CBP_RenderingEngine;
}

using namespace cbpipe;

namespace cbscene {
    
    struct DLL_PUBLIC CBAR_AssetCallbackI {
        
        CBAR_AssetCallbackI() {
            
        }
        
        virtual void attachedToScene() = 0;
        
        virtual void detachedFromScene() = 0;
        
        virtual void touchedAtPoint(cbpipe::TouchPoint touch) {};
        
        virtual void rotatedBy(cbpipe::TouchPoint touch) {};

        virtual void rotatedBy(float amount) {};
    };
    
    class CBAR_Scene;
    
    class DLL_PUBLIC CBAR_Asset {
    public:
        friend class CBAR_Scene;
        CBAR_Asset(const std::string &assetID, std::shared_ptr<CBAR_AssetCallbackI>callback);
        
        enum asset_type {
            asset_type_all,
            asset_type_paint,
            asset_type_floor,
            asset_type_model,
        };
        
        virtual asset_type getType() {return asset_type_all;};
        
        virtual std::vector<HitTestResultType>getHitTypes() const {
            static const std::vector<HitTestResultType>hitTypes = {
                HitTestResultTypeHorizontalPlane,
                HitTestResultTypeVerticalPlane,
                HitTestResultTypeExistingPlane,
                HitTestResultTypeExistingPlaneUsingExtent,
                HitTestResultTypeExistingPlaneUsingGeometry,
            };
            return hitTypes;
        };
        
        Eigen::Matrix4f getWorldTransform() const;
        void setWorldTransform(const Eigen::Matrix4f &transform);
        
        std::string getAssetID() const;
        void setAssetID(const std::string& assetID);//internal use only
        
        const std::shared_ptr<CBAR_AssetCallbackI> getCallback() const;
        
        CBAR_Scene * getScene();
        void setScene(CBAR_Scene *scene);
        
        std::map<std::string, std::string> userData;
        
        virtual void captureState(std::shared_ptr<cbpipe::UndoState> state);
        virtual void reloadState(std::shared_ptr<cbpipe::UndoState> state);
        
        virtual bool saveJSONState(cbscene::CBAR_Scene *scene, Json::Value &assetNode);
        virtual void loadJSONState(cbscene::CBAR_Scene *scene, const Json::Value &assetNode);
        
        virtual bool saveToDirectory(const std::string &location) = 0;
        virtual void loadFromDirectory(const std::string &location, int applyRotation) = 0;
        
        virtual std::shared_ptr<cbpipe::CBP_Renderer> getRenderer();
        virtual void destroyRenderer() {};
        
        void setValues();
        
        bool isSelected() const;
        
        virtual bool isUtilized();
        
        virtual void captureToStill();
        
        virtual void goingLive();
        
        virtual void touchedAtPoint(cbpipe::TouchPoint &touch);
        
        virtual void rotatedBy(cbpipe::TouchPoint &touch);
        virtual void rotatedBy(float amount);
        
        bool isAttached() const;
        virtual void attach(cbpipe::CBP_RenderingEngine *engine);
        virtual void detach(cbpipe::CBP_RenderingEngine *engine);
        
        void commitChanges();
    protected:
        ~CBAR_Asset();
        
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
        
        void setIsSelected(bool value);
    };
}


#endif /* CB_ARAsset_hpp */
