//
//  CB_RenderingPipeline.cpp
//  Cambrian
//
//  Created by Joel Teply on 5/8/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#include "CBP_RenderingEngine.hpp"

#include <cbar/pipeline/motion/motion.h>
#include <cbar/pipeline/rendering/rendering.h>

#include <cbar/CBAR_VideoFrame.hpp>
#include <cbar/scene/scene.h>
#include <cbar/pipeline/util/util.h>
#include <cbar/pipeline/analysis/analysis.h>

#include <imaging/Imaging.h>
#include <utility/Directory.h>
#include <imaging/Accelerated.h>
#include <imaging/Coloring.h>
#include <thread>

using namespace cbar;

namespace cbpipe {
    
    class DLL_LOCAL RenderThread : public QueueThread<bool> {
    public:
        RenderThread() : QueueThread<bool>("RenderThread", 1)
        {}
        ~RenderThread() {}
        
        virtual bool handleItem(bool isLive) { return false; };
    };
    
    static std::weak_ptr<CBP_RenderingEngine> staticInstance;

    struct CBP_RenderingEngine::Impl
    {
        Impl(CBP_RenderingEngine *parent, cbar::CBAR_Client *arView, std::shared_ptr<cbar::CBAR_CallbackI> callback)
            : m_parent(parent), m_callback(callback), m_arView(arView) {
            
        }
        
        ~Impl() {
            CBLog("Destroying CBP_RenderingEngine");
        }
        
        CBP_RenderingEngine *m_parent;
        std::shared_ptr<cbar::CBAR_CallbackI> m_callback;
        std::shared_ptr<cbscene::CBAR_Scene> m_scene;
        
        cv::Vec4f m_cameraPosition = cv::Vec4f();
        
        bool m_hasSetOverlay = false;
        bool m_isRunning = false;
        bool m_isRenderingPaused = false;
        bool m_isStillMode = true;
        bool m_isCapturing = false;
        FPSTimer m_fpsTimer;
        
        cbar::CBAR_Client *m_arView = 0;
        
        CBMutex m_renderMutex;
        
        TouchPoint m_lastTouch;
        
        cv::Size_<float> m_fov;
        int m_rotation = 0;
        Eigen::Matrix4f m_displayTransform;
        
        cv::Size m_frameSize;
        cv::Size m_outputSize;
        
        //Video Render
        cbar::CBAR_VideoFramePtr m_frame;
        
        std::vector<cv::Mat> _lastOverlay;
        
        int m_ppCreationIndex = 0;
        
        int m_debugMode = 0;
        
        cv::Point2f m_2DOffset;
        
        void _prepare_viewport(void *nwh, void *glContext,
                             int frameWidth, int frameHeight,
                             int outputWidth, int outputHeight,
                             float hFOV, int rotation,
                             const Eigen::Matrix3f &cameraIntrinsics) {
            
            m_rotation = 90 * (rotation / 90);//multiple of 90
            m_frameSize = cv::Size(frameWidth, frameHeight);
            m_outputSize = cv::Size(outputWidth, outputHeight);
            
            float frameAspect = float(frameWidth) / float(frameHeight);
            float screenAspect = float(outputWidth) / float(outputHeight);
            
            bool isRotated = rotation/90 % 2;
            if (isRotated) {
                frameAspect = 1.0f / frameAspect;
            }
            
            m_fov = cv::Size_<float>(hFOV, hFOV / screenAspect);
            
            cv::Size2f scale;
            scale.width = frameAspect > screenAspect ? frameAspect / screenAspect : 1.0;
            scale.height = frameAspect > screenAspect ? 1.0 : screenAspect / frameAspect;
            
            Eigen::Matrix4f scaleMat = CBP_MatrixUtil::makeScaleMatrix(scale.width, scale.height, 1.0);
            Eigen::Matrix4f rotationMat = CBP_MatrixUtil::makeRotationMatrix(0.0f, 0.0f, toRadians(rotation));
            
            m_displayTransform = scaleMat * rotationMat;
        }
        
        int _get_debug_mode() const {
#if DEBUG
            return 1;
#endif
            return m_debugMode;
        }
        
        void _set_debug_mode(bool on) {
            int newDebugMode = m_debugMode;
            
            if (on)  newDebugMode++;
            else newDebugMode = 0;
            
            int maxValue = 5;
            if (newDebugMode > maxValue) {
                newDebugMode = 0;
            }
            
            if (newDebugMode == m_debugMode) {
                return;
            }
            
            m_debugMode = newDebugMode;
        }
        
        void _pause_rendering(bool pause) {
            m_isRenderingPaused = pause;
            
            if (!pause && m_scene) {
                m_scene->goingLive();
            }
        }
        
