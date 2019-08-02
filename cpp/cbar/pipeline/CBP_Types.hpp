//
//  CBP_Types.hpp
//  Cambrian
//
//  Created by Joel Teply on 11/7/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#ifndef CBP_Types_hpp
#define CBP_Types_hpp

#include <stdio.h>
#include <cambrian.h>
#include <opencv2/core/core.hpp>
#include <cbcommon/CB_Types.h>
#include <utility/CommonUtility.h>
#include <memory>
#include <map>
#include <vector>
#include <Eigen/Geometry>

#include <utility/cache/cache.hpp>
#include <utility/cache/fifo_cache_policy.hpp>

#if DEBUG
//turn on/off optical flow dots
#   define DEBUG_MODE 1

//show entire plane
#   define DEBUG_PLANE 0
#   define DEBUG_FILL 0
#   define DEBUG_LIGHTS 0
#   define DEBUG_TOUCH_POINT 0
#   define SHOW_TIMING 0
#   define SHOW_ALLOCATION 0
#   define DEBUG_RESULT 0
#   define LOG_DEEP_INFO 0
#   define LOG_DEEP_RUNS 0
#   define LOG_DEEP_NON_RUNS 0

#   define LOG_THREAD_LIFECYCLE 1
#   define LOG_THREADS 1
#   define LOG_MUTEXES 0
#endif


/*
 * Image and screenshot logging and their frequencies
 *
 */

//Ability to disable ALL saved and uploaded diagnostic images and screenshots
#define DISABLE_ALL_LOGGING 1
#define UPLOAD_LOGGED_IMAGES 0

//default image logging frequency. Faster than this, saved or uploaded image calls are ignored
#define IMAGE_LOG_FREQUENCY 5.0

//segmentation:
#define DEBUG_DEEP_SEGMENTATION 0
#define DEBUG_SOURCE_IMAGES 0
#define DEBUG_NORMALS 0

//composite surfaces
#define DEBUG_SURFACES 1

//extra faked mask
#define DEBUG_HALLUCINATION 0

//app screenshots
#define TAKE_SCREENSHOTS 0
#define SCREENSHOT_FREQUENCY 10.0

//Disable all image logging
#if DISABLE_ALL_LOGGING
#   undef DEBUG_DEEP_SEGMENTATION
#   undef DEBUG_SOURCE_IMAGES
#   undef DEBUG_NORMALS
#   undef DEBUG_HALLUCINATION
#   undef TAKE_SCREENSHOTS
#endif

//rewrite thread priorities in CBAR_View for all background threads.
#define REPRIORITIZE_THREADS 1

#define FLOOR_VALUES {1} //floor, road/route, sidewalk, earth/ground, water, rug/carpet
#define WALL_VALUES {0, 14} //wall,  door / double door
#define LIGHT_SOURCE_VALUES {8, 82, 36} //windowpane/window, light/light source, lamp

#if SHOW_TIMING
#   define CBLogTiming(format, ...) CBLog(format, ##__VA_ARGS__)
#else
#   define CBLogTiming(format, ...) {}
#endif

#if SHOW_ALLOCATION
#   define CBLogAllocation(format, ...) CBLog(format, ##__VA_ARGS__)
#else
#   define CBLogAllocation(format, ...) {}
#endif

namespace cbpipe {
    
    typedef enum CBAR_PredictionType
    {
        CBAR_PredictionType_Semantic = 0,
        CBAR_PredictionType_Normals,
        CBAR_PredictionType_Elevation,
        CBAR_PredictionType_Shadows,
    } CBAR_PredictionType;
    
    struct CBP_DeepModelTerminal {
        cv::Size resolution  = cv::Size(256,256);
        std::string name;
        double multiplier = 1.0f/255.0f;
        int depth = CV_8UC3;
        bool flipBGR = false;
    };
    
    struct CBP_DeepModelInfo {
        
        CBAR_PredictionType type;
        std::string name = "Generic Deep Model";
        
        CBP_DeepModelTerminal input;
        CBP_DeepModelTerminal output;
        
        bool run_test_images = false;
        bool preload_image = false;
    };
    
