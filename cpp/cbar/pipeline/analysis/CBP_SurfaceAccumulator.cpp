//
//  CBP_SurfaceAccumulator.cpp
//  Cambrian
//
//  Created by Joel Teply on 2/1/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#include "CBP_SurfaceAccumulator.hpp"

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

#define MAX_CACHE_SIZE 5

#define DEBUG_PROJECTION 0
#define DEBUG_MERGE 1
#define DEBUG_MERGE_RESULT 0
#define DEBUG_REINTEGRATION 0

using namespace imaging;

namespace cbpipe {
    
    struct CBP_SurfaceAccumulator::Impl
    {
        Impl(CBP_SurfaceAccumulator *parent, const std::string &name, float range, float ppm)
            : m_parent(parent), m_name(name), m_range(range), m_ppm(ppm) {
            m_normal = Eigen::Vector3f(0,1,0);
        }
        
        ~Impl() {
            
        }
        
        CBP_SurfaceAccumulator *m_parent;
        std::string m_name;
        
        Eigen::Vector3f m_normal = Eigen::Vector3f::Zero();
        Eigen::Vector3f m_center = Eigen::Vector3f::Zero();
        
        CBMutex m_mergedMutex;
        std::vector<CBP_LargeImage> m_mergedImages;
        CBP_LargeImage m_mergedBounds;
        std::vector<cv::Vec4f> m_mergedLines;
        
        cv::Mat m_boundsSrc;
        cv::Mat m_boundsSrcBlurred;
        int64_t m_frameIndex = -1;
        
        float m_alphaValues[5] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
        bool m_debug[5] = {false, false, false, false, false};
        
        float m_range = 15.0f;
        float m_ppm = 50.0f;

        class surface {
        public:
            int64_t frameIndex;
            int surfaceRotation;
            Eigen::Vector3f center;
            Eigen::Vector3f normal;
            cv::Size workingSize;
            std::vector<cv::Mat> data;
            std::vector<cv::Vec4f> lines;
        };
        
        std::deque<std::shared_ptr<surface>> m_surfaces;
        bool m_hasReintegrated = false;
        cv::Size m_frameSize;
        int m_frameRotation;
        
        void _system_now_stable(cbar::CBAR_VideoFramePtr frame, const Eigen::Vector3f &groundCenter) {
            m_mergedMutex.lock();
            m_center = groundCenter;
            if (m_hasReintegrated || !m_surfaces.size()) {
                m_hasReintegrated = true;
                m_mergedMutex.unlock();
                return;
            };
            m_hasReintegrated = true;
            //re-integrate surfaces
            auto start = sys_usec_time();
            
#if DEBUG_REINTEGRATION
            logMergedImages("before-rei");
#endif
            _clear_all();
            std::deque<std::shared_ptr<surface>> surfaces = m_surfaces;
            m_surfaces.clear();
            m_mergedMutex.unlock();
            //auto start = sys_usec_time();
            
            for (auto &surf : surfaces) {
                _process_surface(surf);
            }
            
#if DEBUG_REINTEGRATION
            logMergedImages("after-rei");
#endif
            
            CBLog("Reintegrating %lu surfaces in %s took %.2f seconds", surfaces.size(), m_name.c_str(), seconds_elapsed(start));
        }
        
