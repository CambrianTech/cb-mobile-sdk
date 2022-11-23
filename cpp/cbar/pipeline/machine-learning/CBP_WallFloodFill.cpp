//
//  CBP_WallFloodFill.cpp
//  Cambrian
//
//  Created by Joel Teply on 4/7/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#include "CBP_WallFloodFill.hpp"

#include <opencv2/ml/ml.hpp>

#include <fstream>

#include <utility/CommonUtility.h>
#include <utility/Directory.h>
#include <utility/Diagnostics.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Imaging.h>
#include <imaging/LineProcessing.h>

#include "CBP_MLUtility.hpp"

using namespace imaging;
using namespace cbar;

namespace cbpipe {
    
    static CBMutex lsdMutex;
    
    struct CBP_WallFloodFill::Impl
    {
        Impl(CBP_WallFloodFill *floodFill, double constantMean, double constantStdDev)
            : m_floodFill(floodFill), m_constantMean(constantMean), m_constantStdDev(constantStdDev) {
                
            }
       ~Impl() {}
        
        CBP_WallFloodFill *m_floodFill;
        
        double m_constantMean = 10;
        double m_constantStdDev = 15.0f;
        cv::Mat m_floodImage;

        void _prepare(cv::Ptr<texture_grid> grid, bool isVideo) {
            
            CBLogTiming("1) prepare start at frame %d", CBAR_VideoFrame::lastFrameIndex());
            
            double scale = fmax(300.0f / double(grid->srcImage.cols),
                                300.0f / double(grid->srcImage.rows));
            
            cv::Mat reducedRGB;
            cv::resize(grid->srcImage, reducedRGB, cv::Size(scale * double(grid->srcImage.cols), scale * double(grid->srcImage.rows)));

            cv::Mat reducedBW;
            cv::extractChannel(reducedRGB, reducedBW, 1);
            
            if (m_floodFill->m_isCanceled) return;
            
            cv::Mat edges, edgesRefined;
            ImageProcessing::freiChen(reducedBW, edges, 5.0);
            
            if (m_floodFill->m_isCanceled) return;
            
            //cv::bilateralFilter(edges, edgesRefined, 5, 30, 80);
            cv::medianBlur(edges, edgesRefined, 7);
            
            CBLogTiming("2) prepare bilateralFilter at frame %d", CBAR_VideoFrame::lastFrameIndex());
            
            if (m_floodFill->m_isCanceled) return;
            
            reducedBW = reducedBW + edgesRefined;
            cv::insertChannel(reducedBW, reducedRGB, 1);
            
            if (m_floodFill->m_isCanceled) return;
            
            _draw_lines(reducedBW, reducedRGB);
            
            cv::resize(reducedRGB, m_floodImage, grid->srcImage.size());
            
            if (m_floodFill->m_isCanceled) return;
            
            CBLogTiming("3) prepare complete at frame %d", CBAR_VideoFrame::lastFrameIndex());
            //Diagnostics::SaveDiagnosticImage(true, m_floodImage, "floodFill.jpg");
        }
        
