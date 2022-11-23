//
//  CBAR_View.cpp
//  Cambrian
//
//  Created by Joel Teply on 1/16/13.
//
//

#include "CBAR_View.hpp"
#include <imaging/ImageProcessing.h>
#include <imaging/ImageDefines.h>
#include <imaging/Imaging.h>
#include <imaging/Drawing.h>
#include <utility/CommonUtility.h>
#include <imaging/LineProcessing.h>
#include <imaging/Accelerated.h>
#include <imaging/Geometry.h>
#include <cbcommon/CB_Config.h>
#include <utility/Directory.h>

#include <cbar/pipeline/pipeline.h>
#include <cbar/pipeline/util/CBP_MatrixUtil.hpp>

using namespace imaging;
using namespace cbpipe;

namespace cbar {
    
    struct CBAR_View::Impl
    {
        Impl(CBAR_View *parent, std::shared_ptr<CBAR_CallbackI> callback) : m_parent(parent)
        {
            m_isFlooring = false;
            
            m_isRunning = false;
            m_tappedImage = false;
            
            m_renderingPipeline = std::shared_ptr<CBP_RenderingEngine>(new cbpipe::CBP_RenderingEngine(parent, callback));
            m_renderingPipeline->initialize(m_renderingPipeline);
        }
        ~Impl() {
            
        }
        
        CBAR_View *m_parent;
        
        std::shared_ptr<CBP_RenderingEngine> m_renderingPipeline;
        
        bool m_tappedImage;
        bool m_isFlooring;
        
        bool m_isHighQualityCamera;
        
        cv::Size m_frameSize;
        
        //optical flow
        bool m_isRunning = false;
        CBMutex m_startStopMutex;
        bool m_isStopping = false;
        
        bool m_isFocusing = false;
        
        ToolMode m_toolMode = ToolModeSelect;
        double m_brushSize = 60;
        
        std::function<void (std::shared_ptr<cbscene::CBAR_Scene>)> m_captureCallback;
        
        void start_running(bool startLive) {
            if (m_renderingPipeline->isStillMode() != startLive && (is_running() || m_isStopping)) return; //already started
            
            CBLog("Starting CBAR_View");
            
            srand (int(time(NULL)));
            
            m_tappedImage = false;
            
            std::lock_guard<CBMutex> lockGuard(m_startStopMutex);
            m_isRunning = true;
            m_renderingPipeline->startRunning(startLive);
            
#if REPRIORITIZE_THREADS
            //set thread priorities
            auto threads = CBThread::getRunningThreads();
            
            //skip main thread
            for (int i=1; i<threads.size(); i++) {
                auto name = CBThread::getThreadName(threads[i]);
                //non cambrian or apple thread:
                if (name.rfind("CB", 0) == 0 || name.rfind("com.apple", 0) == 0) {
                    //Cambrian thread, already set
                } else if (name.length() == 0 || name.rfind("BackgroundWorker", 0) == 0) {
                    CBThread::setPThreadPriority(threads[i], CBThreadPriorityLowest);
                } else {
                    CBThread::setPThreadPriority(threads[i], CBThreadPriorityLow);
                }
            }
#endif
         }
        
        bool is_running() {
            bool isRunning;
            std::lock_guard<CBMutex> lockGuard(m_startStopMutex);
            isRunning = m_isRunning;
            return isRunning;
        }
        
        void stop_running() {
            
            if (!m_isRunning || m_isStopping) return; //already stopped
            
            m_isStopping = true;
            CBLog("Stopping CBAR_View");
            
            { // scope the lock_guard
                std::lock_guard<CBMutex> lockGuard(m_startStopMutex);
                m_isRunning = false;
                m_renderingPipeline->stopRunning();
            
                //        if (!_renderingPipeline.empty()) {
                //            _renderingPipeline->cleanup();
                //            _renderingPipeline.release();
                //        }
            
                m_isStopping = false;
            }

            CBLog("CBAR_View Stopped");
        }
        
        void capture_current_state() {
            m_renderingPipeline->captureCurrentState();
        }
        
        int get_debug_mode() const {
            return m_renderingPipeline->getDebugMode();
        }
        
        void change_debug_mode(bool forward) {
            m_renderingPipeline->changeDebugMode(forward);
        }
        
        void touched_at(cv::Point2f touchPointNormalized, TouchStep step) {
            if (!m_isRunning) return;
            
            m_renderingPipeline->touchedAt(touchPointNormalized, m_toolMode, step);
        }
        
        void rotate_gesture(float amount, const cv::Point2f &normalizedPoint, TouchStep step) {
            m_renderingPipeline->rotateGesture(amount, normalizedPoint, step);
        }