        void _add_surface_data(cbar::CBAR_VideoFramePtr frame, const std::vector<cv::Mat> &data, const std::vector<cv::Vec4f> *lines, int surfaceRotation) {
            
            auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return;
            auto planeAnalyzer = renderer->getAnalyzerOfType<CBP_PlaneAnalyzer>(); if (!planeAnalyzer) return;
            
            planeAnalyzer->getGroundPlane(m_center, m_normal);
            
            if (m_normal.isZero() || (!data.size() && (!lines || !lines->size()))) return;

            m_frameSize = frame->frameSize();
            m_frameRotation = frame->getFrameRotation();
            
            
            std::shared_ptr<surface> surf = std::shared_ptr<surface>(new surface);
            surf->frameIndex = frame->frameIndex;
            surf->surfaceRotation = surfaceRotation;
            surf->data = data;
            surf->workingSize = data.empty() ? frame->frameSize() : data[0].size();
            
            if (lines) {
                //Working size is 1920 by 1440, Working size is 1920 by 1440
                //CBLog("Working size is %d by %d", surface.workingSize.width, surface.workingSize.height);
                surf->lines = *lines;
            }
            
#if MAX_CACHE_SIZE
            if (!m_hasReintegrated) {
                m_surfaces.push_back(surf);
                if (m_surfaces.size() > MAX_CACHE_SIZE) {
                    m_surfaces.pop_front();
                }
            }
#endif
            
            if (_process_surface(surf)) {
                m_frameIndex = frame->frameIndex;
            }
        }
        
        bool _process_surface(std::shared_ptr<surface> surface) {
            cv::Rect roi3d;
            std::vector<cv::Mat> projectedImages(surface->data.size());
            cv::Mat bounds, boundsBlurred;
            
            std::vector<cv::Vec4f> projectedLines;
            
            m_mergedMutex.lock();
            
            surface->normal = m_normal;
            surface->center = m_center;
            _get_projected_images(surface, projectedImages, projectedLines, 1.5 * m_ppm, bounds, boundsBlurred, roi3d);
            
            if (!roi3d.empty() && !bounds.empty() && (!projectedImages.empty() || !projectedLines.empty())) {
                _merge_image_data(projectedImages, projectedLines, bounds, boundsBlurred, roi3d);
            }
            
            m_mergedMutex.unlock();
            
            return true;
        }
        