        void _draw_lines(const cv::Mat &bwImage, cv::Mat &rgbImageDest) {
            //detect lines
            {
                double lScale = 1.0;
                double sigma_scale = 1.0;
                double quant = 2.0;
                double ang_th = 30.5;
                double log_eps = 0;
                double density_th = 0.3;
                int n_bins = 1024;
                std::vector<cv::Vec4f> detectedLines;
                
                {std::lock_guard<CBMutex> lockGuard(lsdMutex);
                    auto lsd = cv::createLineSegmentDetector(cv::LSD_REFINE_STD, lScale, sigma_scale, quant, ang_th, log_eps, density_th, n_bins);
                    if(!lsd.empty()) {
                        lsd->detect(bwImage, detectedLines);
                        lsd.release();
                    }
                }
                
                std::sort(detectedLines.begin(), detectedLines.end(), [](const cv::Vec4f &lineA, const cv::Vec4f &lineB) {
                    double distanceA = Geometry::euclideanDistanceSq(cv::Point2f(lineA[0], lineA[1]), cv::Point2f(lineA[2], lineA[3]));
                    double distanceB = Geometry::euclideanDistanceSq(cv::Point2f(lineB[0], lineB[1]), cv::Point2f(lineB[2], lineB[3]));
                    
                    return distanceA > distanceB;
                });
                
                int minLength = fmin(bwImage.cols, bwImage.rows) / 10;
                int minLengthSQ = minLength * minLength;
                
                auto numLines = detectedLines.size();
                
                for (int i=0; i<numLines; i++) {
                    const auto& line = detectedLines[i];
                    
                    double length = Geometry::euclideanDistanceSq(cv::Point2f(line[0], line[1]), cv::Point2f(line[2], line[3]));
                    
                    if (length < minLengthSQ) break;
                    
                    cv::Point2f pointA = cv::Point(line[0], line[1]);
                    cv::Point2f pointB = cv::Point(line[2], line[3]);
                    Geometry::scaleLineSegment(pointA, pointB, 1.2);
                    
                    cv::line(rgbImageDest, pointA, pointB, cv::Scalar(0,255,0));
                }
            }
        }
        
        inline void _ensure_data(texture_sample &element, const cv::Mat &rgbImage) {
            if (element.data.empty()) {
                cv::Scalar mean, stddev;
                element.data.resize(6);
                cv::meanStdDev(rgbImage(element.roi), mean, stddev);
                //mean = cv::mean(rgbImage(element.roi));
                for (int i=0; i<3; i++) {
                    element.data[i] = mean[i];
                    element.data[i+3] = stddev[i];
                }
            }
        }
        
        bool _is_match(cv::Ptr<texture_grid> grid, texture_sample &elementA, texture_sample &elementB) {
            
            _ensure_data(elementA, m_floodImage);
            _ensure_data(elementB, m_floodImage);
            
            double totalMean = 0;
            double totalStdDev = 0;
            for (int i=0; i<3; i++) {
                totalMean += powf(elementA.data[i] - elementB.data[i], 2.0);
                totalStdDev += powf(elementA.data[i+3] - elementB.data[i+3], 2.0);
            }
            
            double avgMean = sqrt(totalMean);
            double avgStdDev = sqrt(totalStdDev);
            
            double meanThreshold =  m_constantMean * elementA.size.width;
            double stddevThreshold = m_constantStdDev * (1 + 10.0f / elementA.size.width);
            
            return avgMean < meanThreshold && avgStdDev <= stddevThreshold;
        }
        
        
    };
    
    CBP_WallFloodFill::CBP_WallFloodFill(const cv::Size &gridSize, double constantMean, double constantStdDev)
        : CBP_FloodFillAlgorithm(PaintPointType_Surface, gridSize, cv::Size(1280,1280)) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this,  constantMean, constantStdDev));
    }
    
    CBP_WallFloodFill::~CBP_WallFloodFill() {
        
    }
    
    void CBP_WallFloodFill::getReplicatedSeedValue(cv::Ptr<texture_grid> grid,
                                                     std::vector<texture_sample*>&seeds, texture_sample &replicatedSample) {
        
    }
    
    void CBP_WallFloodFill::prepare(cv::Ptr<texture_grid> grid, bool isVideo) {
        
        m_pImpl->_prepare(grid, isVideo);
    }
    
    cv::Mat CBP_WallFloodFill::runGrid(cv::Ptr<texture_grid> grid, const std::vector<cv::Point2f> &paintPoints, bool isVideo) {
        CBLogTiming("1) runGrid start at frame %d", CBAR_VideoFrame::lastFrameIndex());
        cv::Mat result = CBP_FloodFillAlgorithm::runGrid(grid, paintPoints, false);
        CBLogTiming("2) runGrid complete at frame %d", CBAR_VideoFrame::lastFrameIndex());
        return result;
    }
    
    bool CBP_WallFloodFill::isMatch(cv::Ptr<texture_grid> grid, texture_sample &elementA, texture_sample &elementB) {
        return m_pImpl->_is_match(grid, elementA, elementB);
    }
};