        void _flush() {
            m_parent->m_analyzer->flush();
            
            if (m_scene) {
                for (auto it : m_scene->getAssets()) {
                    if (auto renderer = it.second->getRenderer()) {
                        renderer->flush();
                    }
                }
            }
        }
        
        void _set_scene(std::shared_ptr<cbscene::CBAR_Scene> scene) {
            m_scene = scene;
            //show scene
            auto frame = m_scene->getVideoFrame();
            
            if (frame.empty()) {
                m_isStillMode = false;
            }
            else {
                m_isStillMode = true;
                _flush();
                m_callback->sceneImageLoaded(frame->getRGBAImage());
            }
        }
        
        void _add_frame_data(const cbar::RawFrame &frameData) {
            
            if (m_isRenderingPaused || !m_isRunning || m_isStillMode) return;

            m_frame = new cbar::CBAR_VideoFrame(frameData);
            
            m_fpsTimer.tick();
            
            m_parent->m_analyzer->addItem(m_frame);
            
            if (!m_scene || m_frame.empty() || !m_frame->frameSize().width) return;
            
            m_scene->setVideoFrame(m_frame);
        }

        void _capture_current_state() {
            //already in the scene
            //tell color finder to stop finding crap.
            
            m_isCapturing = true;
            
            m_scene->captureToStill();
            
            m_parent->m_analyzer->captureCurrentState();
            
            m_isCapturing = false;
            m_isStillMode = true;
        }
        
        void _fit_image_to_output(const cv::Mat &src, cv::Mat &dest) const {
            if (src.empty()) return;
            
            cv::Size2f finalSize = m_parent->getOutputSize();
            if (finalSize.empty()) {
                finalSize = cv::Size2f(src.cols, src.rows);
            }
            
            int applyRotation = m_parent->getFrameRotation();
            
            bool rotated = false;
            if (applyRotation) {
                ImageProcessing::rotate_image_90n(src, dest, applyRotation);
                rotated = true;
            } else {
                dest = src.clone();
            }
            
            float frameAR = float(dest.cols) / float(dest.rows);
            float finalAR = finalSize.width / finalSize.height;
            
            //crop image to fit final size
            cv::Size currentSize = dest.size();
            
            if (frameAR > finalAR) { //if frame is wider than output, clip its left/right sides, and preserve height
                int newFrameWidth = int(float(dest.rows) * finalAR);
                dest = dest(cv::Rect((dest.cols - newFrameWidth)/2, 0, newFrameWidth, dest.rows));
            } else if (frameAR < finalAR) { //if frame is skinnier than output, clip its top and bottom, and preserve width
                int newFrameHeight = int(float(dest.cols) / finalAR);
                dest = dest(cv::Rect(0, (dest.rows - newFrameHeight)/2, dest.cols, newFrameHeight));
            }
            
            //resize if needed
            if (currentSize.width != finalSize.width || currentSize.height != finalSize.height) {
                cv::resize(dest, dest, finalSize);
            }
        }
        
        void _start_running(bool videoMode) {
            
            m_isRenderingPaused = false;
            m_isRunning = true;
            m_hasSetOverlay = false;
            m_isStillMode = !videoMode;
            
            if (m_scene) {
                m_scene->goingLive();
            }
            
            _flush();
            
            m_parent->m_analyzer->start();
        }
        
        void _stop_running() {
            m_isRunning = false;
            
            m_parent->m_analyzer->abort();
            
            m_arView = 0;
        }
        
        float _get_angular_speed(int64_t deltaFrames) {

            int64_t endIndex = fmax(0,CBAR_VideoFrame::lastFrameIndex()-1);
            int64_t startIndex = fmax(0, endIndex - deltaFrames);
            
            if (startIndex < 0) return 0.0f;
            
            Eigen::Matrix3f posThen = _get_model_view(startIndex).topLeftCorner<3,3>();
            Eigen::Matrix3f posNow = _get_model_view(endIndex).topLeftCorner<3,3>();
            
            deltaFrames = endIndex - startIndex;
            float elapsedTime = float(deltaFrames) / m_parent->getFPS();
            
            Eigen::Quaternionf velocity = CBP_MatrixUtil::getAngularVelocity(CBP_MatrixUtil::getQuaternion(posThen),
                                                                             CBP_MatrixUtil::getQuaternion(posNow), elapsedTime);
            
            float angularSpeed = velocity.norm();
            return angularSpeed;
        }
        
