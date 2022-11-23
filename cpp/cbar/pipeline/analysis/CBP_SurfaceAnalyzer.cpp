//
//  CBP_SurfaceAnalyzer.cpp
//  Cambrian
//
//  Created by Joel Teply on 7/12/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#include "CBP_SurfaceAnalyzer.hpp"

#include <cbar/pipeline/rendering/rendering.h>
#include <cbcommon/CB_Config.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Coloring.h>
#include <cbar/pipeline/util/util.h>
#include <algorithm>

#include <cbar/pipeline/machine-learning/machine-learning.h>
#include <cbar/pipeline/analysis/CBP_AnalysisTypes.hpp>
#include <imaging/Accelerated.h>

#define MIN_CONTOUR_AREA_METERS 1.5
#define MIN_PROB_DENSITY 0.5f

#define SHADOWS_DEBUG 0
#define PLOT_CLOUD 0

#define BLUR_RADIUS_METERS 0.05
#define MAX_BLUR_RADIUS 17

#define DEBUG_DUAL_IMAGE 0

#define DEBUG_ALIGNMENT 0

using namespace imaging;

namespace cbpipe {
    
    struct CBP_SurfaceAnalyzer::Impl
    {
        Impl(CBP_SurfaceAnalyzer *parent, cbscene::CBAR_SurfaceAsset *surfaceAsset)
        : m_parent(parent), m_surfaceAsset(surfaceAsset) {
            
            m_params.max_rotation_velocity = 0.3f;
            m_params.min_run_seconds = 2.0f;
            m_params.min_run_distance = 1.0f;
            
            m_accumulators[acc_semantic] = std::shared_ptr<CBP_SurfaceAccumulator>((new CBP_DualRangeAccumulator("semantic", m_nearRange, 100, m_farRange, 50)));
            m_accumulators[acc_semantic]->setAlphaAtIndex(0, 0.3f);
            
            m_accumulators[acc_normals] = std::shared_ptr<CBP_SurfaceAccumulator>((new CBP_SurfaceAccumulator("normals", m_farRange, 30)));
            m_accumulators[acc_shadows] = std::shared_ptr<CBP_SurfaceAccumulator>((new CBP_SurfaceAccumulator("shadows", m_farRange, 30)));
            m_accumulators[acc_horiz_lines] = std::shared_ptr<CBP_SurfaceAccumulator>((new CBP_SurfaceAccumulator("h_lines", m_farRange, 30)));
            m_accumulators[acc_vert_lines] = std::shared_ptr<CBP_SurfaceAccumulator>((new CBP_SurfaceAccumulator("v_lines", m_farRange, 30)));

            
            //m_accumulators[acc_semantic]->setDebugEnabled(1, true);
        }
        
        ~Impl() {

        }
        
        area_run_params m_params;
        std::map<acc_index, std::shared_ptr<CBP_SurfaceAccumulator>> m_accumulators;
        
        const float m_nearRange = 5.0f;
        const float m_farRange = 20.0f;
        
        CBP_SurfaceAnalyzer *m_parent;
        cbscene::CBAR_SurfaceAsset *m_surfaceAsset;
        
        bool m_needsProcessing = false;
        bool m_cleared = false;
        bool m_isHorizontalPlane = false;
        
        int64_t m_lastContoursFrameIndex = -1;
        int64_t m_lastShadowsFrameIndex = -1;
        
        Eigen::Vector3f m_planeNormal, m_planeCenter;
        
        bool m_skipUpdate = false;
        bool m_isReintegrating = false;
        
        int64_t m_startTime = 0;
        int64_t m_lastScreenshot = 0;
        
        std::vector<cv::Point> m_lastContour;
        
        void _initialize() {
            m_startTime = sys_usec_time();
            _set_plane_normal(Eigen::Vector3f(0,1,0));
            _set_plane_center(Eigen::Vector3f(0,0,0));
            m_isHorizontalPlane = m_surfaceAsset->getHitTestType() == HitTestResultTypeHorizontalPlane;
        }
        
        void _clear_all() {
            if (m_lastContoursFrameIndex < 0) return;
            
            for (auto const& kv : m_accumulators) {
                kv.second->clear();
            }
            
            m_needsProcessing = false;
            m_cleared = true;
        }
        
