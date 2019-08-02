//
//  CBP_SurfaceRenderer.hpp
//  Cambrian
//
//  Created by Joel Teply on 11/13/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#ifndef CBP_SurfaceRenderer_hpp
#define CBP_SurfaceRenderer_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>

#include <cambrian.h>
#include <cbar/pipeline/rendering/CBP_Renderer.hpp>
#include <cbar/pipeline/motion/motion.h>

namespace cbpipe {
    class CBP_RenderingEngine;
    class CBP_Segmenter;
    class PaintPoint;
    class SegmentationResult;
    
    class DLL_LOCAL CBP_SurfaceRenderer : public CBP_Renderer, public CBP_AmbienceReceiver {
    public:
        CBP_SurfaceRenderer(cbscene::CBAR_Asset *asset, std::string name);
        ~CBP_SurfaceRenderer();
        
        virtual bool handleFrame(cbar::CBAR_VideoFramePtr frame);
        
        virtual void touchedAtPoint(TouchPoint &touch);
        virtual void clearAll();
        
        virtual void captureState(std::shared_ptr<cbpipe::UndoState> state);
        virtual void reloadState(std::shared_ptr<cbpipe::UndoState> state);
        
        virtual void render(bool isStillMode);
        
        int getIndex();
        void releaseIndex();
        
        virtual PaintPointType getType()  {
            return PaintPointType_Surface;
        };
        
        virtual cv::Mat getMask();
        virtual void ambienceCalculationUpdated();
    protected:
        
        virtual cv::Scalar getDrawColor(const TouchPoint &pp) { return cv::Scalar::all(255); };
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif /* CBP_SurfaceRenderer_hpp */