        float _get_translational_speed(int64_t deltaFrames) {
            
            int64_t endIndex = CBAR_VideoFrame::lastFrameIndex();
            int64_t startIndex = endIndex - deltaFrames;
            
            if (startIndex < 0) return 0.0f;
            
            Eigen::Matrix4f posThen = _get_model_view(startIndex);
            Eigen::Matrix4f posNow = _get_model_view(endIndex);
            
            float elapsedTime = float(deltaFrames) / m_parent->getFPS();
            
            auto velocity = CBP_MatrixUtil::getTranslationalVelocity(posThen, posNow, elapsedTime);
            
            float angularSpeed = velocity.norm();
            return angularSpeed;
        }
        
        TouchPoint _common_touches(const cv::Point2f &normalizedPoint, TouchStep step) {
            
            TouchPoint touch;
            
            if (!m_scene) {
                CBError("No scene touches, ignored.");
                return touch;
            };
            
            touch.step = step;
            //this is kind of lame:
            auto videoPositionNormalized = _screen_to_video_position_normalized(normalizedPoint);
            touch.imageOrigin = _denormalize_point_within_size(videoPositionNormalized, m_frameSize);
            touch.screenOrigin = normalizedPoint;
            touch.srcSize = m_outputSize;
            
            touch.creationIndex = m_ppCreationIndex;
            touch.creationTime = sys_usec_time();
            
            auto asset =  m_scene->getSelectedAsset();
            if (asset) {
                touch.associatedAssetID = asset->getAssetID();
            }
            
            m_ppCreationIndex ++;
            
            return touch;
        }
        
        void _touched_at(const cv::Point2f &normalizedPoint, ToolMode toolMode, TouchStep step) {
            
            if (!m_scene) return;

            TouchPoint touch = _common_touches(normalizedPoint, step);
            touch.toolMode = toolMode;
            
            bool isFirstTouch = step == TouchStepBegan || step == TouchStepTapped;
            
            if (!isFirstTouch) {
                touch.hits = m_lastTouch.hits;
            }

            if (auto asset =  m_scene->getSelectedAsset()) {
                if (isFirstTouch) {
                    for (const auto testType : asset->getHitTypes()) {
                        auto hitResults = m_parent->getCallback()->hitTestAtPoint(normalizedPoint, testType);
                        touch.hits.insert(touch.hits.end(), hitResults.begin(), hitResults.end());
                    }
                }
                asset->touchedAtPoint(touch);
            }

            m_lastTouch = touch;
            
            if (step == TouchStepBegan) {
                if (auto colorAnalyzer = m_parent->getAnalyzerOfType<CBP_AmbienceSampler>()) {
                    std::thread([colorAnalyzer](){
                        colorAnalyzer->recalculate();
                    }).detach();
                }
            }
            
#if DEBUG_TOUCH_POINT
            CBLog("Touched video at (%.1f, %.1f) in (%dx%d)", m_lastTouch.imageOrigin.x, m_lastTouch.imageOrigin.y,
                  m_parent->getFrameSize().width,  m_parent->getFrameSize().height);
#endif
        }
        
        void _rotate_gesture(float amount, const cv::Point2f &normalizedPoint, TouchStep step) {
            TouchPoint touch = _common_touches(normalizedPoint, step);
            touch.rotation = amount;
            
            if (auto asset =  m_scene->getSelectedAsset()) {
                asset->rotatedBy(touch);
            }
        }
        
        void _rotated_by(float amount) {
            if (auto asset =  m_scene->getSelectedAsset()) {
                asset->rotatedBy(amount);
            }
        }
        
        void _clear_all() {
            if (auto scene = m_scene) {
                for (auto itr : scene->getAssets()) {
                    auto renderer = itr.second->getRenderer();
                    std::thread([renderer](){
                        renderer->clearAll();
                    }).detach();
                }
            }
            
            if (auto surfaceAnalyzer = m_parent->getAnalyzerOfType<CBP_SurfaceAnalyzer>()) {
                std::thread([surfaceAnalyzer](){
                    surfaceAnalyzer->clearAll();
                }).detach();
            }
            
            auto areaAnalyzers = m_parent->getAnalyzersOfType<CBP_AreaAnalyzer>();
            for (auto analyzer : areaAnalyzers) {
                analyzer->needsRefresh();
            }
            
        }
        
        cv::Scalar _get_color_in_video_at_point(const cv::Point2f &point) {
            
            auto frame = m_scene->getVideoFrame();
            
            if (frame.empty()) return cv::Scalar::all(0);
            
            auto videoPoint = _screen_to_video_position(point);
            
            auto color = Imaging::meanAtPoint(frame->getRGBImage(), videoPoint, 20);
            color[3] = 255;
            color = Coloring::bgrToRGB(color);
            
            return color;
        }

