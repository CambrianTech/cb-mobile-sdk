//
//  CBAR_SurfaceAsset.hpp
//  Cambrian
//
//  Created by Joel Teply on 3/15/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBAR_MaskedAsset_hpp
#define CBAR_MaskedAsset_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <utility/CommonUtility.h>
#include <utility/JSONHelper.h>
#include "CBAR_Asset.hpp"

namespace cbpipe {
    class CBP_Renderer;
    class CBP_SurfaceRenderer;
    struct SegmentationResult;
}

using namespace cbpipe;

namespace cbscene {
    
    struct DLL_PUBLIC CBAR_SurfaceData {
        int index;
        cv::Rect2f extents3D;
        Eigen::Vector3f center, normal;
        std::vector<Eigen::Vector3f> vertices;
        std::vector<int> indices;
        cv::Mat maskImage;
        cv::Rect2f maskExtents3D;
        cv::Mat shadowsImage;
        cv::Rect2f shadowsExtents3D;
        Eigen::Vector2f rotation2D = Eigen::Vector2f(0.0f, 0.0f);
    };
    
    struct DLL_PUBLIC CBAR_SurfaceAssetCallbackI : public CBAR_AssetCallbackI {
        
        CBAR_SurfaceAssetCallbackI() {}
        ~CBAR_SurfaceAssetCallbackI() {}
        
        virtual void surfaceDataUpdated(const CBAR_SurfaceData &surface) = 0;
    };
    
    class DLL_LOCAL CBAR_SurfaceAsset : public CBAR_Asset {
    public:
        CBAR_SurfaceAsset(const std::string &assetID,
                          const std::shared_ptr<CBAR_SurfaceAssetCallbackI>callback,
                          cv::Size textureSize,
                          HitTestResultType planeType,
                          const std::vector<int> &labelTypes);
        
        virtual cv::Scalar getColor() const { return cv::Scalar::all(0); };
        
        int getIndex();
        
        virtual std::shared_ptr<cbpipe::CBP_Renderer> getRenderer();
        virtual void destroyRenderer();
        
        void attach(cbpipe::CBP_RenderingEngine *engine);
        void detach(cbpipe::CBP_RenderingEngine *engine);
        
        virtual std::shared_ptr<cbpipe::CBP_SurfaceRenderer> getSurfaceRenderer();
        
        virtual bool saveJSONState(cbscene::CBAR_Scene *scene, Json::Value &assetNode);
        virtual void loadJSONState(cbscene::CBAR_Scene *scene, const Json::Value &assetNode);
        
        virtual bool saveToDirectory(const std::string &location);
        virtual void loadFromDirectory(const std::string &location, int applyRotation);
        
        virtual void reloadState(std::shared_ptr<cbpipe::UndoState> state);
        
        std::shared_ptr<CBAR_SurfaceAssetCallbackI> getSurfaceCallback() const;
        
        float getScale() const;
        void setScale(float scale);
        
        float getPlaneRotation() const;
        void setPlaneRotation(float value);
        
        Eigen::Vector3f getPlanePosition() const;
        void setPlanePosition(const Eigen::Vector3f &value);
        
        HitTestResultType getHitTestType() const;
        std::vector<int> getLabelTypes() const;
        
        cv::Size getMaskSize() const;
        cv::Size getShadowsSize() const;
                
    protected:
        ~CBAR_SurfaceAsset();
        
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
    
}

#endif /* CBAR_MaskedAsset_hpp */