        void _system_now_stable(cbar::CBAR_VideoFramePtr frame, const Eigen::Vector3f &groundCenter) {
            for (auto const& kv : m_accumulators) {
                kv.second->systemNowStable(frame, groundCenter);
            }
            
#if TAKE_SCREENSHOTS
            auto startTime = m_startTime;
            std::thread([startTime](){
                Diagnostics::SaveDiagnosticScreenshot("stable_screenshot_%d", int(seconds_elapsed(startTime)));
            }).detach();
#endif
        }
        
        void _reintegration_starting(cbar::CBAR_VideoFramePtr frame) {
            m_isReintegrating = true;
            m_skipUpdate = true;
        }
        
        void _reintegration_completed(cbar::CBAR_VideoFramePtr frame) {
            m_isReintegrating = false;
        }
        
        bool _analyze(cbar::CBAR_VideoFramePtr frame) {
            
            if (m_isReintegrating) return false;
            
            auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return false;
            auto callback = renderer->getCallback(); if (!callback) return false;
            auto surfaceCallback = m_surfaceAsset->getSurfaceCallback(); if (!callback) return false;
            auto tracker = renderer->getAnalyzerOfType<CBP_FeatureTracker>(); if (!tracker) return false;
            auto planeAnalyzer = renderer->getAnalyzerOfType<CBP_PlaneAnalyzer>(); if (!planeAnalyzer) return false;            
            //auto start = sys_usec_time();
            
            cv::Mat mask3d, bounds, color;
            cv::Rect2f extents3d;
            float ppm;
            
            bool success = _get_probability_mask(renderer, mask3d, bounds, color, extents3d, ppm); if (!success) return false;
            
            //CBLog("getProbabilityMask took %.2f seconds", seconds_elapsed(start)); start = sys_usec_time();
            //Diagnostics::SaveDiagnosticMask(mask3d, color, "mask");
            
            _hallucinate_mask(renderer, frame, mask3d, bounds, extents3d);
            
            //Diagnostics::SaveDiagnosticMask(mask3d, color, "hallucinated");
            
            cbscene::CBAR_SurfaceData surface;
            surface.index =  m_surfaceAsset->getIndex();
            surface.extents3D = extents3d;
            surface.maskExtents3D = extents3d;
            success = _get_contours(renderer, surface, mask3d, color, ppm); if (!success) return false;
            
            //CBLog("getContours took %.2f seconds", seconds_elapsed(start)); start = sys_usec_time();

            _get_shadows(surface);
            
            //CBLog("Sending contours");
            
            planeAnalyzer->getGroundPlane(surface.center, surface.normal);
            
            if (m_isReintegrating) return false;
            
            if (m_skipUpdate) {
                m_skipUpdate = false;
                return false;
            }
            
            std::thread([callback, surfaceCallback, surface](){
                surfaceCallback->surfaceDataUpdated(surface);
            }).detach();
            
#if TAKE_SCREENSHOTS
            if (seconds_elapsed(m_lastScreenshot) > SCREENSHOT_FREQUENCY) {
                m_lastScreenshot = sys_usec_time();
                Diagnostics::SaveDiagnosticScreenshot("screenshot_%d", int(seconds_elapsed(m_startTime)));
            }
#endif
            
            return true;
        }

        int64_t _get_last_updated_index() {
            return m_accumulators[acc_semantic]->getFrameIndex();
        }
        