        Eigen::Matrix4f _get_model_view(int64_t frameIndex) {
            if (frameIndex) {
                if (auto featureTracker = m_parent->getAnalyzerOfType<CBP_FeatureTracker>()) {
                    return featureTracker->getWorldTransform(frameIndex);
                }
            }
            return m_scene->getWorldTransform();
        }
        
        Eigen::Matrix4f _get_projection(int64_t frameIndex) {
            Eigen::Matrix4f proj;
            if (frameIndex) {
                if (auto featureTracker = m_parent->getAnalyzerOfType<CBP_FeatureTracker>()) {
                    proj = featureTracker->getCameraProjection(frameIndex);
                }
            } else {
                proj = m_scene->getCameraProjection();
            }
            
            //hack. iPad issues?
            if (!m_scene->has6DOF()) {
                proj(0,0) *= 0.5;
                proj(1,1) *= 0.5;
            }
            
            return proj;
        }
        
        inline cv::Point2f _normalize_point_within_size(const cv::Point2f &point2D, const cv::Size &size) {
            return cv::Point2f(point2D.x / float(size.width),
                               point2D.y / float(size.height));
        }
        
        inline cv::Point2f _denormalize_point_within_size(const cv::Point2f &point2D, const cv::Size &size) {
            return cv::Point2f(point2D.x * float(size.width),
                               point2D.y * float(size.height));
        }
        
        cv::Point3f _project_point_to_video(const Eigen::Vector3f &point3D, const Eigen::Matrix4f &transform) {
            
            cv::Point3f result3 = _project_point_to_video_normalized(point3D, transform);
            cv::Point2f result2 = _denormalize_point_within_size(cv::Point2f(result3.x, result3.y), m_frameSize);
            
            return cv::Point3f(result2.x, result2.y, result3.z);
        }
        
        cv::Point3f _project_point_to_video_normalized(const Eigen::Vector3f &point3D, const Eigen::Matrix4f &transform) {
            
            cv::Point3f normalizedPoint =  _world_to_view_point_normalized(point3D, transform);
            cv::Point2f result = _screen_to_video_position_normalized(cv::Point2f(normalizedPoint.x, normalizedPoint.y));
            return cv::Point3f(result.x, result.y, normalizedPoint.z);
        }
        
        Eigen::Vector3f _unproject_point_from_video(const cv::Point2f &point2D, float projectedDistance,
                                                const Eigen::Matrix4f &transform, const Eigen::Matrix4f &projectionMatrix) {
            return _unproject_point_from_video_normalized(_normalize_point_within_size(point2D, m_frameSize),
                                                     projectedDistance, transform, projectionMatrix);
        }
        
        Eigen::Vector3f _unproject_point_from_video_normalized(const cv::Point2f &point2D, float projectedDistance,
                                                          const Eigen::Matrix4f &transform,
                                                          const Eigen::Matrix4f &projectionMatrix) {
            
            //rotate point
            cv::Point2f point2DNormalized = _video_to_screen_position_normalized(point2D);
            
            if (projectedDistance == 0) {
                projectedDistance = 2.0f;
            }
            
            Eigen::Vector3f point3D = _view_to_world_point(point2DNormalized,
                                                           projectedDistance,
                                                           transform.inverse(),
                                                           projectionMatrix);
            
            return point3D;
        }
        
        Eigen::Vector3f _view_to_world_point(const cv::Point2f &point2DNormalized,
                                                         float distance,
                                                         const Eigen::Matrix4f &mvpInverse,
                                                         const Eigen::Matrix4f &projection) {
\
            return cbpipe::CBP_MatrixUtil::unprojectPoint(cv::Point2f(point2DNormalized.x + m_2DOffset.x,
                                                                      point2DNormalized.y + m_2DOffset.y),
                                                          distance, mvpInverse, projection);
            
        }
        
        cv::Point3f _world_to_view_point_normalized(const Eigen::Vector3f &point3D, const Eigen::Matrix4f &mvp) {
            
            cv::Point3f result = cbpipe::CBP_MatrixUtil::projectPoint(point3D, mvp);
            
            return cv::Point3f(result.x + m_2DOffset.x, result.y + m_2DOffset.y, result.z);
        }
        
        std::vector<cv::Point> _project_point_cloud(const std::vector<Eigen::Vector3f> &points3D, const Eigen::Matrix4f &transform) {
            std::vector<cv::Point> points2D(points3D.size());
            
            int i = 0;
            for (const auto& point3 : points3D) {
                auto point = _project_point_to_video(point3, transform);
                points2D[i] = cv::Point(point.x, point.y);
                i++;
            }
            return points2D;
        }
        