        void rotated_by(float amount) {
            m_renderingPipeline->rotatedBy(amount);
        }
        
        cv::Rect get_crop(int inputWidth, int inputHeight, int outputWidth, int outputHeight) {
            
            float inputAspectRatio = float(inputWidth) / float(inputHeight);
            float outputAspectRatio = float(outputWidth) / float(outputHeight);
            
            int cropWidth = inputWidth;
            int cropHeight = inputHeight;
            if (inputAspectRatio > outputAspectRatio) {
                cropWidth = cropWidth * (outputAspectRatio / inputAspectRatio);
            } else {
                cropHeight = cropHeight * (inputAspectRatio / outputAspectRatio);
            }
            
            cv::Rect crop = cv::Rect((inputWidth - cropWidth) / 2, (inputHeight - cropHeight) / 2, cropWidth, cropHeight);
            
            return crop;
        }
        
        void prepare_viewport(void *nwh, void *glContext,
                             int frameWidth, int frameHeight,
                             int outputWidth, int outputHeight,
                             float deviceFOV, int rotation,
                             const Eigen::Matrix3f &cameraIntrinsics) {
            
            if (m_renderingPipeline) {
                m_renderingPipeline->prepareViewport(nwh, glContext, frameWidth, frameHeight, outputWidth, outputHeight, deviceFOV, rotation, cameraIntrinsics);
            }
        }
        
        void add_frame(const cbar::RawFrame &frame) {
            if (m_renderingPipeline && m_isRunning) {
                m_renderingPipeline->addFrame(frame);
            }
        }
        
        std::vector<cv::Point3f> get_paint_points(PaintPointType type) {
            
            std::vector<cv::Point3f>points;
            
            if (m_renderingPipeline) {
                //auto paintPoints = m_renderingPipeline->getPaintPoints();
                //points = cbpipe::container_cast<std::vector<cv::Point3f>>(paintPoints);
            }
            return points;
        }
        
        void pause_rendering(bool isPaused) {
            if (m_renderingPipeline) {
                m_renderingPipeline->pauseRendering(isPaused);
            }
        }
        
        bool is_rendering_paused() const {
            if (m_renderingPipeline) {
                return m_renderingPipeline->isRenderingPaused();
            }
            return false;
        }
    };
    