        bool _get_probability_mask(std::shared_ptr<CBP_RenderingEngine> renderer,
                                cv::Mat &mask3d,
                                cv::Mat &bounds,
                                cv::Mat &colorImage,
                                cv::Rect2f &extents3d,
                                float &ppm) {
            
            int64_t frameIndex = m_accumulators[acc_semantic]->getFrameIndex();
            
            if (frameIndex < 0 || m_lastContoursFrameIndex == frameIndex) return false;
            
            auto tracker = renderer->getAnalyzerOfType<CBP_FeatureTracker>(); if (!tracker) return false;
            auto planeAnalyzer = renderer->getAnalyzerOfType<CBP_PlaneAnalyzer>(); if (!planeAnalyzer) return false;
            
            std::vector<cv::Vec4f> mergedLines;
            ppm = m_accumulators[acc_semantic]->getPixelsPerMeter();
            std::vector<cv::Mat>mergedProbImages;
            cv::Rect imageRoi;
            m_accumulators[acc_semantic]->getSurfaceData(mergedProbImages, mergedLines, bounds, imageRoi);
            if (!mergedProbImages.size()) return false;
            
            extents3d = cv::Rect2f(float(imageRoi.x) / ppm, float(imageRoi.y) / ppm,
                                   float(imageRoi.width) / ppm, float(imageRoi.height) / ppm);
            
            cv::Mat &mergedProb = mergedProbImages[0];
            colorImage = mergedProbImages[1];
            
            //cv::adaptiveThreshold(mergedProb, mergedProb, 255, cv::ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 13, 0);
            
            Eigen::Vector3f center, normal;
            planeAnalyzer->getGroundPlane(center, normal);
            
            m_lastContoursFrameIndex = frameIndex;
            
            mask3d = cv::Mat::zeros(mergedProb.rows, mergedProb.cols, CV_8UC1);
            mask3d.setTo(1, mergedProb > 50);
            
#if DEBUG_DUAL_IMAGE
            Diagnostics::SaveDiagnosticImage(false, mergedProb, "mergedProb.png");
            Diagnostics::SaveDiagnosticMask(mask3d, colorImage, "mergedMask.png");
#endif
            
            cv::Mat maskedAreas;
            Accelerated::roughErode(mask3d, maskedAreas, cv::Size(5,5));
            
            //perform a watershed with the current frame
            cv::Mat nzPoints;
            cv::findNonZero(maskedAreas,nzPoints);
            cv::Rect roi = cv::boundingRect(nzPoints);
            
            if (roi.empty()) return false;
            
            std::vector<cv::Vec4i> hierarchy;
            std::vector<std::vector<cv::Point>> planeContours;

            //add some padding
            int padding = ppm * 1.5;
            cv::Rect expandedROI = cv::Rect(-padding, -padding, roi.width + 2 * padding, roi.height + 2 * padding);
            
            cv::Size extents2d = mask3d.size();
            
            ImageProcessing::expandImageToROI(mask3d(roi), mask3d, expandedROI);
            ImageProcessing::expandImageToROI(colorImage(roi), colorImage, expandedROI);
            ImageProcessing::expandImageToROI(bounds(roi), bounds, expandedROI);
            
            //Diagnostics::SaveDiagnosticImage(false, mergedColor, "color-merged.png");
            roi.width = expandedROI.width;
            roi.height = expandedROI.height;
            roi.x -= padding;
            roi.y -= padding;
            
            cv::Size2f scale = cv::Size2f(float(roi.width) / float(extents2d.width),
                                          float(roi.height) / float(extents2d.height));
            
            extents3d = cv::Rect2f((extents3d.x * ppm + float(roi.x))/ppm,
                                   (extents3d.y * ppm + float(roi.y))/ppm,
                                   scale.width * extents3d.width,
                                   scale.height * extents3d.height);
            
            cv::threshold(bounds, bounds, 254, 255, cv::THRESH_TOZERO);
            
            //Diagnostics::SaveDiagnosticImage(false, bounds, "bounds");
            
            //Diagnostics::SaveDiagnosticMask(mask3d, mergedColor, "overlay");
            
            imaging::ImageProcessing::refineMask(mask3d, colorImage, 15, 15, mask3d, cv::Size(640,640));
            cv::dilate(mask3d, mask3d, cv::getStructuringElement(CV_SHAPE_ELLIPSE,cv::Size(3,3)), cv::Point(-1,-1), 2);
#if DEBUG_DUAL_IMAGE
            Diagnostics::SaveDiagnosticMask(mask3d, colorImage, "overlay-refined");
#endif
            
            return true;
        }
        
        inline cv::Point _point_in_mask(const Eigen::Vector3f &point3d, const cv::Rect2f &extents3d, const cv::Size &maskSize) {
            cv::Point2f pointXY(point3d.x(), point3d.z());
            cv::Point2f pointNormalized((pointXY.x - extents3d.x) / extents3d.width, (pointXY.y - extents3d.y) / extents3d.height);
            
            return cv::Point(pointNormalized.x * float(maskSize.width), pointNormalized.y * float(maskSize.height));
        }
        
