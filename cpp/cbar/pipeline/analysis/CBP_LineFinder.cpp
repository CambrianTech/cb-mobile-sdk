//
//  CBP_LineFinder.cpp
//  Cambrian
//
//  Created by Joel Teply on 3/16/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#include "CBP_LineFinder.hpp"

#include <cbar/pipeline/rendering/rendering.h>
#include <cbcommon/CB_Config.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Coloring.h>
#include <cbar/pipeline/util/util.h>
#include <imaging/Imaging.h>

#include <cbar/pipeline/machine-learning/machine-learning.h>

using namespace imaging;

namespace cbpipe {
    
    struct CBP_LineFinder::Impl
    {
        Impl(CBP_LineFinder *parent) : m_parent(parent) {
            
        }
        
        ~Impl() {}
        
        CBP_LineFinder *m_parent;
        int64_t m_lastUpdateFrame = -1;
        
        bool analyze(cbar::CBAR_VideoFramePtr frame) {
            
            auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return false;
            auto surfaceAnalyzers = renderer->getAnalyzersOfType<CBP_SurfaceAnalyzer>(); if (surfaceAnalyzers.empty()) return false;
            
            
            //opencv line segment detector has bugs with memory and must be destroyed
            cv::Mat src;
            cv::pyrDown(frame->getBWImage(), src);
            std::vector<cv::Vec4f> detectedLines;
            {
                auto lsd = cv::createLineSegmentDetector(cv::LSD_REFINE_ADV);
                lsd->detect(src, detectedLines);
            }
            
            Geometry::groupLines(detectedLines, detectedLines);
            
            double minLength = sqrt(src.size().area()) / 8.0;
            int minLengthSQ = minLength * minLength;
            auto numLines = detectedLines.size();
            
            std::vector<cv::Vec4f> finalLines;
            float scale = float(src.cols) / float(frame->getBWImage().cols);
            finalLines.reserve(numLines);
            for (int i=0; i<numLines; i++) {
                const auto& detectedLine = detectedLines[i];
                cv::Vec4f line = detectedLine / scale;
                double length = Geometry::euclideanDistanceSq(cv::Point2f(line[0], line[1]), cv::Point2f(line[2], line[3]));
                
                if (length < minLengthSQ) continue;
                
                finalLines.push_back(line);
            }
            
            for (auto analyzer : surfaceAnalyzers) {
                analyzer->linesFound(frame, finalLines);
            }
            
            return true;
        }
    };
    
    CBP_LineFinder::CBP_LineFinder() : CBP_AreaAnalyzer("CBP_LineFinder") {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
        
        setPriority(CBThreadPriorityLowest);
    }
    
    CBP_LineFinder::~CBP_LineFinder() {
        
    }
    
    bool CBP_LineFinder::analyze(cbar::CBAR_VideoFramePtr frame) {
        return m_pImpl->analyze(frame);
    }
};
