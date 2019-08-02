//
//  CB_ARFloor.hpp
//  Cambrian
//
//  Created by Joel Teply on 3/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBAR_Floor_hpp
#define CBAR_Floor_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <utility/JSONHelper.h>

#include "CBAR_SurfaceAsset.hpp"

namespace cbscene {
    
    class DLL_PUBLIC CBAR_Floor : public CBAR_SurfaceAsset {
    public:
        CBAR_Floor(const std::string &assetID, const std::shared_ptr<CBAR_SurfaceAssetCallbackI>callback);
        ~CBAR_Floor();
        
        virtual asset_type getType() {return asset_type_floor;};
        
        virtual std::vector<HitTestResultType>getHitTypes() const {
            static const std::vector<HitTestResultType>hitTypes = {
                HitTestResultTypeHorizontalPlane
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

#endif /* CB_ARFloor_hpp */