        void _hallucinate_mask(std::shared_ptr<CBP_RenderingEngine> renderer,
                            cbar::CBAR_VideoFramePtr frame,
                            cv::Mat &mask3d,
                            const cv::Mat &bounds,
                            const cv::Rect2f &extents3d) {
            //halucinate
            auto tracker = renderer->getAnalyzerOfType<CBP_FeatureTracker>(); if (!tracker) return;
            auto planeAnalyzer = renderer->getAnalyzerOfType<CBP_PlaneAnalyzer>(); if (!planeAnalyzer) return;
            
            Eigen::Vector3f planeCenter, planeNormal;
            planeAnalyzer->getGroundPlane(planeCenter, planeNormal);

            Eigen::Matrix4f worldTransform = tracker->getWorldTransform(frame->frameIndex);
            Eigen::Matrix4f camera = worldTransform.inverse(); //IMPORTANT: two steps required to prevent RELEASE ONLY crash from optimizer
            auto cameraPos = camera.col(3);
            auto cameraPosition = Eigen::Vector3f(cameraPos.x(), cameraPos.y(), cameraPos.z());
            
            Eigen::Vector3f planeBeneathMe = Eigen::Vector3f(cameraPosition.x(), planeCenter.y(), cameraPosition.z());
            
            std::vector<cv::Point> screenPoints = {
                cv::Point(0,0),
                cv::Point(frame->frameSize().width-1,0),
                cv::Point(frame->frameSize().width-1,frame->frameSize().height-1),
                cv::Point(0,frame->frameSize().height-1)
            };
            
            auto screenPoints3dOrigin = renderer->unprojectPointCloud(screenPoints, 0.011, frame->frameIndex);
            auto screenPoints3dForward = renderer->unprojectPointCloud(screenPoints, 0.015, frame->frameIndex);//forward clip
            
            Eigen::Vector3f intersection;
            std::vector<Eigen::Vector3f> intersections;
            for (int i=0; i<screenPoints.size(); i++) {
                bool isValid = Geometry::getPlaneIntersection(planeNormal,
                                                                planeCenter,
                                                                intersection,
                                                                screenPoints3dOrigin[i],
                                                                screenPoints3dForward[i]);
                if (isValid) {
                    intersections.push_back(intersection);
                }
            }
            
            if (intersections.size() < 2) return;
            
            sort(intersections.begin(), intersections.end(),
                 [planeBeneathMe](const Eigen::Vector3f & a, const Eigen::Vector3f & b) -> bool {
                     double distanceToCameraA = (a - planeBeneathMe).norm();
                     double distanceToCameraB = (b - planeBeneathMe).norm();
                return distanceToCameraA < distanceToCameraB;
            });
            
            
            std::vector<cv::Point> planePoints(intersections.size());
            for (int i=0; i<intersections.size(); i++) {
                planePoints[i] = _point_in_mask(intersections[i], extents3d, mask3d.size());
            }
            
            cv::Point2f cameraImagePoint = _point_in_mask(planeBeneathMe, extents3d, mask3d.size());
            
            double distanceToMesh = 1.2 * fmax(Geometry::euclideanDistance(cameraImagePoint, planePoints[0]),
                                       Geometry::euclideanDistance(cameraImagePoint, planePoints[1]));
            

            cv::Mat hallucination;
            Accelerated::roughDilate(mask3d, hallucination, cv::Size(10,10));
            cv::circle(hallucination, cameraImagePoint, distanceToMesh, cv::Scalar::all(255), cv::FILLED);
            
            std::vector<cv::Point> farPoints = _get_far_points(cameraImagePoint, bounds, hallucination);
            
            std::sort(farPoints.begin(), farPoints.end(), [cameraImagePoint](const cv::Point &pointA, const cv::Point &pointB) {
                float angleA = Geometry::angleAtVertex(cameraImagePoint, pointA, pointB);
                float angleB = Geometry::angleAtVertex(cameraImagePoint, pointB, pointA);
                
                return angleA > angleB;
            });
            //can do more complex extending of contours here:
            
            std::vector<cv::Point> farPointsContour;
            for (int i=0; i<farPoints.size(); i++) {
                cv::Point2f pointA = farPoints[i];
                cv::Point2f pointB = farPoints[(i+1) % farPoints.size()];
                
                float radiusA = Geometry::euclideanDistance(pointA, cameraImagePoint);
                float radiusB = Geometry::euclideanDistance(pointB, cameraImagePoint);
                
                float angle = Geometry::angleAtVertex(cameraImagePoint, pointB, pointA);
                
                float inc = 0.05;
                int steps = angle / inc;
                float radiusInc = (radiusB - radiusA) / float(steps);
                float radius = radiusA;
                cv::Point2f unitVector = (pointA - cameraImagePoint) / radius;
                for (float rotation=0; rotation<angle; rotation+=inc, radius += radiusInc) {
                    cv::Point2f unitRotated = Geometry::rotatePoint(unitVector, cv::Point2f(0,0), rotation);
                    unitRotated *= radius * 0.8;
                    farPointsContour.push_back(unitRotated + cameraImagePoint);
                }
            }
            
            if (farPointsContour.size()) {
                std::vector<std::vector<cv::Point>>contours = {farPointsContour};
                cv::drawContours(hallucination, contours, 0, cv::Scalar::all(255), CV_FILLED);
            }
            
#if DEBUG_HALLUCINATION
            cv::Mat debug = cv::Mat::zeros(mask3d.rows, mask3d.cols, CV_8UC1);
            debug.setTo(128, hallucination);
            debug.setTo(40, bounds);
            debug.setTo(255, mask3d);
            cv::cvtColor(debug, debug, CV_GRAY2RGB);
            int i=0;
            for (const auto &point : farPoints) {
                cv::drawMarker(debug, point, cv::Scalar(255,0,0), cv::MARKER_TRIANGLE_UP);
                cv::putText(debug, string_sprintf("%d", i), point, cv::FONT_HERSHEY_PLAIN, 3, cv::Scalar(0,255,0), 2);
                i++;
            }
            
            cv::drawContours(debug, contours, 0, cv::Scalar(0,0,255));
            Diagnostics::SaveDiagnosticImage(false, debug, "hallucination");
#endif
            
            //only copy to unseen areas
            hallucination.copyTo(mask3d, 255-bounds);
        }
        
