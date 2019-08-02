//
//  CBP_DualRangeAccumulator.cpp
//  Cambrian
//
//  Created by Joel Teply on 3/10/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#include "CBP_DualRangeAccumulator.hpp"

#include <cbar/pipeline/rendering/rendering.h>
#include <cbcommon/CB_Config.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Coloring.h>
#include <cbar/pipeline/util/util.h>
#include <algorithm>

#include <cbar/pipeline/machine-learning/machine-learning.h>
#include <cbar/pipeline/analysis/CBP_AnalysisTypes.hpp>
#include <cbar/pipeline/analysis/CBP_LargeImage.hpp>
#include <opencv2/core/eigen.hpp>

//this and setDebugEnabled called on this class
#define DEBUG_ENABLED 0

using namespace imaging;

namespace cbpipe {
    
    struct CBP_DualRangeAccumulator::Impl
    {
        Impl(CBP_DualRangeAccumulator *parent, const std::string &name, float nearRange, float nearPPM, float farRange, float farPPM)
            : m_parent(parent), m_name(name) {
                m_nearRange = std::shared_ptr<CBP_SurfaceAccumulator>(new CBP_SurfaceAccumulator(name + "-near", nearRange, nearPPM));
        }
        
        ~Impl() {
            
        }
        
        CBP_DualRangeAccumulator *m_parent;
        std::string m_name;
        
        std::shared_ptr<CBP_SurfaceAccumulator> m_nearRange;
        
        void combineSurfaceData(std::vector<cv::Mat> &mergedImagesNear, cv::Mat &boundsNear, cv::Rect2f &mergedRoiNear, float ppmNear,
                                std::vector<cv::Mat> &mergedImagesFar, cv::Mat &boundsFar, cv::Rect2f &mergedRoiFar, float ppmFar,
                                std::vector<cv::Mat> &mergedImages, cv::Mat &bounds, cv::Rect2f &extents3d) {
            //combine near and far into one image
            bounds = boundsFar;
            extents3d = cv::Rect2f(float(mergedRoiFar.x)/ppmFar, float(mergedRoiFar.y)/ppmFar,
                                   float(mergedRoiFar.width)/ppmFar, float(mergedRoiFar.height)/ppmFar);
            
            float ppm = ppmNear;
            
            cv::Mat boundsNearModified = boundsNear.clone();
#if DEBUG_ENABLED
            Diagnostics::SaveDiagnosticImage(false, boundsNear, "forebounds-before.png");
#endif
            
            blurredLines(boundsNearModified, 140);
            //cv::distanceTransform(boundsNear, boundsNear, cv::DIST_L1, 10, CV_8U);
            
            //Diagnostics::SaveDiagnosticImage(false, bounds, "boundsFar.png");
            //Diagnostics::SaveDiagnosticImage(false, boundsNear, "boundsNear.png");
            
            cv::Rect2f extents3dNear = cv::Rect2f(float(mergedRoiNear.x)/ppm, float(mergedRoiNear.y)/ppm,
                                                  float(mergedRoiNear.width)/ppm, float(mergedRoiNear.height)/ppm);
            cv::Rect nearROI(ppm * (extents3dNear.x - extents3d.x),
                             ppm * (extents3dNear.y - extents3d.y),
                             boundsNearModified.cols, boundsNearModified.rows);
            
            //rounding issues
            nearROI.x = fmax(0, fmin(nearROI.x, boundsNearModified.cols-1));
            nearROI.y = fmax(0, fmin(nearROI.y, boundsNearModified.rows-1));
            
            mergedImages.resize(mergedImagesFar.size());
            for (int i=0; i<mergedImagesFar.size(); i++) {
                
                float scaleUp = ppm / ppmFar;
                cv::resize(mergedImagesFar[i], mergedImages[i], cv::Size(scaleUp * float(mergedImagesFar[i].cols), scaleUp * float(mergedImagesFar[i].rows)));
                cv::resize(bounds, bounds, mergedImages[i].size());
                
                cv::Rect roiMax = nearROI & cv::Rect(0,0, mergedImages[i].cols, mergedImages[i].rows)
                    & cv::Rect(0,0, mergedImagesNear[i].cols, mergedImagesNear[i].rows);
                cv::Mat result = mergedImages[i](roiMax);
                
                cv::Mat foreground = mergedImagesNear[i](cv::Rect(0,0,roiMax.width, roiMax.height));
                cv::Mat background = mergedImages[i](roiMax);
                
#if DEBUG_ENABLED
                if (m_parent->getDebugEnabled(i)) {
                    Diagnostics::SaveDiagnosticImage(false, foreground, "foreground");
                    Diagnostics::SaveDiagnosticImage(false, background, "background");
                    Diagnostics::SaveDiagnosticImage(false, boundsNearModified, "forebounds");
                    Diagnostics::SaveDiagnosticImage(false, boundsFar, "backbounds");
                }
#endif
                
                mergeImages(foreground, boundsNearModified, background, boundsFar, result);
               
#if DEBUG_ENABLED
                if (m_parent->getDebugEnabled(i)) {
                    Diagnostics::SaveDiagnosticImage(false, result, "result");
                }
#endif
                
                mergedImages[i](roiMax) = result;
            }
        }
        