    CBAR_View::CBAR_View(std::shared_ptr<CBAR_CallbackI> callback) : CBAR_Client()
    {
        printf("Initializing CBAR_View");
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, callback));
    }
    
    CBAR_View::~CBAR_View()
    {
        CBLog("Destroying CBAR_View");
        if (isRunning()) stopRunning();
        
        CBLog("CBAR_View Destroyed");
    }
    
    void CBAR_View::startRunning(bool startLive) {
        m_pImpl->start_running(startLive);
    }
    
    bool CBAR_View::isRunning() {
        return m_pImpl->is_running();
    }
    
    void CBAR_View::stopRunning() {
        m_pImpl->stop_running();
    }
    
    bool CBAR_View::isStillMode() const {
        if (m_pImpl->m_renderingPipeline) {
            return m_pImpl->m_renderingPipeline->isStillMode();
        }
        return false;
    }
    
    void CBAR_View::setStillMode(bool value) {
        if (m_pImpl->m_renderingPipeline) {
            m_pImpl->m_renderingPipeline->setStillMode(value);
        }
    }
    
    int CBAR_View::getDebugMode() const {
        return m_pImpl->get_debug_mode();
    }
    
    void CBAR_View::changeDebugMode(bool forward) {
        m_pImpl->change_debug_mode(forward);
    }
    
    ToolMode CBAR_View::getToolMode() {
        return m_pImpl->m_toolMode;
    }
    
    void CBAR_View::setToolMode(ToolMode toolMode) {
        m_pImpl->m_toolMode = toolMode;
    }
    
    double CBAR_View::getBrushRadius() {
        return m_pImpl->m_brushSize;
    }
    
    void CBAR_View::setBrushRadius(double brushSize) {
        m_pImpl->m_brushSize = brushSize;
    }
    
    void CBAR_View::touchedAt(const cv::Point2f &touchPointNormalized, TouchStep step) {
        m_pImpl->touched_at(touchPointNormalized, step);
    }
    
    void CBAR_View::tappedAt(const cv::Point2f &touchPointNormalized) {
        m_pImpl->touched_at(touchPointNormalized, TouchStepTapped);
    }
    
    void CBAR_View::rotateGesture(float amount, const cv::Point2f &normalizedPoint, TouchStep step) {
        m_pImpl->rotate_gesture(amount, normalizedPoint, step);
    }

    void CBAR_View::rotatedBy(float amount) {
        m_pImpl->rotated_by(amount);
    }
    
    void CBAR_View::clearAll() {
        if (m_pImpl->m_renderingPipeline) {
            m_pImpl->m_renderingPipeline->clearAll();
        }
    }
    
    void CBAR_View::prepareViewport(void *nwh, void *glContext,
                                    int frameWidth, int frameHeight,
                                    int outputWidth, int outputHeight,
                                    float deviceFOV, int rotation,
                                    const Eigen::Matrix3f &cameraIntrinsics) {
        
        m_pImpl->prepare_viewport(nwh, glContext,
                                 frameWidth, frameHeight,
                                 outputWidth, outputHeight,
                                 deviceFOV, rotation, cameraIntrinsics);
        
    }
    
    void CBAR_View::addFrame(const cbar::RawFrame &frame)
    {
        m_pImpl->add_frame(frame);
    }
    
    void CBAR_View::stillRender() {
        if (m_pImpl->m_renderingPipeline) {
            m_pImpl->m_renderingPipeline->stillRender();
        }
    }
    
    std::vector<cv::Point3f> CBAR_View::getPaintPoints(PaintPointType type) {
        return m_pImpl->get_paint_points(type);
    }
    
    void CBAR_View::pauseRendering(bool isPaused) {
        m_pImpl->pause_rendering(isPaused);
    }
    
    bool CBAR_View::isRenderingPaused() const {
        return m_pImpl->is_rendering_paused();
    }
    
    void CBAR_View::setScene(std::shared_ptr<cbscene::CBAR_Scene> scene) {
        //TODO: Allow scenes to be swapped out without the channel merging crashing. Temp fix in swift code
        // is to call stopRunning() on ARView before setting the new scene. Doing the same in cpp crashes
        m_pImpl->m_renderingPipeline->setScene(scene);
    }
    
    std::shared_ptr<cbscene::CBAR_Scene> CBAR_View::getScene() const {
        return m_pImpl->m_renderingPipeline->getScene();
    }
    
    const std::shared_ptr<CBP_RenderingEngine> CBAR_View::renderingPipeline() {
        return m_pImpl->m_renderingPipeline;
    }
    
    bool CBAR_View::isFocusing() const {
        return m_pImpl->m_isFocusing;
    }
    
    void CBAR_View::setIsFocusing(bool focusing) {
        m_pImpl->m_isFocusing = focusing;
    }
    
    void CBAR_View::captureCurrentState() {
        m_pImpl->capture_current_state();
    }
    
    cv::Scalar CBAR_View::getColorInVideoAtPoint(const cv::Point2f &point) {
        return m_pImpl->m_renderingPipeline->getColorInVideoAtPoint(point);
    }
    
    void CBAR_View::setPointCloudData(const std::map<uint64_t, Eigen::Vector3f> &points) {
        m_pImpl->m_renderingPipeline->setPointCloudData(points);
    }
    
    cv::Point2f CBAR_View::get2DOffset() const {
        return m_pImpl->m_renderingPipeline->get2DOffset();
    }
    
    void CBAR_View::set2DOffset(const cv::Point2f &offset) {
        m_pImpl->m_renderingPipeline->set2DOffset(offset);
    }
    
    void CBAR_View::planeAnchorAdded(const PlaneAnchor &anchordata) {
        m_pImpl->m_renderingPipeline->planeAnchorAdded(anchordata);
    }
    
    void CBAR_View::planeAnchorUpdated(const PlaneAnchor &anchordata) {
        m_pImpl->m_renderingPipeline->planeAnchorUpdated(anchordata);
    }
    
    void CBAR_View::planeAnchorRemoved(const PlaneAnchor &anchordata) {
        m_pImpl->m_renderingPipeline->planeAnchorRemoved(anchordata);
    }
    
    void CBAR_View::userAnchorAdded(const UserAnchor &anchordata) {
        m_pImpl->m_renderingPipeline->userAnchorAdded(anchordata);
    }
    
    void CBAR_View::userAnchorUpdated(const UserAnchor &anchordata) {
        m_pImpl->m_renderingPipeline->userAnchorUpdated(anchordata);
    }
    
    void CBAR_View::userAnchorRemoved(const UserAnchor &anchordata) {
        m_pImpl->m_renderingPipeline->userAnchorRemoved(anchordata);
    }
    
    void CBAR_View::screenshotSaved() {
        m_pImpl->m_renderingPipeline->screenshotSaved();
    }
    
    void CBAR_View::updateHeading(const Eigen::Vector3f &heading) {
        m_pImpl->m_renderingPipeline->updateHeading(heading);
    }
}
