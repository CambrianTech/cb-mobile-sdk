//
//  CBARSurfaceAsset.m
//  CambrianSDK
//
//  Created by Joel Teply on 12/10/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#import "CBSurfaceAsset.h"
#import "CBARInternal.h"
#include <cbar/pipeline/rendering/rendering.h>

#define DEBUG_MASKS 0

@implementation CBSurfaceAsset

@end

CBARSurfaceAssetCallback::CBARSurfaceAssetCallback(CBAugmentedAsset * _Nonnull asset) : CBARAssetCallback(asset), cbscene::CBAR_SurfaceAssetCallbackI() {
    
}

void CBARSurfaceAssetCallback::attachedToScene() {
    CBARAssetCallback::attachedToScene();
}

void CBARSurfaceAssetCallback::detachedFromScene() {
    CBARAssetCallback::detachedFromScene();
}

inline void cvMatToCBImage(const cv::Mat &image, CBImageData &cbImage) {
    cbImage.image = (void *) image.ptr();
    cbImage.channels = image.channels();
    cbImage.width = image.cols;
    cbImage.height = image.rows;
}

void CBARSurfaceAssetCallback::surfaceDataUpdated(const cbscene::CBAR_SurfaceData &cbSurface) {

    CBSurfaceData surface;
    
    surface.index = cbSurface.index;
    surface.center = {cbSurface.center.x(), cbSurface.center.y(), cbSurface.center.z()};
    surface.normal = {cbSurface.normal.x(), cbSurface.normal.y(), cbSurface.normal.z()};
    surface.rotation2D = {cbSurface.rotation2D.x(), cbSurface.rotation2D.y()};
    surface.meshExtents3D = {cbSurface.extents3D.x, cbSurface.extents3D.y, cbSurface.extents3D.width, cbSurface.extents3D.height};
    surface.mesh.indices = cbSurface.indices;
    
    size_t numPoints = cbSurface.vertices.size();
    surface.mesh.vertices.resize(numPoints);
    
    for (int i=0; i<numPoints; i++) {
        const auto &vertice = cbSurface.vertices[i];
        surface.mesh.vertices[i] = {vertice.x(), vertice.y(), vertice.z()};
    }
    
    cvMatToCBImage(cbSurface.maskImage, surface.maskImageData);
    surface.maskExtents3D = {cbSurface.maskExtents3D.x, cbSurface.maskExtents3D.y,
        cbSurface.maskExtents3D.width, cbSurface.maskExtents3D.height};
    
    cvMatToCBImage(cbSurface.shadowsImage, surface.shadowsImageData);
    surface.shadowsExtents3D = {cbSurface.shadowsExtents3D.x, cbSurface.shadowsExtents3D.y,
        cbSurface.shadowsExtents3D.width, cbSurface.shadowsExtents3D.height};
    
    [CambrianUnityInterface.sharedInstance.session surfaceDataUpdated:surface];
    
}