        void blurredLines(cv::Mat &mask, int thickness) {
            //draw black border on mask
            cv::rectangle(mask, cv::Point(0, 0),
                          cv::Point(mask.cols - 1, mask.rows - 1), cv::Scalar(0), thickness, cv::FILLED);
            
            cv::Rect roiImage = cv::Rect(0, 0, mask.cols, mask.rows);
            //blur it and the mask
            cv::Rect roiTop = roiImage & cv::Rect(cv::Point(0, 0), cv::Point(mask.cols-1, thickness));
            //cv::rectangle(mask, roiTop, cv::Scalar(255));
            cv::blur(mask(roiTop), mask(roiTop), cv::Size(thickness/2, thickness/2));
            
            cv::Rect roiRight = roiImage & cv::Rect(cv::Point(mask.cols-thickness-1, thickness), cv::Point(mask.cols-1, mask.rows-thickness-1));
            //cv::rectangle(mask, roiRight, cv::Scalar(255));
            cv::blur(mask(roiRight), mask(roiRight), cv::Size(thickness/2, thickness/2));
            
            cv::Rect roiBottom = roiImage & cv::Rect(cv::Point(0, mask.rows-thickness-1), cv::Point(mask.cols-1, mask.rows-1));
            //cv::rectangle(mask, roiBottom, cv::Scalar(255));
            cv::blur(mask(roiBottom), mask(roiBottom), cv::Size(thickness/2, thickness/2));
            
            cv::Rect roiLeft = roiImage & cv::Rect(cv::Point(0, thickness), cv::Point(thickness-1, mask.rows-thickness-1));
            //cv::rectangle(mask, roiLeft, cv::Scalar(255));
            cv::blur(mask(roiLeft), mask(roiLeft), cv::Size(thickness/2, thickness/2));
        }
        
        void mergeImages(const cv::Mat& foreground, const cv::Mat& foreground_alpha, const cv::Mat& background, const cv::Mat& background_alpha, cv::Mat& outImage) {
            for (int y = 0; y < background.rows; ++y) {
                for (int x = 0; x < background.cols; ++x) {
                    double opacity_level = double(foreground_alpha.data[y * foreground_alpha.step + x]) / 255.;
                    //double bg_level = double(background_alpha.data[y * background_alpha.step + x]) / 255.;
                    //if (bg_level < 1.0) opacity_level = 0.0f;
                    // now combine the background and foreground pixel, using the opacity, but only if opacity > 0.
                    for (int c = 0; opacity_level > 0 && c < outImage.channels(); ++c) {
                        unsigned char foregroundPx = foreground.data[y * foreground.step + x * foreground.channels() + c];
                        unsigned char backgroundPx = background.data[y * background.step + x * background.channels() + c];
                        outImage.data[y*outImage.step + outImage.channels()*x + c] = backgroundPx * (1.0 - opacity_level) + foregroundPx * opacity_level;
                    }
                }
            }
        }
    };
    
