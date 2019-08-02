//
//  CBP_WallSegmenter.cpp
//  Cambrian
//
//  Created by Joel Teply on 10/12/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#include "CBP_WallSegmenter.hpp"

#include <imaging/ImageProcessing.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Accelerated.h>

#include <cbar/pipeline/motion/motion.h>
#include <cbar/pipeline/machine-learning/machine-learning.h>

#include <opencv2/ml/ml.hpp>

using namespace imaging;

namespace cbpipe {
    
    struct CBP_WallSegmenter::Impl
    {
        Impl(CBP_WallSegmenter *segmenter, const cv::Size &gridUnitSize)
          : m_segmenter(segmenter), m_floodFill(gridUnitSize)
        {}
        
        ~Impl() {}
        
        CBP_WallSegmenter *m_segmenter;
        CBP_WallFloodFill m_floodFill;
        
        int m_floodPaintDiff = 3;
        cv::Point2f m_lastPoint = cv::Point(-1, -1);
        bool m_isCanceled = false;
        
        void cancel() {
            m_floodFill.cancel();
            m_isCanceled = true;
        }
        
        FillResult segmentImage(const cv::Point2f &paintPoint, cbar::CBAR_VideoFramePtr frame) {
            m_isCanceled = false;
            
            cv::Mat smallResult = m_floodFill.run(frame, paintPoint);
            
            FillResult result;
            result.frameIndex = frame->frameIndex;
            
            if (m_isCanceled) return result;
            
            int countPixels = cv::countNonZero(smallResult);
            
            if (countPixels > 5) {
                int maxSize = cv::sqrt(countPixels);
                
                cv::Mat reducedRGB;
                cv::resize(frame->getRGBImage(), reducedRGB, cv::Size(frame->frameSize().width / 2,
                                                                      frame->frameSize().height / 2));
                
                imaging::ImageProcessing::refineMask(smallResult, reducedRGB, 30, fmin(maxSize,25), result.overlay, cv::Size(640,640));
                
                if (m_isCanceled) return result;
                
                std::vector<LineSegment>longLines;
                ImageProcessing::getPolygonalMask(result.overlay, result.overlay, longLines, fmin(maxSize/3, 5), fmin(maxSize, 10));
                
                imaging::ImageProcessing::refineMask(result.overlay, reducedRGB, 3, 5, result.overlay);
                
                if (m_isCanceled) return result;
                
                cv::blur(result.overlay, result.overlay, cv::Size(5,5));
                
                if (m_isCanceled) return result;
                
#if DEBUG_RESULT
                ImageProcessing::overlayMaskOntoRGB(result.overlay, reducedRGB);
                Diagnostics::SaveDiagnosticImage(true, reducedRGB, "result.png");
#endif
                
                cv::resize(result.overlay, result.overlay, frame->frameSize());
            }
            
            return result;
        }
    };
    
    CBP_WallSegmenter::CBP_WallSegmenter(const cv::Size &gridUnitSize)
    : CBP_Segmenter("CBP_WallSegmenter") {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, gridUnitSize));
    }
    
    CBP_WallSegmenter::~CBP_WallSegmenter() {
        
    }
    
    FillResult CBP_WallSegmenter::segmentAtPoint(const cv::Point2f &paintPoint, cbar::CBAR_VideoFramePtr frame) {
        return m_pImpl->segmentImage(paintPoint, frame);
    }
    
    void CBP_WallSegmenter::cancel() {
        m_pImpl->cancel();
    }
};