        void _get_projected_images(std::shared_ptr<surface> surface,
                                std::vector<cv::Mat> &projectedImages,
                                std::vector<cv::Vec4f> &projectedLines,
                                int padding,
                                cv::Mat &bounds,
                                cv::Mat &boundsBlurred,
                                cv::Rect &roi3d) {
            
            auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return;
            auto tracker = renderer->getAnalyzerOfType<CBP_FeatureTracker>(); if (!tracker) return;
            
            Eigen::Matrix4f worldTransform = tracker->getWorldTransform(surface->frameIndex);
            Eigen::Matrix4f camera = worldTransform.inverse(); //IMPORTANT: two steps required to prevent RELEASE ONLY crash from optimizer
            auto cameraPos = camera.col(3);
            auto cameraPosition = Eigen::Vector3f(cameraPos.x(), cameraPos.y(), cameraPos.z());
  
            float range = 0.5f;
            auto point = cv::Point(m_frameSize.width / 2, m_frameSize.height / 2);
            Eigen::Vector3f center3d = Eigen::Vector3f(cameraPosition.x(), surface->center.y(), cameraPosition.z());
            
            Eigen::Vector3f clip3d = renderer->unprojectPoint(point, m_range, surface->frameIndex);
            clip3d.y() = surface->center.y();
            
            double viewingAngle = (renderer->getFOV().width / 2.0f);
            cv::Point2f origin = cv::Point2f(center3d.x(), center3d.z());
            cv::Point2f clip2d = cv::Point2f(clip3d.x(), clip3d.z());
            cv::Point2f leftClip = Geometry::rotatePoint(clip2d, origin, -viewingAngle);
            cv::Point2f rightClip = Geometry::rotatePoint(clip2d, origin, viewingAngle);
            
            std::vector<Eigen::Vector3f> planePoints = {
                Eigen::Vector3f(center3d.x() - range, center3d.y(), center3d.z() - range),
                Eigen::Vector3f(center3d.x() + range, center3d.y(), center3d.z() - range),
                Eigen::Vector3f(center3d.x() + range, center3d.y(), center3d.z() + range),
                Eigen::Vector3f(center3d.x() - range, center3d.y(), center3d.z() + range),
                Eigen::Vector3f(center3d.x(), center3d.y(), center3d.z()),
                clip3d,
                Eigen::Vector3f(leftClip.x, center3d.y(), leftClip.y),
                Eigen::Vector3f(rightClip.x, center3d.y(), rightClip.y),
            };
            
            std::vector<cv::Point> planePoints2dInt = renderer->projectPointCloud(planePoints, surface->frameIndex);
            std::vector<cv::Point2f> boundingRectVideo, boundingRectPlane;
            
            std::vector<cv::Point2f> planePoints2d(planePoints2dInt.size());
            
            for (int i=0; i<planePoints2d.size(); i++) {
                cv::Point2f normalized = planePoints2dInt[i];
                normalized.x /= float(m_frameSize.width);
                normalized.y /= float(m_frameSize.height);
                //normalized = Geometry::rotatePoint(normalized, cv::Point2f(0.5,0.5), -M_PI_2 * frame->getFrameRotation());
                planePoints2d[i] = cv::Point2f(float(surface->workingSize.width) * normalized.x, float(surface->workingSize.height) * normalized.y);
            }
            
            double scale = m_range * m_ppm;
            for (int i=0; i<4; i++) {
                boundingRectVideo.push_back(planePoints2d[i]);
                const auto &point3d = planePoints[i];
                cv::Point2f pointOnPlane2d = cv::Point2f(scale/2.f + m_ppm * (point3d.x() - center3d.x() + range),
                                                         scale/2.f + m_ppm * (point3d.z() - center3d.z() + range));
                boundingRectPlane.push_back(pointOnPlane2d);
            }
            
            cv::Matx33f M = cv::getPerspectiveTransform(boundingRectVideo, boundingRectPlane);
            
            std::vector<cv::Point2f> srcPoints, dstPoints;
            
            srcPoints = {planePoints2d[4], planePoints2d[5], planePoints2d[6], planePoints2d[7]};
            cv::perspectiveTransform(srcPoints,dstPoints,M);
            
            cv::Rect roi = cv::boundingRect(dstPoints);
            roi.x -= padding;
            roi.y -= padding;
            roi.width += 2 * padding;
            roi.height += 2 * padding;
            
            cv::Point2f offset = roi.tl();
            
            for (auto &point : boundingRectPlane) {
                point -= offset;
            }
            
            cv::Point cameraAdjusted = dstPoints[0] - offset;
            
            cv::Point camera2d = cv::Point(cameraPos.x() * m_ppm, cameraPos.z() * m_ppm);
            
            cv::Point offset3d = camera2d - cameraAdjusted;
            
            cv::Matx33f M2 = cv::getPerspectiveTransform(boundingRectVideo, boundingRectPlane);
            
            projectedImages.resize(surface->data.size());
            for (int i=0; i<surface->data.size(); i++) {
                cv::Mat resized;
                //TODO: remove this rotation by performing it in the matrix transform above
                ImageProcessing::rotate_image_90n(surface->data[i], resized, surface->surfaceRotation - m_frameRotation);
                cv::warpPerspective(resized, projectedImages[i], M2, roi.size(), cv::INTER_LINEAR);
            }
            
            if (m_boundsSrc.empty()) {
                m_boundsSrc = surface->surfaceRotation % 2 != 0 ? cv::Mat::zeros(surface->workingSize.width, surface->workingSize.height, CV_8UC1) : cv::Mat::zeros(surface->workingSize.height, surface->workingSize.width, CV_8UC1);
                int rectWidth = 40;
                cv::rectangle(m_boundsSrc, cv::Point(rectWidth, rectWidth),
                              cv::Point(m_boundsSrc.cols - rectWidth - 1, m_boundsSrc.rows - rectWidth - 1), cv::Scalar::all(255), cv::FILLED);
                
                if (projectedImages.size()) {
                    cv::blur(m_boundsSrc, m_boundsSrcBlurred, cv::Size(rectWidth, rectWidth));
                }
                
                cv::rectangle(m_boundsSrc, cv::Point(5, 5),
                              cv::Point(m_boundsSrc.cols - 5 - 1, m_boundsSrc.rows - 5 - 1), cv::Scalar::all(255), cv::FILLED);
                //m_boundsSrc.setTo(cv::Scalar(255));
            }

            cv::warpPerspective(m_boundsSrc, bounds, M2, roi.size(), cv::INTER_NEAREST);
            
            if (!m_boundsSrcBlurred.empty()) {
                cv::warpPerspective(m_boundsSrcBlurred, boundsBlurred, M2, roi.size(), cv::INTER_LINEAR);
                cv::Mat edges;
                //blur the edges of the blurred mask
                cv::resize(m_boundsSrcBlurred, edges, boundsBlurred.size());
                for (int i = 0; i < boundsBlurred.step * boundsBlurred.rows; ++i) {
                    boundsBlurred.data[i] = std::min(boundsBlurred.data[i], edges.data[i]);
                }
            }
            
            roi3d = cv::Rect2f(offset3d.x, offset3d.y, roi.width, roi.height);
            
            if (surface->lines.size()) {
                std::vector<cv::Point2f> linePointsSrc, linePointsDest;
                linePointsSrc.reserve(surface->lines.size() * 2);
                for (const auto &line : surface->lines) {
                    linePointsSrc.push_back(cv::Point2f(line[0], line[1]));
                    linePointsSrc.push_back(cv::Point2f(line[2], line[3]));
                }
                cv::perspectiveTransform(linePointsSrc, linePointsDest, M2);
                
                projectedLines.reserve(surface->lines.size());

                for (int i=0; i<linePointsDest.size(); i+=2) {
                    const auto &pointA = linePointsDest[i];
                    const auto &pointB = linePointsDest[i+1];
                    
                    if (!Geometry::isWithinSize(pointA, bounds.size()) || !Geometry::isWithinSize(pointB, bounds.size())) continue;
                    
                    if (!bounds.at<uchar>(int(pointA.y), int(pointA.x))) continue;
                    if (!bounds.at<uchar>(int(pointB.y), int(pointB.x))) continue;
                    
                    projectedLines.push_back(cv::Vec4f(pointA.x, pointA.y, pointB.x, pointB.y));
                }
            }
            
#if DEBUG_PROJECTION
            bool debug = projectedLines.size() && m_debug[0];
            for (int i=0; i<projectedImages.size(); i++) {
                if (m_debug[i]) {
                    debug = true;
                    Diagnostics::SaveDiagnosticImage(false, projectedImages[i], "%s_projection_%d_(%d).png", m_name.c_str(), i, m_hasReintegrated);
                }
            }
            if (debug) {
                cv::Mat debugBounds = bounds.clone();
                for (const auto &line : projectedLines) {
                    cv::line(debugBounds, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]), cv::Scalar::all(127), 1.0f, CV_AA);
                }
                Diagnostics::SaveDiagnosticImage(false, debugBounds, "%s_mask_%d_(%d).png", m_name.c_str(), m_hasReintegrated);
            }
#endif
            
        }
        
        void _merge_image_data(std::vector<cv::Mat> &images, std::vector<cv::Vec4f> &projectedLines,
                            const cv::Mat &bounds, const cv::Mat &boundsBlurred, const cv::Rect &roi3d) {
            
            if (m_mergedBounds.region().empty()) {
                m_mergedBounds = CBP_LargeImage(bounds, roi3d);
                for (const auto &image : images) {
                    m_mergedImages.push_back(CBP_LargeImage(image, roi3d));
                }
                m_mergedLines = projectedLines;
                return;
            }
            
            auto oldRoi = m_mergedBounds.region();
            auto newRoi = oldRoi | roi3d;
            
            for (int i=0; i<m_mergedImages.size(); i++) {
                cv::Mat result = m_mergedImages[i](roi3d);
                
#if DEBUG_MERGE
                if (m_debug[i]) {
                    //CBLog("Alpha: %f", m_alphaValues[i]);
                    Diagnostics::SaveDiagnosticImage(false, images[i], "foreground_%d", m_hasReintegrated);
                    Diagnostics::SaveDiagnosticImage(false, boundsBlurred, "foreground_alpha_%d", m_hasReintegrated);
                    Diagnostics::SaveDiagnosticImage(false, result, "background_%d", m_hasReintegrated);
                    Diagnostics::SaveDiagnosticImage(false, m_mergedBounds(roi3d), "background_alpha_%d", m_hasReintegrated);
                }
#endif
                
                _alpha_blend(images[i], boundsBlurred, result, m_mergedBounds(roi3d), m_alphaValues[i], result);
                
#if DEBUG_MERGE
                if (m_debug[i]) {
                    Diagnostics::SaveDiagnosticImage(false, result, "result_%d", m_hasReintegrated);
                }
#endif
                m_mergedImages[i](roi3d) = result;
            }
            
            //merge lines
            std::vector<cv::Vec4f> allLines;
            std::vector<cv::Vec4f> mergeLines;
            
            //adjust and trim existing lines
            auto offsetOld = oldRoi.tl() - newRoi.tl();
            for (const auto &line : m_mergedLines) {
                //if line is within these new bounds, drop it.
                cv::Point2f pointA = cv::Point(line[0], line[1]) + offsetOld;
                cv::Point2f pointB = cv::Point(line[2], line[3]) + offsetOld;
                
                bool isWithinNewBounds = false;
                if (Geometry::isWithinSize(pointA, bounds.size()) && Geometry::isWithinSize(pointB, bounds.size())
                    && bounds.at<uchar>(pointA) && bounds.at<uchar>(pointB)) {
                    isWithinNewBounds = true;
                };
                
                auto adjustedLine = cv::Vec4f(pointA.x, pointA.y, pointB.x, pointB.y);
                if (isWithinNewBounds) {
                    mergeLines.push_back(adjustedLine);
                } else {
                    //out of visible area, just adjust to bounds
                    allLines.push_back(adjustedLine);
                }
            }
            
            auto offsetNew = roi3d.tl() - newRoi.tl();
            for (const auto &line : projectedLines) {
                cv::Point2f pointA = cv::Point(line[0], line[1]) + offsetNew;
                cv::Point2f pointB = cv::Point(line[2], line[3]) + offsetNew;
                mergeLines.push_back(cv::Vec4f(pointA.x, pointA.y, pointB.x, pointB.y));
            }
            
            //now merge and add all new lines
            Geometry::groupLines(mergeLines, mergeLines);
            allLines.insert(allLines.end(), mergeLines.begin(), mergeLines.end());
            
            m_mergedLines = allLines;
            
            //save new bounds
            m_mergedBounds(roi3d) += bounds;
            cv::threshold(m_mergedBounds(roi3d), m_mergedBounds(roi3d), 255, 0, cv::THRESH_TRUNC);
            
#if DEBUG_MERGE_RESULT
            logMergedImages();
#endif
        }
        
        void _log_merged_images(std::string suffix="") {
            bool debug = m_mergedLines.size() && m_debug[0];
            for (int i=0; i<m_mergedImages.size(); i++) {
                if (m_debug[i]) {
                    debug = true;
                    cv::Mat debugImage = m_mergedImages[i].get();
                    _draw_lines(debugImage, cv::Scalar(255, 255, 0));
                    Diagnostics::SaveDiagnosticImage(false, debugImage, "%s_surface_%d_(%d)%s.png", m_name.c_str(), i, m_hasReintegrated, suffix.c_str());
                }
            }
            if (debug) {
                cv::Mat debugBounds = m_mergedBounds.get().clone();
                _draw_lines(debugBounds, cv::Scalar::all(127));
                Diagnostics::SaveDiagnosticImage(false, debugBounds, "%s_bounds_(%d)%s.png", m_name.c_str(), m_hasReintegrated, suffix.c_str());
            }
        }
        
        void _draw_lines(cv::Mat &image, cv::Scalar color) {
            for (const auto &line : m_mergedLines) {
                cv::line(image, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]), color, 1.0f, CV_AA);
            }
        }
        
        void _alpha_blend(const cv::Mat& foreground, const cv::Mat& foreground_alpha,
                        const cv::Mat& background, const cv::Mat& background_alpha, float opacity, cv::Mat& outImage) {
            for (int y = 0; y < background.rows; ++y) {
                for (int x = 0; x < background.cols; ++x) {
                    // determine the opacity of the foregrond pixel, using its fourth (alpha) channel.
                    double fg_alpha = double(foreground_alpha.data[y * foreground_alpha.step + x]) / 255.;
                    int bg_alpha = background_alpha.data[y * background_alpha.step + x];
                    double opacity_level = bg_alpha < 255 ? 1.0f : fg_alpha * opacity;

                    // and now combine the background and foreground pixel, using the opacity, but only if opacity > 0.
                    for (int c = 0; opacity_level > 0 && c < outImage.channels(); ++c) {
                        unsigned char foregroundPx = foreground.data[y * foreground.step + x * foreground.channels() + c];
                        unsigned char backgroundPx = background.data[y * background.step + x * background.channels() + c];
                        outImage.data[y*outImage.step + outImage.channels()*x + c] = backgroundPx * (1.0 - opacity_level) + foregroundPx * opacity_level;
                    }
                }
            }
        }
        
        void _get_surface_data(std::vector<cv::Mat> &mergedImages, std::vector<cv::Vec4f> &mergedLines, cv::Mat &bounds, cv::Rect &extents3d) {
            mergedImages.resize(m_mergedImages.size());
            for (int i=0; i<m_mergedImages.size(); i++) {
                mergedImages[i] = m_mergedImages[i].get();
            }
            bounds = m_mergedBounds.get();
            extents3d = m_mergedBounds.region();
            mergedLines.insert(mergedLines.end(), m_mergedLines.begin(), m_mergedLines.end());
        }
        
        void _set_surface_data(int index, cv::Mat data, cv::Mat bounds, cv::Rect roi) {
            if (roi.empty()) {
                roi = m_mergedBounds.region();
            }
            if (!bounds.empty()) {
                m_mergedBounds(roi) |= bounds;
            }
            m_mergedImages[index](roi) = data;
        }
        
        void _clear_all() {
            m_mergedImages.clear();
            m_mergedBounds = CBP_LargeImage();
            m_mergedLines.clear();
            cv::Mat m_boundsSrc = cv::Mat();
            cv::Mat m_boundsSrcBlurred = cv::Mat();
        }
    };
    
    CBP_SurfaceAccumulator::CBP_SurfaceAccumulator(const std::string &name, float range, float ppm) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, name, range, ppm));
    }
    
    CBP_SurfaceAccumulator::~CBP_SurfaceAccumulator() {
        
    }
    
    const std::string& CBP_SurfaceAccumulator::getName() const {
        return m_pImpl->m_name;
    }
    
    bool CBP_SurfaceAccumulator::getDebugEnabled(int index) const {
        return m_pImpl->m_debug[index];
    }
    
    void CBP_SurfaceAccumulator::setDebugEnabled(int index, bool debug) {
        m_pImpl->m_debug[index] = debug;
    }
    
    float CBP_SurfaceAccumulator::getRangeMeters() const {
         return m_pImpl->m_range;
    }
    
    float CBP_SurfaceAccumulator::getPixelsPerMeter() const {
        return m_pImpl->m_ppm;
    }
    
    float CBP_SurfaceAccumulator::getAlphaAtIndex(int index) const {
        return m_pImpl->m_alphaValues[index];
    }
    
    void CBP_SurfaceAccumulator::setAlphaAtIndex(int index, float alpha) {
        m_pImpl->m_alphaValues[index] = alpha;
    }
    
    Eigen::Vector3f CBP_SurfaceAccumulator::getPlaneNormal() const {
        return m_pImpl->m_normal;
    }
    
    void CBP_SurfaceAccumulator::setPlaneNormal(const Eigen::Vector3f &normal) {
        m_pImpl->m_normal = normal;
    }
    
    Eigen::Vector3f CBP_SurfaceAccumulator::getPlaneCenter() const {
        return m_pImpl->m_center;
    }
    
    void CBP_SurfaceAccumulator::setPlaneCenter(const Eigen::Vector3f &center) {
        m_pImpl->m_center = center;
    }
    
    void CBP_SurfaceAccumulator::addSurfaceData(cbar::CBAR_VideoFramePtr frame, const std::vector<cv::Mat> &data,
                                                const std::vector<cv::Vec4f> *lines, int surfaceRotation) {
        m_pImpl->_add_surface_data(frame, data, lines, surfaceRotation);
    }
    
    void CBP_SurfaceAccumulator::clear() {
        m_pImpl->m_mergedMutex.lock();
        m_pImpl->_clear_all();
        m_pImpl->m_mergedMutex.unlock();
    }
    
    int64_t CBP_SurfaceAccumulator::getFrameIndex() const {
        return m_pImpl->m_frameIndex;
    }
    
    void CBP_SurfaceAccumulator::systemNowStable(cbar::CBAR_VideoFramePtr frame, const Eigen::Vector3f &groundCenter) {
        m_pImpl->_system_now_stable(frame, groundCenter);
    }
    
    void CBP_SurfaceAccumulator::getSurfaceData(std::vector<cv::Mat> &mergedImages, std::vector<cv::Vec4f> &mergedLines, cv::Mat &bounds, cv::Rect &extents3d) {
        return m_pImpl->_get_surface_data(mergedImages, mergedLines, bounds, extents3d);
    }
    
    void CBP_SurfaceAccumulator::setSurfaceData(int index, cv::Mat data, cv::Mat bounds, cv::Rect roi) {
        m_pImpl->_set_surface_data(index, data, bounds, roi);
    }
    
    std::vector<CBP_LargeImage>& CBP_SurfaceAccumulator::mergedImages() {
        return m_pImpl->m_mergedImages;
    }
    
    CBP_LargeImage& CBP_SurfaceAccumulator::mergedBounds() {
        return m_pImpl->m_mergedBounds;
    }
    
    cv::Rect CBP_SurfaceAccumulator::get2DExtents(cv::Rect2f roi3d, cv::Rect roi2d) const {
        if (roi2d.empty()) {
            roi2d = m_pImpl->m_mergedBounds.region();
        }
        if (!roi3d.empty()) {
            return cv::Rect(roi3d.x * m_pImpl->m_ppm - roi2d.x, roi3d.y * m_pImpl->m_ppm - roi2d.y,
                            roi3d.width * m_pImpl->m_ppm, roi3d.height * m_pImpl->m_ppm);
        }
        return roi2d;
    }
    
    cv::Rect2f CBP_SurfaceAccumulator::get3DExtents(cv::Rect roi2d) const {
        if (roi2d.empty()) {
            roi2d = get2DExtents();
        }
        return cv::Rect2f(float(roi2d.x) / m_pImpl->m_ppm, float(roi2d.y) / m_pImpl->m_ppm,
                          float(roi2d.width) / m_pImpl->m_ppm, float(roi2d.height) / m_pImpl->m_ppm);
    }
    
    void CBP_SurfaceAccumulator::lock() {
        m_pImpl->m_mergedMutex.lock();
    }
    
    void CBP_SurfaceAccumulator::unlock() {
        m_pImpl->m_mergedMutex.unlock();
    }
};