        std::vector<Eigen::Vector3f> _unproject_point_cloud(const std::vector<cv::Point> &points2D,
                                                         float distance, const Eigen::Matrix4f &mvp, const Eigen::Matrix4f &projectionMatrix) {
            std::vector<Eigen::Vector3f> points3D(points2D.size());
            
            int i = 0;
            for (const auto& point : points2D) {
                points3D[i] = _unproject_point_from_video(point, distance, mvp, projectionMatrix);
                i++;
            }
            return points3D;
        }
        
        cv::Point2f _screen_to_video_position(const cv::Point2f &screenPoint) {
            return _denormalize_point_within_size(_screen_to_video_position_normalized(_normalize_point_within_size(screenPoint, m_outputSize)), m_frameSize);
        }
        
        cv::Point2f _video_to_screen_position(const cv::Point2f &videoPoint) {
            return _denormalize_point_within_size(_video_to_screen_position_normalized(_normalize_point_within_size(videoPoint, m_frameSize)), m_outputSize);
        }
        
        cv::Point2f _screen_to_video_position_normalized(const cv::Point2f &normalizedScreenPoint) {
            
            Eigen::Vector4f point4(normalizedScreenPoint.x - 0.5, normalizedScreenPoint.y - 0.5, 0.0, 1.0);
            Eigen::Vector4f result = m_displayTransform.inverse() * point4;
            
            return cv::Point2f(result.x() + 0.5f, result.y() + 0.5f);
        }
        
        cv::Point2f _video_to_screen_position_normalized(const cv::Point2f &videoPointNormalized) {
            
            Eigen::Vector4f point4(videoPointNormalized.x - 0.5f, videoPointNormalized.y - 0.5f, 0.0, 1.0);
            Eigen::Vector4f result = m_displayTransform * point4;
            
            return cv::Point2f(result.x() + 0.5f, result.y() + 0.5f);
        }
        
        void _get_video_bounding_box(const Eigen::Vector3f &cen, const Eigen::Vector3f &ext,  const Eigen::Matrix4f &transform,
                                 std::vector<cv::Point> &videoBounds, std::vector<Eigen::Vector3f> &worldBounds) {
            worldBounds =
            {
                Eigen::Vector3f(cen.x()-ext.x(), cen.y()-ext.y(), cen.z()-ext.z()),
                Eigen::Vector3f(cen.x()+ext.x(), cen.y()-ext.y(), cen.z()-ext.z()),
                Eigen::Vector3f(cen.x()-ext.x(), cen.y()-ext.y(), cen.z()+ext.z()),
                Eigen::Vector3f(cen.x()+ext.x(), cen.y()-ext.y(), cen.z()+ext.z()),
                Eigen::Vector3f(cen.x()-ext.x(), cen.y()+ext.y(), cen.z()-ext.z()),
                Eigen::Vector3f(cen.x()+ext.x(), cen.y()+ext.y(), cen.z()-ext.z()),
                Eigen::Vector3f(cen.x()-ext.x(), cen.y()+ext.y(), cen.z()+ext.z()),
                Eigen::Vector3f(cen.x()+ext.x(), cen.y()+ext.y(), cen.z()+ext.z())
            };
            
            videoBounds = _project_point_cloud(worldBounds, transform);
        }
        
        cv::Rect2f _get_video_bounds(const Eigen::Vector3f &cen, const Eigen::Vector3f &ext,  const Eigen::Matrix4f &transform) {
            
            std::vector<cv::Point> videoBounds;
            std::vector<Eigen::Vector3f> worldBounds;
            
            _get_video_bounding_box(cen, ext, transform, videoBounds, worldBounds);
            
            cv::Point2f min = videoBounds[0];
            cv::Point2f max = videoBounds[0];
            for (const auto &v : videoBounds)
            {
                min.x = fmin(min.x, v.x);
                min.y = fmin(min.y, v.y);
                
                max.x = fmax(max.x, v.x);
                max.y = fmax(max.y, v.y);
            }
            return cv::Rect2f(min.x, min.y, max.x-min.x, max.y-min.y);
        }
        
        CBMutex m_screenshotMutex;
        CBCondition m_screenshotCondition;
        bool m_screenshotSuccess = false;
        
