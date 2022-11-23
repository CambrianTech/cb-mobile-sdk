//
//  FloodFillAlgorithm.cpp
//  Cambrian
//
//  Created by Joel Teply on 10/23/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#include "CBP_FloodFillAlgorithm.hpp"

#include <cbar/pipeline/rendering/rendering.h>
#include <imaging/ImageProcessing.h>
#include <utility/Directory.h>
#include <utility/Diagnostics.h>
#include "CBP_MLUtility.hpp"
//#include <opencv2/dnn/dnn.hpp>
#include "CBP_FloodFillIterator.hpp"

#include <fstream>

namespace cbpipe {
    
    struct CBP_FloodFillAlgorithm::Impl
    {
        Impl(CBP_FloodFillAlgorithm *floodFill, PaintPointType type, const cv::Size &gridUnitSize, const cv::Size &maxImageSize)
            :  m_floodFill(floodFill), m_type(type), m_gridUnitSize(gridUnitSize), m_maxImageSize(maxImageSize)
        {

        }
        ~Impl() {
            
        }
        
        cv::Mat _run_grid(cv::Ptr<texture_grid> grid, bool isVideo) {
            
            auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return cv::Mat();
            
            //push back initial samples
            std::vector<texture_sample*>sampleStack;
            for (int i=0; i<grid->seeds.size(); i++) {
                auto &seed = *grid->seeds[i];
                sampleStack.push_back(&seed);
            }
            
            if (m_floodFill->m_isCanceled) return cv::Mat();
            
            m_floodFill->prepare(grid, isVideo);
            
            if (m_floodFill->m_isCanceled) return cv::Mat();
            
            std::vector<texture_sample*>initialSampleStack = sampleStack;
            
            //flood fill some
            CBP_FloodFillIterator iterator;
            
            int count = 0;
            while (initialSampleStack.size() && !m_floodFill->m_isCanceled) {
                texture_sample sample = *initialSampleStack.back();
                count++;
                initialSampleStack.pop_back();
                
                std::vector<texture_sample*>newSamples;
                if (iterator.iterativeAnalysis(grid, sample, newSamples, [count](texture_sample & a, texture_sample & b) {
                    return count < 10;
                })) {
                    sampleStack.insert(sampleStack.begin(), newSamples.begin(), newSamples.end());
                    grid->selectedSamples.insert(grid->selectedSamples.end(), newSamples.begin(), newSamples.end());
                }
            }
            
            if (m_floodFill->m_isCanceled) return cv::Mat();
            
            std::vector<texture_sample *> ffMatches;
            for (auto &sample : sampleStack) {
                sample->isOn = true;
                sample->isSeed = true;
                ffMatches.push_back(sample);
            }
            
            if (!ffMatches.size()) return cv::Mat();
            
            //get initial seed value
            texture_sample replicatedSample = *ffMatches[0];
            replicatedSample.label = INT_MIN;
            m_floodFill->getReplicatedSeedValue(grid, ffMatches, replicatedSample);
            
            if (replicatedSample.label != INT_MIN || !replicatedSample.data.empty()) {
                for (auto &sample : sampleStack) {
                    sample->data = replicatedSample.data;
                    sample->label = replicatedSample.label;
                }
            }
            
            cv::Mat smallResult = cv::Mat::zeros(grid->gridDimensions.height, grid->gridDimensions.width, CV_8UC1);
            
            m_floodFill->getFFMatches(grid, sampleStack, ffMatches);
            smallResult = cv::Mat::zeros(grid->gridDimensions.height, grid->gridDimensions.width, CV_8UC1);
            
            for (int j=0; j<ffMatches.size(); j++) {
                const texture_sample &sample = *ffMatches[j];
                smallResult.at<uchar>(sample.gridIndex) = sample.label > 0 ? sample.label : 1;
            }
            
            
            int filled = cv::countNonZero(smallResult);
            
            if (filled < 5) {
                return cv::Mat();
            }
            
#if DEBUG_FILL
            std::vector<cv::Mat> matches;
            matches.push_back(smallResult);
            auto debugImage = CBP_MLUtility::getDebugImage(grid, matches);
            Diagnostics::SaveDiagnosticImage(true, debugImage, "floodFill.png");
#endif
            return smallResult;
        }
        