    typedef enum HitTestResultType
    {
        /** Result type from intersecting the nearest feature point. */
        HitTestResultTypeFeaturePoint     = (1 << 0),
        
        /** Result type from detecting and intersecting a new horizontal plane. */
        HitTestResultTypeHorizontalPlane  = (1 << 1),
        
        /** Result type from detecting and intersecting a new vertical plane. */
        HitTestResultTypeVerticalPlane    = (1 << 2),
        
        /** Result type from intersecting with an existing plane anchor. */
        HitTestResultTypeExistingPlane    = (1 << 3),
        
        /** Result type from intersecting with an existing plane anchor, taking into account the plane's extent. */
        HitTestResultTypeExistingPlaneUsingExtent  = ( 1 << 4),
        HitTestResultTypeExistingPlaneUsingGeometry  = ( 1 << 5)
    } HitTestResultType;
    
//    const char * HitTestResultType_toString(HitTestResultType type) {
//        switch (type) {
//            case HitTestResultTypeFeaturePoint:
//                return "HitTestResultTypeFeaturePoint";
//            case HitTestResultTypeHorizontalPlane:
//                return "HitTestResultTypeHorizontalPlane";
//            case HitTestResultTypeVerticalPlane:
//                return "HitTestResultTypeVerticalPlane";
//            case HitTestResultTypeExistingPlane:
//                return "HitTestResultTypeExistingPlane";
//            case HitTestResultTypeExistingPlaneUsingExtent:
//                return "HitTestResultTypeExistingPlaneUsingExtent";
//            case HitTestResultTypeExistingPlaneUsingGeometry:
//                return "HitTestResultTypeExistingPlaneUsingGeometry";
//        }
//        return "";
//    }
    
    struct DLL_PUBLIC HitTestResult {
        HitTestResultType type;
        double distance;
        Eigen::Matrix4f localTransform;
        Eigen::Matrix4f worldTransform;
        std::string anchorIdentifier;
        bool isValid;
    };
    
    struct DLL_PUBLIC DeepSegmentationResult {
        Eigen::Matrix4f localTransform;
        Eigen::Matrix4f worldTransform;
        std::string anchorIdentifier;
        bool isValid;
    };
    
    typedef enum PlaneAnchorAlignment {
        PlaneAnchorAlignmentHorizontal = (1 << 0),
        PlaneAnchorAlignmentVertical = (1 << 1)
    } PlaneAnchorAlignment;
    
    struct DLL_PUBLIC PlaneAnchor {
        std::string anchorIdentifier;
        Eigen::Matrix4f transform;
        PlaneAnchorAlignment alignment;
        Eigen::Vector3f center;
        Eigen::Vector3f extent;
        
        std::vector<Eigen::Vector3f>contour;
    };
    
    struct DLL_PUBLIC UserAnchor {
        std::string anchorIdentifier;
        Eigen::Matrix4f transform;
    };
    
    enum of_feature_type {
        of_feature_all,
        of_feature_scene,
        of_feature_other,
    };
    
    struct of_match {
        int64_t frameIndex = -1;
        std::vector<int> matchIndexes;
        size_t numMatches = 0;
        bool isPointCloud = false;
    };
    
    typedef struct CBP_Material {
        cv::Vec3f AmbientColor = cv::Vec3f(0.5,0.5,0.5);
        cv::Vec3f DiffuseColor = cv::Vec3f(0.3,0.3,0.3);
        cv::Vec3f SpecularColor = cv::Vec3f(0.5,0.5,0.5);
        float Shininess = 0.0;
    } CBP_Material;
    
    typedef struct CBP_Texture {
        CBP_Material material;
        
        std::string texturePath;
        float scale = 1.0;
    } CBP_Texture;
    
    struct TouchPoint {
        TouchStep step;
        ToolMode toolMode;
        
        int64_t creationIndex = -1;
        int64_t creationTime = 0;
        
        cv::Point2f imageOrigin;
        cv::Point2f screenOrigin;
        
        cv::Size2f srcSize;
        std::string associatedAssetID;
        float rotation = 0;
        
        std::vector<HitTestResult>hits;
        
        // conversion to A (type-cast operator)
        operator cv::Point2f() {return this->imageOrigin;}
    };
    
    struct DLL_PUBLIC RotationGesture : public TouchPoint {
        //float rotation = 0;
    };
    
    struct ColorAdjustment {
        double alpha = 1.0;
        double beta = 0;
        double gamma = 0.75;
        
        double backgroundIntensity;
        cv::Scalar ambientLevel;
    };
    
    template <typename To, typename From>
    To container_cast(From && from) {
        using std::begin; using std::end; // Koenig lookup enabled
        return To(begin(from), end(from));
    }
    
    enum undo_target {
        undo_target_scene,
        undo_target_asset,
        undo_target_renderer,
    };
    
    enum undo_change {
        undo_change_mask,
        undo_change_paint_color,
        undo_change_paint_sheen,
    };
    
    struct DLL_PUBLIC UndoState {
        undo_target target;
        undo_change change;
        std::string assetID;
        std::string uuid;
        
        std::map<std::string, cv::Mat>data;
        std::map<std::string, std::string>stateInfo;
        
        UndoState() {
            
        }
        
        UndoState(undo_target _target, undo_change _change, const std::string &id)
            : target(_target), change(_change), assetID(id) {
                
        }
    };
    
    enum segmentation_type {
        segmentation_type_other = 0,
        segmentation_type_floor,
        segmentation_type_wall,
        segmentation_type_light_source,
    };
    
    template <typename Key, typename Value>
    using fifo_cache_t = typename caches::fixed_sized_cache<Key, Value, caches::FIFOCachePolicy<Key>>;
}

#endif /* CBP_Types_hpp */
