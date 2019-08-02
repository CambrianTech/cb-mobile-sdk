#ifndef __SurfacePainter__
#define __SurfacePainter__

#include <cambrian.h>
#include <imaging/Geometry.h>
#include <cbcommon/CB_Types.h>
#include <imaging/ImageProcessing.h>
#include <utility/Threads.h>
#include <imaging/ImageDefines.h>
#include <cbar/CBAR_VideoThread.hpp>
#include <cbar/CBAR_Common.hpp>
#include <queue>

#include <cbar/CBAR_Client.hpp>
#include <cbar/CBAR_CallbackI.hpp>

#include <cbar/pipeline/CBP_Types.hpp>
#include <cbar/scene/scene.h>

using namespace imaging;

namespace cbpipe {
    class CBP_RenderingEngine;
}

namespace cbar {
    
    class DLL_PUBLIC CBAR_View : public CBAR_Client
    {
        
    public:
        CBAR_View(std::shared_ptr<CBAR_CallbackI> callback);
        ~CBAR_View();
        
        void setScene(std::shared_ptr<cbscene::CBAR_Scene> scene);
        std::shared_ptr<cbscene::CBAR_Scene> getScene() const;
        
        bool isRunning();
        void startRunning(bool videoMode);
        void stopRunning();
        
        //OpenGL
        void prepareViewport(void *nwh, void *glContext,
                             int frameWidth, int frameHeight,
                             int outputWidth, int outputHeight,
                             float deviceFOV, int rotation,
                             const Eigen::Matrix3f &cameraIntrinsics = Eigen::Matrix3f::Identity());
        
        void addFrame(const cbar::RawFrame &frame);
        void stillRender();
        
        void screenshotSaved();
        
        virtual ToolMode getToolMode();
        virtual void setToolMode(ToolMode toolMode);
        
        virtual double getBrushRadius();
        virtual void setBrushRadius(double brushSize);
        
        LightingType getLighting() const;
        void setLighting(const LightingType &);
        
        void touchedAt(const cv::Point2f &normalizedPoint, TouchStep step);
        void tappedAt(const cv::Point2f &normalizedPoint);
        void rotateGesture(float amount, const cv::Point2f &normalizedPoint, TouchStep step);
        void rotatedBy(float amount);
        
        void captureCurrentState();
        
        void clearAll();
        
        const std::shared_ptr<cbpipe::CBP_RenderingEngine> renderingPipeline();
        
        std::vector<cv::Point3f> getPaintPoints(PaintPointType type = PaintPointType_All);
        std::vector<TapPoint> getTapPoints();
        
        int getDebugMode() const;
        void changeDebugMode(bool forward);
        
        bool isFocusing() const;
        void setIsFocusing(bool focusing);
        
        void updateHeading(const Eigen::Vector3f &heading);
        
        void setPointCloudData(const std::map<uint64_t, Eigen::Vector3f> &points);
        
        void planeAnchorAdded(const PlaneAnchor &anchordata);
        void planeAnchorUpdated(const PlaneAnchor &anchordata);
        void planeAnchorRemoved(const PlaneAnchor &anchordata);
        
        void userAnchorAdded(const UserAnchor &anchordata);
        void userAnchorUpdated(const UserAnchor &anchordata);
        void userAnchorRemoved(const UserAnchor &anchordata);
        
        void pauseRendering(bool isPaused);
        bool isRenderingPaused() const;
        
        bool isStillMode() const;
        void setStillMode(bool);
        
        cv::Scalar getColorInVideoAtPoint(const cv::Point2f &point);
        
        cv::Point2f get2DOffset() const;
        void set2DOffset(const cv::Point2f &offset);
        
    private:
        
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
};
#endif