    CBP_DualRangeAccumulator::CBP_DualRangeAccumulator(const std::string &name, float nearRange, float nearPPM, float farRange, float farPPM)
        : CBP_SurfaceAccumulator(name + "-far", farRange, farPPM) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, name, nearRange, nearPPM, farRange, farPPM));
    }
    
    CBP_DualRangeAccumulator::~CBP_DualRangeAccumulator() {
        
    }
    
    const std::string& CBP_DualRangeAccumulator::getName() const {
        return m_pImpl->m_name;
    }
    
    void CBP_DualRangeAccumulator::setDebugEnabled(int index, bool debug) {
        m_pImpl->m_nearRange->setDebugEnabled(index, debug);
        CBP_SurfaceAccumulator::setDebugEnabled(index, debug);
    }
    
    float CBP_DualRangeAccumulator::getPixelsPerMeter() const {
        //return the highest ppm
        return m_pImpl->m_nearRange->getPixelsPerMeter();
    }
    
    void CBP_DualRangeAccumulator::setAlphaAtIndex(int index, float alpha) {
        CBP_SurfaceAccumulator::setAlphaAtIndex(index, alpha);
        m_pImpl->m_nearRange->setAlphaAtIndex(index, alpha);
    }
    
    void CBP_DualRangeAccumulator::setPlaneNormal(const Eigen::Vector3f &normal) {
        CBP_SurfaceAccumulator::setPlaneNormal(normal);
        m_pImpl->m_nearRange->setPlaneNormal(normal);
    }
    
    void CBP_DualRangeAccumulator::setPlaneCenter(const Eigen::Vector3f &center) {
        CBP_SurfaceAccumulator::setPlaneCenter(center);
        m_pImpl->m_nearRange->setPlaneCenter(center);
    }
    
    void CBP_DualRangeAccumulator::addSurfaceData(cbar::CBAR_VideoFramePtr frame, const std::vector<cv::Mat> &data,
                                                  const std::vector<cv::Vec4f> *lines, int surfaceRotation) {
        CBP_SurfaceAccumulator::addSurfaceData(frame, data, lines, surfaceRotation);
        m_pImpl->m_nearRange->addSurfaceData(frame, data, lines, surfaceRotation);
    }
    
    void CBP_DualRangeAccumulator::clear() {
        CBP_SurfaceAccumulator::clear();
        m_pImpl->m_nearRange->clear();
    }
    
    void CBP_DualRangeAccumulator::systemNowStable(cbar::CBAR_VideoFramePtr frame, const Eigen::Vector3f &groundCenter) {
        CBP_SurfaceAccumulator::systemNowStable(frame, groundCenter);
        m_pImpl->m_nearRange->systemNowStable(frame, groundCenter);
    }
    
    void CBP_DualRangeAccumulator::lock() {
        CBP_SurfaceAccumulator::lock();
        m_pImpl->m_nearRange->lock();
    }
    
    void CBP_DualRangeAccumulator::unlock() {
        CBP_SurfaceAccumulator::unlock();
        m_pImpl->m_nearRange->unlock();
    }
    
    void CBP_DualRangeAccumulator::getSurfaceData(std::vector<cv::Mat> &mergedImages, std::vector<cv::Vec4f> &mergedLines,
                                                  cv::Mat &bounds, cv::Rect &extents3d) {
        std::vector<cv::Mat> mergedImagesFar, mergedImagesNear;
        cv::Mat boundsFar, boundsNear;
        cv::Rect extents3dFar, extents3dNear;
        
        CBP_SurfaceAccumulator::lock();
        
        CBP_SurfaceAccumulator::getSurfaceData(mergedImagesFar, mergedLines, boundsFar, extents3dFar);
        m_pImpl->m_nearRange->getSurfaceData(mergedImagesNear, mergedLines, boundsNear, extents3dNear);
        
        CBP_SurfaceAccumulator::unlock();
        
        if (!mergedImagesNear.size() || !mergedImagesFar.size()) return;
        
        cv::Rect2f extents3dFarTemp = extents3dFar, extents3dNearTemp = extents3dNear;
        
        float ppmFar = CBP_SurfaceAccumulator::getPixelsPerMeter();
        float ppmNear = m_pImpl->m_nearRange->getPixelsPerMeter();
        cv::Rect2f extents3dNorm;
        m_pImpl->combineSurfaceData(mergedImagesNear, boundsNear, extents3dNearTemp, ppmNear,
                                    mergedImagesFar, boundsFar, extents3dFarTemp, ppmFar,
                                    mergedImages, bounds, extents3dNorm);
        extents3d = cv::Rect(ppmNear * extents3dNorm.x, ppmNear * extents3dNorm.y,
                             ppmNear * extents3dNorm.width, ppmNear * extents3dNorm.height);
    }
    
    cv::Rect CBP_DualRangeAccumulator::get2DExtents(cv::Rect2f roi3d, cv::Rect roi2d) const {
        float ppm = m_pImpl->m_nearRange->getPixelsPerMeter();
        if (roi2d.empty()) {
            //scale far extents to high res
            roi2d = CBP_SurfaceAccumulator::get2DExtents();
            float ppmFar = CBP_SurfaceAccumulator::getPixelsPerMeter();
            float scale = ppmFar/ppm;
            return cv::Rect(float(roi2d.x) * scale, float(roi2d.y) * scale, float(roi2d.width) * scale, float(roi2d.height) * scale);
        }
        if (!roi3d.empty()) {
            return cv::Rect(roi3d.x * ppm - roi2d.x, roi3d.y * ppm - roi2d.y,
                            roi3d.width * ppm, roi3d.height * ppm);
        }
        return roi2d;
    }
    
    cv::Rect2f CBP_DualRangeAccumulator::get3DExtents(cv::Rect roi) const {
        if (roi.empty()) {
            roi = get2DExtents();
        }
        float ppm = m_pImpl->m_nearRange->getPixelsPerMeter();
        return cv::Rect2f(float(roi.x) / ppm, float(roi.y) / ppm,
                          float(roi.width) / ppm, float(roi.height) / ppm);
    }
};

