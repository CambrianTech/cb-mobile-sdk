//
//  CB_ARScene.hpp
//  Cambrian
//
//  Created by Joel Teply on 3/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBAR_Scene_hpp
#define CBAR_Scene_hpp

#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <memory>
#include <utility/JSONHelper.h>
#include <cbar/CBAR_VideoFrame.hpp>
#include <cbar/pipeline/CBP_Types.hpp>

#include "CBAR_Asset.hpp"

namespace cbpipe {
    class CBP_RenderingEngine;
}

namespace cbscene {
    
    struct DLL_PUBLIC CBAR_SceneCallbackI {
        
        CBAR_SceneCallbackI() {
            
        }
        
        ~CBAR_SceneCallbackI() {
            
        }
        
        virtual std::shared_ptr<cbscene::CBAR_Asset> generateAsset(CBAR_Asset::asset_type assetType, const std::string &assetID) = 0;
        virtual void assetAppended(std::shared_ptr<cbscene::CBAR_Asset> asset) = 0;
        virtual void assetRemoved(std::shared_ptr<cbscene::CBAR_Asset> asset) = 0;
    };
    
    class DLL_PUBLIC CBAR_Scene {
    public:
        CBAR_Scene(std::shared_ptr<cbscene::CBAR_SceneCallbackI> callback, const std::string &sceneID = newUUID(), const void *context=nullptr);
        CBAR_Scene(std::shared_ptr<cbscene::CBAR_SceneCallbackI> callback, const cv::Mat &bgraImage, const std::string &sceneID = newUUID(), const void *context=nullptr);
        
        ~CBAR_Scene();
        
        enum scene_type {
            scene_type_generic,
            scene_type_remodeling,
        };
        
        virtual scene_type getType() {return scene_type_generic;};
        
        const void * getContext() const;
        
        std::string& getSceneID();
        void setSceneID(const std::string &sceneID);
        
        LightingType getLighting();
        void setLighting(LightingType lighting);
        
        const cv::Mat_<float>& getLightPositions() const;
        const cv::Mat_<float>& getLightColors() const;
        
        std::map<std::string, std::string>& getUserData();
        
        std::string getSelectedAssetID() const;
        bool setSelectedAssetID(const std::string &assetID);
        
        std::shared_ptr<cbscene::CBAR_Asset> getSelectedAsset();
        bool setSelectedAsset(std::shared_ptr<cbscene::CBAR_Asset>asset);
        
        const std::map<std::string, std::shared_ptr<cbscene::CBAR_Asset>> getAssets();
        
        virtual bool appendAsset(std::shared_ptr<cbscene::CBAR_Asset> asset);
        
        virtual bool removeAsset(const std::string &assetID);
        
        virtual void clearAssets();
        
        cbar::CBAR_VideoFramePtr getVideoFrame();
        void setVideoFrame(cbar::CBAR_VideoFramePtr);
        
        const Eigen::Matrix4f& getWorldTransform() const;
        void setWorldTransform(const Eigen::Matrix4f& wt, bool has6DOF=true);
        void setWorldTransform(const cv::Vec4f &quaterionXYZW, const cv::Vec4f &cameraPosition);
        bool hasWorldTransform();
        
        bool has6DOF() const;
        
        void setCameraTransform(const Eigen::Matrix4f& ct, bool has6DOF=true);
        Eigen::Matrix4f getCameraTransform() const;
        
        void setDisplayTransform(const Eigen::Matrix4f& dt);
        const Eigen::Matrix4f& getDisplayTransform() const;
        
        const Eigen::Matrix4f& getCameraProjection() const;
        void setCameraProjection(const Eigen::Matrix4f& proj);
        bool hasCameraProjection();
        
        void appendUndoData(std::shared_ptr<cbpipe::UndoState> state);
        
        int getUndoSize() const;
        int getMaxUndoSize() const;
        void setMaxUndoSize(int size);
        bool undoState();
        
        int getRedoSize() const;
        int getMaxRedoSize() const;
        void setMaxRedoSize(int size);
        bool redoState();
        
        int getAssetCount(CBAR_Asset::asset_type assetType);
        bool canAppendAsset(CBAR_Asset::asset_type assetType);

        cv::Size getSaveSize() const;
        
        virtual void saveToDirectory(const std::string &location, bool compressed, std::string &finalPath, std::string &projectJSON);
        virtual std::string loadFromDirectory(const std::string &location, int outputRotation,
                                              const Json::Value* sceneNode=nullptr);
        
        static cv::Mat getOriginalImageAtPath(const std::string &path);
        static cv::Mat getPreviewImageAtPath(const std::string &path);
        
        virtual void captureToStill();
        virtual void goingLive();
        
        virtual void saveJSONState(Json::Value &sceneNode);
        virtual void loadJSONState(const Json::Value& sceneNode);
    protected:
        
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif /* CB_ARScene_hpp */