        void _save_screenshot(const std::string &path, std::function<void(bool, const std::string&)> completion, float maxSeconds) {
            
            CBLog("Saving screenshot to %s", path.c_str());
            std::thread([path, completion, maxSeconds, this](){
                m_screenshotSuccess = false;
                auto start = sys_usec_time();
                m_callback->saveScreenshot(path, false, 100);
                
                m_screenshotMutex.lock();
                while (seconds_elapsed(start) < maxSeconds && !m_screenshotSuccess) {
                    m_screenshotCondition.timedWait(m_screenshotMutex, 100);
                }
                
                bool success = m_screenshotSuccess;
                
                CBLog("Screenshot was %s to %s", (success ? "saved" : "NOT saved"), path.c_str());
                
                m_screenshotSuccess = false;
                m_screenshotMutex.unlock();
                completion(success, path);
            }).detach();
        }
        
        void _screenshot_saved() {
            m_screenshotMutex.lock();
            m_screenshotSuccess = true;
            m_screenshotCondition.signal();
            m_screenshotMutex.unlock();
        }
        
        Eigen::Vector3f m_heading = Eigen::Vector3f::Zero();
        
        Eigen::Vector3f _get_last_heading() {
            return m_heading;
        }
        
        void _update_heading(const Eigen::Vector3f &heading) {
            m_heading = heading;
        }
    };
    