        std::vector<cv::Point> _get_far_points(const cv::Point &center, const cv::Mat &bounds, const cv::Mat &hallucination) {
            std::vector<cv::Point> points;
            std::vector<cv::Vec4i> hierarchy;
            std::vector<std::vector<cv::Point>> boundsContours;
            cv::findContours(bounds, boundsContours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
            for (int i=0; i<boundsContours.size(); i++) {
                std::vector<cv::Point> &polygon = boundsContours[i];
                
                int closestIndex = _get_closest_point_index(center, polygon);
                int closestIndexA = _get_last_point(hallucination, polygon, closestIndex, 5);
                if (closestIndexA != closestIndex && closestIndexA >= 0) {
                    points.push_back(polygon[closestIndexA]);
                }
                
                std::reverse(polygon.begin(),polygon.end());
                int revStart =  (int) polygon.size() - closestIndex-1;
                int closestIndexB = _get_last_point(hallucination, polygon, revStart, 5);
                if (closestIndexB != revStart && closestIndexB >= 0) {
                    points.push_back(polygon[closestIndexB]);
                }
            }
            
            return points;
        }
        
        int _get_last_point(const cv::Mat &hallucination, const std::vector<cv::Point> &contour, int startIndex, int inc) {
            int index = -1;
            cv::Point point = contour[startIndex];
            bool isOn = hallucination.at<uchar>(point);
            auto numPoints = contour.size();
            for (int i=0; i<numPoints && isOn; i+=inc) {
                index = (startIndex + i) % contour.size();
                point = contour[index];
                isOn = hallucination.at<uchar>(point);
            }
            return index;
        }
        
        int _get_closest_point_index(const cv::Point &center, const std::vector<cv::Point> &contour) {
            int index = 0;
            double closestSq = INT_MAX;
            for (int i=0; i<contour.size(); i++) {
                double dist = Geometry::euclideanDistanceSq(center, contour[i]);
                if (dist < closestSq) {
                    closestSq = dist;
                    index = i;
                }
            }
            return index;
        }
        
        bool _get_contours(std::shared_ptr<CBP_RenderingEngine> renderer, cbscene::CBAR_SurfaceData &surface,
                         const cv::Mat &mask3d, const cv::Mat &colorImage, const float ppm) {
            
            std::vector<cv::Vec4i> hierarchy;
            std::vector<std::vector<cv::Point>> planeContours;
            cv::findContours(mask3d, planeContours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_KCOS);
            
            float minArea = powf(ppm * MIN_CONTOUR_AREA_METERS, 2.0f);
            cv::Mat mask = cv::Mat::zeros(mask3d.rows, mask3d.cols, CV_8UC1);
            float maxContourArea = 0.f;
            int maxContourIndex = 0;
            for (int i=0; i<planeContours.size(); i++) {
                auto size = cv::contourArea(planeContours[i]);
                if (size < minArea) continue;
                cv::drawContours(mask, planeContours, i, cv::Scalar::all(255), cv::FILLED);
                
                if (size > maxContourArea) {
                    maxContourArea = size;
                    maxContourIndex = i;
                }
            }
            
            m_lastContour = planeContours[maxContourIndex];
            //Accelerated::roughErode(mask, mask, cv::Size(15,15));
            
            surface.vertices = {
                Eigen::Vector3f(surface.extents3D.x, 0, surface.extents3D.y),
                Eigen::Vector3f(surface.extents3D.x + surface.extents3D.width, 0, surface.extents3D.y),
                Eigen::Vector3f(surface.extents3D.x + surface.extents3D.width, 0, surface.extents3D.y + surface.extents3D.height),
                Eigen::Vector3f(surface.extents3D.x, 0, surface.extents3D.y + surface.extents3D.height),
            };
            
            if (m_isHorizontalPlane) {
                auto groundAnalyzer = renderer->getAnalyzerOfType<CBP_GroundSurfaceAnalyzer>();
                if (groundAnalyzer) {
                    surface.rotation2D = groundAnalyzer->getPrimaryDirection();
                }
            }
            
            surface.indices = {0,1,3,3,2,1};
            
            cv::Size maskSize = m_surfaceAsset->getMaskSize();
            
            cv::resize(mask, mask, maskSize, 0,0, CV_INTER_LINEAR);
            
            cv::Mat color;
            cv::resize(colorImage, color, maskSize, 0,0, CV_INTER_LINEAR);
            
            //determine blur size
            auto blurSize = cv::Size(9, 9);
            float ppmX = float(maskSize.width) / surface.extents3D.width;
            float ppmY = float(maskSize.height) / surface.extents3D.width;
            
            //make an odd number between 5 and MAX_BLUR_RADIUS
            blurSize.width = 1 + 2 * (int(fmax(fmin(MAX_BLUR_RADIUS, BLUR_RADIUS_METERS * ppmX), 5)) / 2);
            blurSize.height = 1 + 2 * (int(fmax(fmin(MAX_BLUR_RADIUS, BLUR_RADIUS_METERS * ppmY), 5)) / 2);
            cv::blur(mask, mask, blurSize);
            
#if DEBUG_ALIGNMENT
            surface.maskImage = color;
#else
            surface.maskImage = mask;
#endif
            
            return true;
        }
        
        void _get_shadows(cbscene::CBAR_SurfaceData &surface) {

            auto network = m_accumulators[acc_shadows];
            
            network->lock();
            int64_t frameIndex = network->getFrameIndex();
            if (m_lastShadowsFrameIndex == frameIndex || frameIndex < 0) {
                network->unlock();
                return;
            }
            
            float ppm = network->getPixelsPerMeter();
            
            std::vector<cv::Mat>mergedImages;
            std::vector<cv::Vec4f> mergedLines;
            cv::Mat bounds;
            cv::Rect shadowsRoi;
            network->getSurfaceData(mergedImages, mergedLines, bounds, shadowsRoi);

            cv::Mat mergedShadows = mergedImages[0];
            network->unlock();
            
            m_lastShadowsFrameIndex = frameIndex;

            //get mask contours in 3D space, sized to this same ppm
            cv::Rect planeRoi2d = cv::Rect(surface.extents3D.x * ppm,
                                           surface.extents3D.y * ppm,
                                           surface.extents3D.width * ppm,
                                           surface.extents3D.height * ppm);
            //take intersection in 3D space
            cv::Point offset = shadowsRoi.tl();
            shadowsRoi &= planeRoi2d;
            
            //subtract offset for image roi
            shadowsRoi.x -= offset.x;
            shadowsRoi.y -= offset.y;
            
            if (!shadowsRoi.area()) return; //nothing intersects with contours
            
            //take intersection
            mergedShadows = mergedShadows(shadowsRoi);
            bounds = 1 - bounds(shadowsRoi);
            
            //pass world space extents
            cv::Rect2f shadowsRoi3d = cv::Rect2f(fmax(offset.x, planeRoi2d.x) / ppm,
                                                 fmax(offset.y, planeRoi2d.y) / ppm,
                                                 float(shadowsRoi.width) / ppm,
                                                 float(shadowsRoi.height) / ppm);
            
            cv::Size shadowSize = m_surfaceAsset->getShadowsSize();
            cv::resize(mergedShadows, mergedShadows, shadowSize, 0,0, CV_INTER_AREA);
            cv::resize(bounds, bounds, shadowSize, 0,0, CV_INTER_NN);
            
            mergedShadows.setTo(cv::Scalar::all(127), bounds);
            //cv::inpaint(mergedShadows, bounds, mergedShadows, 3, CV_INPAINT_TELEA);
            
            cv::Size blurSize;
            blurSize.width = fmin(fmax(blurSize.width * shadowSize.width / shadowsRoi.width, 3), 13);
            blurSize.height = fmin(fmax(blurSize.height * shadowSize.height / shadowsRoi.height, 3), 13);
            cv::blur(mergedShadows, mergedShadows, blurSize);
            
#if SHADOWS_DEBUG
            Diagnostics::SaveDiagnosticImage(false, mergedShadows, "merged_shadows");
#endif
            surface.shadowsImage = mergedShadows;
            surface.shadowsExtents3D = shadowsRoi3d;
        }
        
        void _set_plane_normal(const Eigen::Vector3f &normal) {
            m_planeNormal = normal;
            
            for (auto const& kv : m_accumulators) {
                kv.second->setPlaneNormal(normal);
            }
        }
        
        void _set_plane_center(const Eigen::Vector3f &center) {
            m_planeCenter = center;
            
            for (auto const& kv : m_accumulators) {
                kv.second->setPlaneCenter(center);
            }
        }
        
        void _semantic_data_updated(const deep_result &result) {
            int semantic_index = m_isHorizontalPlane
                ? CBP_SemanticAnalyzer::result_index_semantic_ground : CBP_SemanticAnalyzer::result_index_semantic_walls;
            
            std::vector<cv::Mat> semanticData = {
                result.outputs[semantic_index],
                result.outputs[CBP_SemanticAnalyzer::result_index_bw]
            };
            
            m_accumulators[acc_semantic]->addSurfaceData(result.frame, semanticData);
        }
        
        void _shadows_data_updated(const deep_result &result) {
            std::vector<cv::Mat>shadowsImages;
            m_accumulators[acc_shadows]->addSurfaceData(result.frame, result.outputs);
        }
        
        void _normals_data_updated(const deep_result &result) {
            m_accumulators[acc_normals]->addSurfaceData(result.frame, result.outputs);
        }
        
        void _lines_found(cbar::CBAR_VideoFramePtr frame, const std::vector<cv::Vec4f> &detectedLines) {
            std::vector<cv::Vec4f> horizLines;
            std::vector<cv::Vec4f> vertLines;
            horizLines.reserve(detectedLines.size());
            vertLines.reserve(detectedLines.size());
            for (const auto &line : detectedLines) {
                
                float deltaX = fabs(line[0] - line[2]);
                float deltaY = fabs(line[1] - line[3]);
                if (deltaX < 0.4f * deltaY) {
                    horizLines.push_back(line);
                } else {
                    vertLines.push_back(line);
                }
            }
            m_accumulators[acc_horiz_lines]->addSurfaceData(frame, {}, &horizLines, frame->getFrameRotation());
            m_accumulators[acc_vert_lines]->addSurfaceData(frame, {}, &vertLines, frame->getFrameRotation());
        }
        
        std::vector<cv::Point> _get_last_contour() const {
            return m_lastContour;
        }
    };
    
