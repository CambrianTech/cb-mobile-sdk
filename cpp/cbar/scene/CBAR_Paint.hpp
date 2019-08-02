//
//  CB_ARPaint.hpp
//  Cambrian
//
//  Created by Joel Teply on 3/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBAR_Paint_hpp
#define CBAR_Paint_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <utility/JSONHelper.h>

#include <cbcommon/CB_Types.h>
#include "CBAR_SurfaceAsset.hpp"

namespace cbscene {
    
    class DLL_PUBLIC CBAR_Paint : public CBAR_SurfaceAsset {
    public:
        CBAR_Paint(const std::string &assetID, const std::shared_ptr<CBAR_SurfaceAssetCallbackI> callback);
        ~CBAR_Paint();
        
        virtual cv::Scalar getColor() const;
        void setColor(const cv::Scalar &color);
        
        Sheen getSheen() const;
        void setSheen(Sheen sheen);
        
        Transparency getTransparency() const;
        void setTransparency(Transparency transparency);
        
        double getOpacity();

        virtual asset_type getType() {return asset_type_paint;};
        
        virtual std::vector<HitTestResultType>getHitTypes() const {
            static const std::vector<HitTestResultType>hitTypes = {
                HitTestResultTypeVerticalPlane
            };
            return hitTypes;
        };
        
        virtual void reloadState(std::shared_ptr<cbpipe::UndoState> state);
        
        virtual bool saveJSONState(cbscene::CBAR_Scene *scene, Json::Value &assetNode);
        virtual void loadJSONState(cbscene::CBAR_Scene *scene, const Json::Value &assetNode);
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif /* CB_ARPaint_hpp */