    CBP_RenderingEngine::CBP_RenderingEngine(cbar::CBAR_Client *ar_view, std::shared_ptr<cbar::CBAR_CallbackI> callback)
    {
        CBLogAllocation("Allocating CBP_RenderingEngine");
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, ar_view, callback));
        
        m_analyzer = std::unique_ptr<CBP_Analyzer>(new CBP_Analyzer());
    }
    
    CBP_RenderingEngine::~CBP_RenderingEngine() {
        CBLogAllocation("Deallocating CBP_RenderingEngine");
    }
    
    bool CBP_RenderingEngine::appendAnalyzer(std::shared_ptr<CBP_AnalyzerThread> analyzer) {
        return m_analyzer->appendAnalyzer(analyzer);
    }
    
    bool CBP_RenderingEngine::removeAnalyzer(std::shared_ptr<CBP_AnalyzerThread> analyzer) {
        return m_analyzer->removeAnalyzer(analyzer);
    }
    
    std::shared_ptr<CBP_RenderingEngine> CBP_RenderingEngine::sharedInstance() {
        return staticInstance.lock();
    }
    
    void CBP_RenderingEngine::initialize(std::shared_ptr<CBP_RenderingEngine> renderer) {
        staticInstance = renderer;
    }
    
    void CBP_RenderingEngine::prepareViewport(void *nwh, void *glContext,
                                              int frameWidth, int frameHeight,
                                              int outputWidth, int outputHeight,
                                              float hFOV, int rotation,
                                              const Eigen::Matrix3f &cameraIntrinsics) {
        
        m_pImpl->_prepare_viewport(nwh, glContext,
                                 frameWidth, frameHeight,
                                 outputWidth, outputHeight,
                                 hFOV, rotation, cameraIntrinsics);
        
    }
    
    void CBP_RenderingEngine::addFrame(const cbar::RawFrame &frameData) {
        m_pImpl->_add_frame_data(frameData);
    }
    
    void CBP_RenderingEngine::stillRender() {
        
    }
    
    cbar::CBAR_Client * CBP_RenderingEngine::getARView() const {
        return m_pImpl->m_arView;
    }
    
    void CBP_RenderingEngine::touchedAt(const cv::Point2f &normalizedPoint, ToolMode toolMode, TouchStep step) {
        return m_pImpl->_touched_at(normalizedPoint, toolMode, step);
    }
    
    void CBP_RenderingEngine::rotateGesture(float amount, const cv::Point2f &normalizedPoint, TouchStep step) {
        m_pImpl->_rotate_gesture(amount, normalizedPoint, step);
    }
    
    void CBP_RenderingEngine::rotatedBy(float amount) {
        m_pImpl->_rotated_by(amount);
    }
    
    int CBP_RenderingEngine::getFrameRotation() const {
        return m_pImpl->m_rotation;
    }
    
    int CBP_RenderingEngine::getOutputRotation() const {
        return -90;
    }
    
    cv::Size_<float> CBP_RenderingEngine::getFOV() const {
        return m_pImpl->m_fov;
    }
    
    float CBP_RenderingEngine::getAngularSpeed(int64_t deltaFrames) {
        return m_pImpl->_get_angular_speed(deltaFrames);
    }
    
    float CBP_RenderingEngine::getTranslationalSpeed(int64_t deltaFrames) {
        return m_pImpl->_get_translational_speed(deltaFrames);
    }
    
    int CBP_RenderingEngine::getDebugMode() const {
        return m_pImpl->_get_debug_mode();
    }
    
    void CBP_RenderingEngine::changeDebugMode(bool isOn) {
        m_pImpl->_set_debug_mode(isOn);
    }
    
    cv::Size CBP_RenderingEngine::getFrameSize() const {
        return m_pImpl->m_frameSize;
    }
    
    cv::Size CBP_RenderingEngine::getOutputSize() const {
        return m_pImpl->m_outputSize;
    }
    
    void CBP_RenderingEngine::startRunning(bool videoMode) {
        m_pImpl->_start_running(videoMode);
    }
    
    void CBP_RenderingEngine::stopRunning() {
        m_pImpl->_stop_running();
    }
    
    void CBP_RenderingEngine::pauseRendering(bool pause) {
        if (pause != m_pImpl->m_isRenderingPaused) {
            m_pImpl->_pause_rendering(pause);
        }
    }
    
    bool CBP_RenderingEngine::isRenderingPaused() const {
        return m_pImpl->m_isRenderingPaused;
    }
    
    void CBP_RenderingEngine::setStillMode(bool value) {
        m_pImpl->m_isStillMode = value;
    }
    
    std::shared_ptr<cbscene::CBAR_Scene> CBP_RenderingEngine::getScene() const {
        return m_pImpl->m_scene;
    }
    
    void CBP_RenderingEngine::setScene(std::shared_ptr<cbscene::CBAR_Scene> scene) {
        m_pImpl->_set_scene(scene);
    }
    
    void CBP_RenderingEngine::setLighting(const LightingType &lighting) {
        
    }
    
    void CBP_RenderingEngine::captureCurrentState() {
        m_pImpl->_capture_current_state();
        m_pImpl->m_isStillMode = true;
    }
    
    void CBP_RenderingEngine::clearAll() {
        m_pImpl->_clear_all();
    }
    
    const std::shared_ptr<cbar::CBAR_CallbackI> CBP_RenderingEngine::getCallback() const {
        return m_pImpl->m_callback;
    }
    
    bool CBP_RenderingEngine::isStillMode() const {
        return m_pImpl->m_isCapturing || m_pImpl->m_isStillMode;
    }
    
    cv::Scalar CBP_RenderingEngine::getColorInVideoAtPoint(const cv::Point2f &point) {
        return m_pImpl->_get_color_in_video_at_point(point);
    }
    
    void CBP_RenderingEngine::fitImageToOutput(const cv::Mat &src, cv::Mat &dest) const {
        m_pImpl->_fit_image_to_output(src, dest);
    }
    
    void CBP_RenderingEngine::writePNG(const std::string &path, const cv::Mat &src) const {
        Directory::ensure_parent_dir_exists(path);
        
        std::vector<int> scene_params;
        scene_params.push_back(cv::IMWRITE_PNG_STRATEGY);
        scene_params.push_back(cv::IMWRITE_PNG_STRATEGY_RLE);
        
        cv::imwrite(path.c_str(), src, scene_params);
    }
    
    Eigen::Vector3f CBP_RenderingEngine::unprojectPoint(const cv::Point2f &point2D, float projectedDistance, int64_t frameIndex) {
        auto proj = m_pImpl->_get_projection(frameIndex);
        auto view = m_pImpl->_get_model_view(frameIndex);
        return m_pImpl->_unproject_point_from_video(point2D, projectedDistance, proj * view, proj);
    }
    
    cv::Point3f CBP_RenderingEngine::projectPoint(const Eigen::Vector3f &point3D, int64_t frameIndex) {
        auto proj = m_pImpl->_get_projection(frameIndex);
        auto view = m_pImpl->_get_model_view(frameIndex);
        return m_pImpl->_project_point_to_video(point3D, proj * view);
    }
    
    std::vector<Eigen::Vector3f> CBP_RenderingEngine::unprojectPointCloud(const std::vector<cv::Point> &points,
                                                                          float projectedDistance, int64_t frameIndex) {
        auto proj = m_pImpl->_get_projection(frameIndex);
        auto view = m_pImpl->_get_model_view(frameIndex);
        return m_pImpl->_unproject_point_cloud(points, projectedDistance, proj * view, proj);
    }
    
    std::vector<cv::Point> CBP_RenderingEngine::projectPointCloud(const std::vector<Eigen::Vector3f> &points, int64_t frameIndex) {
        auto proj = m_pImpl->_get_projection(frameIndex);
        auto view = m_pImpl->_get_model_view(frameIndex);
        return m_pImpl->_project_point_cloud(points, proj * view);
    }
    
    void CBP_RenderingEngine::getVideoBoundingBox(const Eigen::Vector3f &cen, const Eigen::Vector3f &ext,
                             std::vector<cv::Point> &videoBounds, std::vector<Eigen::Vector3f> &worldBounds, int64_t frameIndex) {
        auto proj = m_pImpl->_get_projection(frameIndex);
        auto view = m_pImpl->_get_model_view(frameIndex);
        m_pImpl->_get_video_bounding_box(cen, ext, proj * view, videoBounds, worldBounds);
    }
    
    cv::Rect2f CBP_RenderingEngine::getVideoBounds(const Eigen::Vector3f &cen, const Eigen::Vector3f &ext, int64_t frameIndex) {
        auto proj = m_pImpl->_get_projection(frameIndex);
        auto view = m_pImpl->_get_model_view(frameIndex);
        return m_pImpl->_get_video_bounds(cen, ext, proj * view);
    }
    
    void CBP_RenderingEngine::setPointCloudData(const std::map<uint64_t, Eigen::Vector3f> &points) {
        if (auto featureTracker = getAnalyzerOfType<CBP_FeatureTracker>()) {
            std::thread([featureTracker, points](){
                featureTracker->setPointCloudData(points);
            }).detach();
        }
    }
    
    cv::Point2f CBP_RenderingEngine::screenToVideoPosition(const cv::Point2f &screenPoint) {
        return m_pImpl->_screen_to_video_position(screenPoint);
    }
    
    cv::Point2f CBP_RenderingEngine::screenToVideoPositionNormalized(const cv::Point2f &screenPointNormalized) {
        return m_pImpl->_screen_to_video_position_normalized(screenPointNormalized);
    }
    
    cv::Point2f CBP_RenderingEngine::videoToScreenPosition(const cv::Point2f &videoPoint) {
        return m_pImpl->_video_to_screen_position(videoPoint);
    }
    
    cv::Point2f CBP_RenderingEngine::videoToScreenPositionNormalized(const cv::Point2f &videoPointNormalized) {
        return m_pImpl->_video_to_screen_position_normalized(videoPointNormalized);
    }
    
    void CBP_RenderingEngine::showPoints(const std::map<uint64_t, Eigen::Vector3f> &points, const cv::Scalar &color) {
        
    }
    
    cv::Point2f CBP_RenderingEngine::get2DOffset() const {
        return m_pImpl->m_2DOffset;
    }
    
    void CBP_RenderingEngine::set2DOffset(const cv::Point2f &offset) {
        m_pImpl->m_2DOffset = offset;
    }
    
    void CBP_RenderingEngine::planeAnchorAdded(const PlaneAnchor &anchordata) {
        if (auto planeAnalyzer = getAnalyzerOfType<CBP_PlaneAnalyzer>()) {
            std::thread([planeAnalyzer, anchordata](){
                planeAnalyzer->anchorAdded(anchordata);
            }).detach();
        }
    }
    
    void CBP_RenderingEngine::planeAnchorUpdated(const PlaneAnchor &anchordata) {
        if (auto planeAnalyzer = getAnalyzerOfType<CBP_PlaneAnalyzer>()) {
            std::thread([planeAnalyzer, anchordata](){
                planeAnalyzer->anchorUpdated(anchordata);
            }).detach();
        }
    }
    
    void CBP_RenderingEngine::planeAnchorRemoved(const PlaneAnchor &anchordata) {
        if (auto planeAnalyzer = getAnalyzerOfType<CBP_PlaneAnalyzer>()) {
            std::thread([planeAnalyzer, anchordata](){
                planeAnalyzer->anchorRemoved(anchordata);
            }).detach();
        }
    }
    
    void CBP_RenderingEngine::userAnchorAdded(const UserAnchor &anchordata) {
        
    }
    
    void CBP_RenderingEngine::userAnchorUpdated(const UserAnchor &anchordata) {
        
    }
    
    void CBP_RenderingEngine::userAnchorRemoved(const UserAnchor &anchordata) {
        
    }
    
    TouchPoint CBP_RenderingEngine::getLastTouch() const {
        return m_pImpl->m_lastTouch;
    }
    
    float CBP_RenderingEngine::getFPS() const {
        return m_pImpl->m_fpsTimer.getFPS();
    }
    
    void CBP_RenderingEngine::saveScreenshot(const std::string &path,
                                                          std::function<void(bool, const std::string&)> completion,
                                                          float maxSeconds) {
        m_pImpl->_save_screenshot(path, completion, maxSeconds);
    }
    
    void CBP_RenderingEngine::screenshotSaved() {
        m_pImpl->_screenshot_saved();
    }
    
    void CBP_RenderingEngine::updateHeading(const Eigen::Vector3f &heading) {
        m_pImpl->_update_heading(heading);
    }
    
    Eigen::Vector3f CBP_RenderingEngine::getLastHeading() {
        return m_pImpl->_get_last_heading();
    }
};

