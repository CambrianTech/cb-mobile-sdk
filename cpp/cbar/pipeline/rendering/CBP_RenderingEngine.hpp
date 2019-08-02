//
//  CB_RenderingPipeline.hpp
//  Cambrian
//
//  Created by Joel Teply on 5/8/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#ifndef CB_RenderingEngine_hpp
#define CB_RenderingEngine_hpp

#include <stdio.h>
#include <memory>
#include <future>
#include <opencv2/opencv.hpp>

#include <cambrian.h>

#include <cbar/CBAR_View.hpp>
#include <cbar/pipeline/CBP_Types.hpp>
#include <cbar/pipeline/rendering/rendering.h>

#include <Eigen/Geometry>
#include <Eigen/QR>
#include <Eigen/LU>

//defined in build settings
//#define FLOORING 1
#define MODELS 0

#include "CBP_Renderer.hpp"

namespace cbpipe {
    class CBP_MotionTracker;
    class CBP_LineFinder;
    class CBP_ViewRenderer;
    class CBP_Analyzer;
    
    class DLL_LOCAL CBP_RenderingEngine
    {
    public:
        CBP_RenderingEngine(cbar::CBAR_Client *videoPainter, std::shared_ptr<cbar::CBAR_CallbackI>callback);
        ~CBP_RenderingEngine();
        
        static std::shared_ptr<CBP_RenderingEngine> sharedInstance();
        
        template<class T> std::shared_ptr<T> getAnalyzerOfType() {
            return m_analyzer->ofType<T>();
        }
        
        template<class T> std::vector<std::shared_ptr<T>> getAnalyzersOfType() {
            return m_analyzer->getAnalyzersOfType<T>();
        }
        
        bool appendAnalyzer(std::shared_ptr<CBP_AnalyzerThread> analyzer);
        bool removeAnalyzer(std::shared_ptr<CBP_AnalyzerThread> analyzer);
        
        void setScene(std::shared_ptr<cbscene::CBAR_Scene> scene);
        std::shared_ptr<cbscene::CBAR_Scene> getScene() const;
        
        void initialize(std::shared_ptr<CBP_RenderingEngine> renderer);
        
        void prepareViewport(void *nwh, void *glContext,
                             int frameWidth, int frameHeight,
                             int outputWidth, int outputHeight,
                             float deviceFOV, int rotation,
                             const Eigen::Matrix3f &cameraIntrinsics);
                
        void addFrame(const cbar::RawFrame &frame);
        
        Eigen::Vector3f getLastHeading();
        void updateHeading(const Eigen::Vector3f &heading);
                
        void stillRender();
        void clearAll();
        
        void saveScreenshot(const std::string &path, std::function<void(bool, const std::string&)> completion, float maxSeconds=3.0f);
        void screenshotSaved();
        
        cbar::CBAR_Client *getARView() const;
        
        cv::Point2f screenToVideoPosition(const cv::Point2f &screenPoint);
        cv::Point2f screenToVideoPositionNormalized(const cv::Point2f &screenPointNormalized);
        
        cv::Point2f videoToScreenPosition(const cv::Point2f &videoPoint);
        cv::Point2f videoToScreenPositionNormalized(const cv::Point2f &videoPointNormalized);

        Eigen::Vector3f unprojectPoint(const cv::Point2f &point2D, float projectedDistance=0.0, int64_t frameIndex=0);
        cv::Point3f projectPoint(const Eigen::Vector3f &point3D, int64_t frameIndex=0);
        
        std::vector<Eigen::Vector3f> unprojectPointCloud(const std::vector<cv::Point> &points, float distance=1.0, int64_t frameIndex=0);
        std::vector<cv::Point> projectPointCloud(const std::vector<Eigen::Vector3f> &points, int64_t frameIndex=0);
        
        void getVideoBoundingBox(const Eigen::Vector3f &cen, const Eigen::Vector3f &ext,
                                 std::vector<cv::Point> &videoBounds, std::vector<Eigen::Vector3f> &worldBounds, int64_t frameIndex=0);
        cv::Rect2f getVideoBounds(const Eigen::Vector3f &cen, const Eigen::Vector3f &ext, int64_t frameIndex=0);
        
        
        void setPointCloudData(const std::map<uint64_t, Eigen::Vector3f> &points);
        
        void planeAnchorAdded(const PlaneAnchor &anchordata);
        void planeAnchorUpdated(const PlaneAnchor &anchordata);
        void planeAnchorRemoved(const PlaneAnchor &anchordata);
        
        void userAnchorAdded(const UserAnchor &anchordata);
        void userAnchorUpdated(const UserAnchor &anchordata);
        void userAnchorRemoved(const UserAnchor &anchordata);
        
        TouchPoint getLastTouch() const;
                
        cv::Size_<float> getFOV() const;
        
        cv::Size getFrameSize() const;
        cv::Size getOutputSize() const;
        
        int getFrameRotation() const;
        int getOutputRotation() const;
        
        void fitImageToOutput(const cv::Mat &src, cv::Mat &dest) const;
        void writePNG(const std::string &path, const cv::Mat &src) const;
        
        int getDebugMode() const;
        void changeDebugMode(bool on);

        float getAngularSpeed(int64_t deltaFrames);
        float getFPS() const;
        float getTranslationalSpeed(int64_t deltaFrames);
                
        //gestures
        void touchedAt(const cv::Point2f &normalizedPoint, ToolMode toolMode, TouchStep step);
        void tappedAt(const cv::Point2f &normalizedPoint, ToolMode toolMode);
        void rotateGesture(float amount, const cv::Point2f &normalizedPoint, TouchStep step);
        void rotatedBy(float amount);
        
        void startRunning(bool videoMode);
        void stopRunning();
        
        void pauseRendering(bool pause);
        bool isRenderingPaused() const;
        
        bool isStillMode() const;
        void setStillMode(bool);
        
        void setLighting(const LightingType &);
        
        void captureCurrentState();
        
        const std::shared_ptr<cbar::CBAR_CallbackI> getCallback() const;
        
        cv::Scalar getColorInVideoAtPoint(const cv::Point2f &point);
        
        cv::Point2f get2DOffset() const;
        void set2DOffset(const cv::Point2f &offset);
        
        void showPoints(const std::map<uint64_t, Eigen::Vector3f> &points, const cv::Scalar &color);
    private:
                
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
        
        std::unique_ptr<CBP_Analyzer> m_analyzer;
    };
};

#endif /* CB_RenderingPipeline_hpp */