        PaintPointType m_type;
        CBP_FloodFillAlgorithm *m_floodFill;
        
        cv::Size m_gridUnitSize;
        cv::Size m_maxImageSize;
    };
    
    CBP_FloodFillAlgorithm::CBP_FloodFillAlgorithm(PaintPointType type,
                                                   const cv::Size &gridUnitSize,
                                                   const cv::Size &maxImageSize) {

        m_pImpl = std::unique_ptr<Impl>(new Impl(this, type, gridUnitSize, maxImageSize));
    }
    
    CBP_FloodFillAlgorithm::~CBP_FloodFillAlgorithm() {

    }
    
    void CBP_FloodFillAlgorithm::setFrame(cbar::CBAR_VideoFramePtr frame, const std::vector<cv::Point2f>&points) {

        _retainedGrid = CBP_MLUtility::createGrid(frame, m_pImpl->m_gridUnitSize, m_pImpl->m_maxImageSize, points);
    }
    
    cv::Mat CBP_FloodFillAlgorithm::segmentImage(const cv::Point2f &paintPoint, cbar::CBAR_VideoFramePtr frame) {
        if (frame.empty()) {
            //frame = _retainedGrid->frame;
        }
        return run(frame, paintPoint, true);
    }
    
    cv::Mat CBP_FloodFillAlgorithm::runGrid(cv::Ptr<texture_grid> grid, const std::vector<cv::Point2f> &paintPoints, bool isVideo) {

        //CBP_MLUtility::setGridSeeds(grid, paintPoints);
        
        return m_pImpl->_run_grid(grid, isVideo);
    }
    
    cv::Mat CBP_FloodFillAlgorithm::run(cbar::CBAR_VideoFramePtr frame, const cv::Point2f &paintPoint, bool isVideo) {
        m_isCanceled = false;
        std::vector<cv::Point2f> paintPoints = {paintPoint};
        setFrame(frame, paintPoints);
        cv::Mat result = runGrid(_retainedGrid, paintPoints, isVideo);
        imaging::ImageProcessing::rotate_image_90n(result, result, -frame->getFrameRotation());
        return result;
    }

    void CBP_FloodFillAlgorithm::getFFMatches(cv::Ptr<texture_grid> grid,
                                              std::vector<texture_sample*>sampleStack,
                                              std::vector<texture_sample*>&matches) {
        
        CBP_FloodFillIterator iterator;
        
        //auto start = sys_usec_time();
        auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return;
        
        while (sampleStack.size() && !m_isCanceled) {
            texture_sample sample = *sampleStack.back();
            sampleStack.pop_back();
            
            std::vector<texture_sample*>newSamples;
            CBP_FloodFillAlgorithm *floodFill = this;
            
            if (iterator.iterativeAnalysis(grid, sample, newSamples, [floodFill, grid](texture_sample & a, texture_sample & b) {
                return floodFill->isMatch(grid, a, b); })) {
                    sampleStack.insert(sampleStack.begin(), newSamples.begin(), newSamples.end());
                    matches.insert(matches.end(), newSamples.begin(), newSamples.end());
            }
        }
        
        //CBLog("Segmentation took %f seconds.", seconds_elapsed(start));
    }
    
    void CBP_FloodFillAlgorithm::getReplicatedSeedValue(cv::Ptr<texture_grid> grid, std::vector<texture_sample*>&seeds, texture_sample &replicatedSample) {
        
    }

    const cv::Size& CBP_FloodFillAlgorithm::getGridUnitSize() {
        return m_pImpl->m_gridUnitSize;
    }
    
    const cv::Size& CBP_FloodFillAlgorithm::getMaxImageSize() {
        return m_pImpl->m_maxImageSize;
    }
    
    void CBP_FloodFillAlgorithm::cancel() {
        m_isCanceled = true;
    }
};