    CBP_SurfaceAnalyzer::CBP_SurfaceAnalyzer(cbscene::CBAR_SurfaceAsset *surfaceAsset) : CBP_AreaAnalyzer("CBP_SurfaceAnalyzer") {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, surfaceAsset));
    }
    
    CBP_SurfaceAnalyzer::~CBP_SurfaceAnalyzer() {
        
    }
    
    area_run_params CBP_SurfaceAnalyzer::getParams() const {
        return m_pImpl->m_params;
    }
    
    void CBP_SurfaceAnalyzer::initialize() {
        m_pImpl->_initialize();
    }
    
    void CBP_SurfaceAnalyzer::systemNowStable(cbar::CBAR_VideoFramePtr frame, const Eigen::Vector3f &groundCenter) {
        m_pImpl->_system_now_stable(frame, groundCenter);
    }
    
    bool CBP_SurfaceAnalyzer::analyze(cbar::CBAR_VideoFramePtr frame) {
        return m_pImpl->_analyze(frame);
    }
    
    Eigen::Vector3f CBP_SurfaceAnalyzer::getPlaneNormal() const {
        return m_pImpl->m_planeNormal;
    }
    
    void CBP_SurfaceAnalyzer::setPlaneNormal(const Eigen::Vector3f &normal) {
        m_pImpl->_set_plane_normal(normal);
    }
    
    Eigen::Vector3f CBP_SurfaceAnalyzer::getPlaneCenter() const {
        return m_pImpl->m_planeCenter;
    }
    
    void CBP_SurfaceAnalyzer::setPlaneCenter(const Eigen::Vector3f &center) {
        m_pImpl->_set_plane_center(center);
    }
    
    const std::map<acc_index, std::shared_ptr<CBP_SurfaceAccumulator>> CBP_SurfaceAnalyzer::getAccumulators() const {
        return m_pImpl->m_accumulators;
    }
    
    void CBP_SurfaceAnalyzer::clearAll() {
        m_pImpl->_clear_all();
    }
    
    int64_t CBP_SurfaceAnalyzer::getLastUpdatedIndex() {
        return m_pImpl->_get_last_updated_index();
    }
    
    void CBP_SurfaceAnalyzer::semanticDataUpdated(const deep_result &result) {
         m_pImpl->_semantic_data_updated(result);
    }
    
    void CBP_SurfaceAnalyzer::shadowsDataUpdated(const deep_result &result) {
         m_pImpl->_shadows_data_updated(result);
    }
    void CBP_SurfaceAnalyzer::normalsDataUpdated(const deep_result &result) {
        m_pImpl->_normals_data_updated(result);
    }
    
    void CBP_SurfaceAnalyzer::linesFound(cbar::CBAR_VideoFramePtr frame, const std::vector<cv::Vec4f> &detectedLines) {
        m_pImpl->_lines_found(frame, detectedLines);
    }
    
    void CBP_SurfaceAnalyzer::reintegrationStarting(cbar::CBAR_VideoFramePtr frame) {
        m_pImpl->_reintegration_starting(frame);
    }
    
    void CBP_SurfaceAnalyzer::reintegrationCompleted(cbar::CBAR_VideoFramePtr frame) {
        m_pImpl->_reintegration_completed(frame);
    }
    
    std::vector<cv::Point> CBP_SurfaceAnalyzer::getLastContour() const {
        return m_pImpl->_get_last_contour();
    }
    
    bool CBP_SurfaceAnalyzer::isGroundPlane() const {
        return m_pImpl->m_isHorizontalPlane;
    }
    
    std::shared_ptr<CBP_SurfaceAnalyzer> CBP_SurfaceAnalyzer::getGroundAnalyzer() {
        std::shared_ptr<CBP_SurfaceAnalyzer> groundAnalyzer;
        auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return groundAnalyzer;
        auto surfaceAnalyzers = renderer->getAnalyzersOfType<CBP_SurfaceAnalyzer>(); if (!surfaceAnalyzers.size()) return groundAnalyzer;
        
        groundAnalyzer = *std::find_if(surfaceAnalyzers.begin(), surfaceAnalyzers.end(),
                                                                            [](std::shared_ptr<CBP_SurfaceAnalyzer> const& item) {
                                                                                return item->isGroundPlane();
                                                                            });
        return groundAnalyzer;
    }
};


