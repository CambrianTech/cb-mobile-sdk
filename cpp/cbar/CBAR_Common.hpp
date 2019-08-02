//
//  CB_PainterCommon.h
//  Cambrian
//
//  Created by Joel Teply on 9/29/15.
//  Copyright © 2015 Joel Teply. All rights reserved.
//

#ifndef CB_PainterCommon_h
#define CB_PainterCommon_h

#include <cambrian.h>
#include <opencv2/opencv.hpp>
#include <cbcommon/CB_Types.h>
#include <Eigen/Geometry>
#include <Eigen/QR>
#include <Eigen/LU>
#include <map>
#include <cbar/pipeline/CBP_Types.hpp>

namespace cbar {
    
    enum frame_type {
        frame_type_bgra,
        frame_type_420YpCbCr8,
        frame_type_YUV_420_888,
    };
    
    struct DLL_PUBLIC RawFrame {
        frame_type frameType;
        
        uintptr_t textureID = 0;
        
        uint8_t *data = nullptr;
        size_t dataLength = 0;
        
        uint8_t *chromaData = nullptr;
        
        size_t width = 0;
        size_t height = 0;
        size_t stride = 0;
        
        int frameRotation = 0;
    };
    
    struct DLL_PUBLIC structured_light {
        LightingType type;
        cv::Point3f position;
        float radius = 1.0;
        float attenuation = 0.5;
    };
    
    struct DLL_PUBLIC ColorInfo {
        cv::Scalar color;
        cv::Point2f normalizedPosition;
    };
    
    typedef enum
    {
        PaintAmountWayOverpainted,
        PaintAmountWayUnderpainted,
        PaintAmountSlightlyOverpainted,
        PaintAmountSlightlyUnderpainted,
        PaintAmountOkay,
    }  PaintAmount;
    
    typedef enum
    {
        PaintQualityChaos,
        PaintQualityChoppy,
        PaintQualityTooSimple,
        PaintQualityOkay,
    }  PaintQuality;
    
    struct DLL_PUBLIC TapPoint {
        cv::Point2f initialOrigin;
        cv::Vec3f initialOrientation;
        cv::Point2f origin;
        cv::Vec3f orientation;
        cv::Scalar color;
        
        std::vector<cv::Point>cardinalPoints;
    };
    
    struct DLL_LOCAL image_statistical_sample {
        cv::Scalar overlayIntensityAvg;
        cv::Scalar overlayIntensityDev;
        double overlayIntensity;
        
        cv::Scalar bgIntensityAvg;
        cv::Scalar bgIntensityDev;
        double bgIntensity;
        
        cv::Scalar colorGain;
    };
    
    typedef enum
    {
        CBOperationModeMobile = 0,
        CBOperationModeUnity,
        CBOperationModeServer,
    }  CBOperationMode;
    
    typedef enum
    {
        CBDeviceLevelLow = 0,
        CBDeviceLevelModerate,
        CBDeviceLevelHigh,
    }  CBDeviceLevel;
    
    struct CBConfig {
        std::string cbAssetPath;
        std::string workingPath;
        std::string loggingPath;
        std::string primaryAssetPath;
        std::string secondaryAssetPath;
        CBOperationMode mode = CBOperationModeMobile;
        CBDeviceLevel deviceLevel = CBDeviceLevelHigh;
        bool preloadSystem = true;
    };
    
    void CB_Initialize(CBConfig config) DLL_PUBLIC;
    
    std::string getCBAssetPath(const std::string &relativePath) DLL_PUBLIC;
    std::string getWorkingAssetPath(const std::string &relativePath) DLL_PUBLIC;
    std::string getCompressedCBAsset(const std::string &relativePath) DLL_PUBLIC;
    
    std::string getUserAssetPath(const std::string &relativePath) DLL_PUBLIC;
    const CBConfig& getCBConfig() DLL_PUBLIC;
}

#define ASPECT_RATIO(img) (((double) img.cols) / ((double) img.rows))


#endif /* CB_PainterCommon_h */
