//
//  CBP_FloorSegmenter.cpp
//  Cambrian
//
//  Created by Joel Teply on 10/12/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#include "CBP_FloorSegmenter.hpp"

#include <imaging/ImageProcessing.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Accelerated.h>
#include <cbar/pipeline/motion/motion.h>
#include <cbar/pipeline/machine-learning/machine-learning.h>

#include <opencv2/ml/ml.hpp>

#define SHOW_DEBUG_IMAGE 0

using namespace imaging;

namespace cbpipe {
    
    struct CBP_FloorSegmenter::Impl
    {
        Impl(CBP_FloorSegmenter *segmenter, const cv::Size &gridUnitSize)
          : m_segmenter(segmenter), m_floodFill(gridUnitSize, 5, 18)
        {}
       ~Impl() {}
        
        CBP_FloorSegmenter *m_segmenter;
        CBP_WallFloodFill m_floodFill;
        
        int m_floodPaintDiff = 3;
        cv::Point2f m_lastPoint = cv::Point(-1, -1);
        bool m_isCanceled = false;
        
        void _cancel() {
            m_floodFill.cancel();
            m_isCanceled = true;
        }
        
        FillResult _segment_image(const cv::Point2f &paintPoint, cbar::CBAR_VideoFramePtr frame) {
            m_isCanceled = false;
            
            cv::Mat smallResult = m_floodFill.run(frame, paintPoint);
            
            FillResult result;
            result.frameIndex = frame->frameIndex;
            
            int countPixels = cv::countNonZero(smallResult);
            
            if (countPixels > 5) {
                int refinementSize = 2 * fmax(frame->frameSize().width/smallResult.cols, frame->frameSize().height/smallResult.rows);
                imaging::ImageProcessing::refineMask(smallResult, frame->getRGBImage(), refinementSize, refinementSize, result.overlay, cv::Size(200,200));
                
                std::vector<LineSegment>longLines;
                ImageProcessing::getPolygonalMask(result.overlay, result.overlay, longLines, 13, 9);
                cv::dilate(result.overlay, result.overlay, 7, cv::Point(-1,-1), 1, cv::BORDER_REFLECT);
                
                cv::GaussianBlur(result.overlay, result.overlay, cv::Size(9,9), 5);
                
                result.reflections = ImageProcessing::createShadowsAndHighlights(frame->getRGBImage());
                
#if SHOW_DEBUG_IMAGE
                cv::Mat debugImage = frame->getRGBImage();
                ImageProcessing::overlayBWOntoRGB(result.overlay, debugImage);
                Diagnostics::SaveDiagnosticImage(true, debugImage, "debug.png");
#endif
            }
            
            return result;
        }
    };
    
    CBP_FloorSegmenter::CBP_FloorSegmenter(const cv::Size &gridUnitSize)
    : CBP_Segmenter("CBP_FloorSegmenter") {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, gridUnitSize));
    }
    
    CBP_FloorSegmenter::~CBP_FloorSegmenter() {
        
    }
    
    FillResult CBP_FloorSegmenter::segmentAtPoint(const cv::Point2f &paintPoint, cbar::CBAR_VideoFramePtr frame) {
        return m_pImpl->_segment_image(paintPoint, frame);
    }
    
    void CBP_FloorSegmenter::cancel() {
        m_pImpl->_cancel();
    }
};
