//
//  CBP_SemanticAnalyzer.cpp
//  Cambrian
//
//  Created by Joel Teply on 4/2/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#include "CBP_SemanticAnalyzer.hpp"

#include <cbar/pipeline/rendering/rendering.h>
#include <cbcommon/CB_Config.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Coloring.h>
#include <cbar/pipeline/util/util.h>
#include <imaging/Imaging.h>
#include <imaging/FloodFill.hpp>
#include <imaging/Accelerated.h>
#include <opencv2/core/eigen.hpp>

#include <cbar/pipeline/machine-learning/machine-learning.h>

#define FLOOD_FILL 0

using namespace imaging;

namespace cbpipe {
    
    struct CBP_SemanticAnalyzer::Impl
    {
        Impl(CBP_SemanticAnalyzer *parent) : m_parent(parent) {
            m_params.max_rotation_velocity = 0.1f;
            m_params.min_run_distance = 0.3f;
            m_params.max_pitch_radians = M_PI_4;
        }
        
        ~Impl() {}
        
        CBP_SemanticAnalyzer *m_parent;

        cv::Point2f m_point;
        
        CBP_ShadowsEstimator m_shadows;
        CBP_SemanticSegmenter m_segmenter;
        CBP_SurfaceNormalsEstimator m_normals;
        
        area_run_params m_params;
        
        void _initialize() {
            m_segmenter.loadDeepNetwork();
        }
        
        cv::Mat m_lastSegmentationResult;
        
        bool _analyze(cbar::CBAR_VideoFramePtr frame) {
            
            auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return false;
            auto planer = renderer->getAnalyzerOfType<CBP_PlaneAnalyzer>(); if (!planer) return false;
            auto tracker = renderer->getAnalyzerOfType<CBP_FeatureTracker>(); if (!tracker) return false;
            if (!planer->hasGroundPlane()) return false;
            
            m_params.min_run_seconds = planer->secondsStable() > 0.0f ? 1.0f : 3.0f;
            
            //Segmentation
            std::map<std::string, cv::Mat> semanticInput;
            cv::Mat rgb = frame->uprightRGBImage();
            cv::resize(frame->uprightRGBImage(), rgb, cv::Size(256,256));
            
#if DEBUG_SOURCE_IMAGES
            {
                float scale = 1024.0f / fmin(frame->frameSize().width, frame->frameSize().height);
                cv::Mat src;
                cv::resize(frame->uprightRGBImage(), src, cv::Size(), scale, scale);
                Diagnostics::SaveDiagnosticImage(false, src, "source.png");
            }
#endif
            
            cv::Mat segmentedImage;
            bool success = _inference_semantic(rgb, segmentedImage);
            
            if (!success) return false;

            deep_result semanticData;
            semanticData.frame = frame;
            semanticData.outputs.resize(result_index_length);
            semanticData.outputs[result_index_bw] = frame->uprightBWImage();
            
#if DEBUG_DEEP_SEGMENTATION
            cv::Mat debug =  rgb.clone();
            ImageProcessing::overlayMaskOntoRGB(segmentedImage, debug);
            Diagnostics::SaveDiagnosticImage(false, debug, "segmentation");
#endif
            cv::Mat mask = cv::Mat::zeros(segmentedImage.rows, segmentedImage.cols, CV_8UC1);
            mask.setTo(255, segmentedImage > 125);
            cv::erode(mask, mask, cv::getStructuringElement(CV_SHAPE_ELLIPSE, cv::Size(10, 10)));
            cv::dilate(mask, mask, cv::getStructuringElement(CV_SHAPE_ELLIPSE, cv::Size(10, 10)));
            segmentedImage.setTo(0, mask == 0);
            
            //cv::Mat groundMask, wallMask;
            //refineResults(rgb, segmentedImage, groundMask, wallMask);

            semanticData.outputs[result_index_semantic_ground] = segmentedImage;
            //semanticData.outputs[result_index_semantic_ground].setTo(0, groundMask > 0);
            
            semanticData.outputs[result_index_semantic_walls] = 255 - segmentedImage;
            //semanticData.outputs[result_index_semantic_walls].setTo(0, wallMask > 0);
            
            //semanticData.outputs[result_index_semantic_other] = 255 - segmentedImage;
            //semanticData.outputs[result_index_semantic_other].setTo(0, wallMask == 0 & groundMask == 0);
            
            auto stdSize = cv::Size(512, 512);
            for (int i=0; i<semanticData.outputs.size(); i++) {
                if (semanticData.outputs[i].empty()) continue;
                cv::resize(semanticData.outputs[i], semanticData.outputs[i], stdSize);
            }
            
            for (auto surfaceAnalyzer : renderer->getAnalyzersOfType<CBP_SurfaceAnalyzer>()) {
                std::thread([surfaceAnalyzer, semanticData](){
                    surfaceAnalyzer->semanticDataUpdated(semanticData);
                }).detach();
            }
            
            if (auto wallFinder = renderer->getAnalyzerOfType<CBP_WallFinder>()) {
                CBP_WallFinder::semantic_data semantic;
                semantic.frameIndex = frame->frameIndex;
                semantic.rgb = rgb;
                //semantic.normals = normalsResult;
                semantic.semantic = semanticData.outputs[result_index_semantic_walls];
                std::thread([wallFinder, semantic](){
                    wallFinder->semanticDataUpdated(semantic);
                }).detach();
            }
            
            m_lastSegmentationResult = segmentedImage;

            return true;
        }
        
        bool _inference_semantic(const cv::Mat &rgb, cv::Mat &result) {
            std::map<std::string, cv::Mat> semanticInput;
            semanticInput["Placeholder__0"] = rgb;
            semanticInput["Placeholder_1__0"] = m_lastSegmentationResult.empty() ? cv::Mat::zeros(256, 256, CV_8UC1) : m_lastSegmentationResult;
            m_segmenter.inference(rgb, semanticInput, result);
            
            return !result.empty();
        }
        
//        bool inferenceNormals(const cv::Mat &rgb, cv::Mat &result, std::shared_ptr<CBP_FeatureTracker> tracker, cbar::CBAR_VideoFramePtr frame) {
//            std::map<std::string, cv::Mat> pix2PixInput;
//            pix2PixInput["Placeholder__0"] = rgb;
//            m_normals.inference(rgb, pix2PixInput, result);
//
//            if (result.empty()) return false;
//
//            //convert normals to world space
//            Eigen::Matrix4f worldPosition = tracker->getWorldTransform(frame->frameIndex);
//            Eigen::Matrix3f worldRotation = worldPosition.block<3,3>(0,0);
//            //Eigen::Matrix3f cameraRotation = worldRotation.inverse();
//
//            //convert to Y up: (x,y,z) --> (x,z,y)
//            std::vector<cv::Mat>planes;
//            cv::split(result, planes);
//            planes = {planes[0], planes[2], planes[1]};
//            cv::merge(planes, result);
//            result.convertTo(result, CV_32FC3, 2.0f / 255.0, -1.0f);//image is now -1 to 1, Y up
//
//            //per element multiply of 3x3 rotation
//            cv::Mat rotation;
//            eigen2cv(worldRotation, rotation);//convert eigen matrix to CV
//            cv::Mat normalsFlattened = result.reshape(1, result.rows * result.cols);
//            cv::Mat product = normalsFlattened * rotation;
//            result = product.reshape(3, result.rows);
//            //back to 0-255 byte 3 channel image
//            result.convertTo(result, CV_8UC3, 255.0 / 2.0f, 255.0 / 2.0f);
//            //End world space conversion
//
//            return true;
//        }
        
//        void refineResults(const cv::Mat &rgb, const cv::Mat &semantic, cv::Mat &nonFloorMask, cv::Mat &nonWallMask) {
//            nonFloorMask = cv::Mat::zeros(rgb.rows, rgb.cols, CV_8UC1);
//            nonWallMask = cv::Mat::zeros(rgb.rows, rgb.cols, CV_8UC1);
//
//            nonFloorMask.setTo(255, semantic < 5);
//            nonWallMask.setTo(255, semantic > 128);
//
//            Accelerated::roughErode(nonFloorMask, nonFloorMask, cv::Size(30,30));
//            Accelerated::roughErode(nonWallMask, nonWallMask, cv::Size(30,30));
//
//            cv::Size sampleSize = cv::Size(10,10);
//            Eigen::Vector3f up(0,1,0);
//
//#if DEBUG_NORMALS
//            cv::Mat normalsDebug = normals.clone();
//#endif
//            float nonFloorAngleThreshold = M_PI_2 * 0.7;
//            float nonWallAngleThreshold = M_PI_2 * 0.95;
//
//            cv::Rect roi = cv::Rect(0,0, sampleSize.width, sampleSize.height);
//            for (roi.y=0; roi.y<rgb.rows; roi.y+=sampleSize.height) {
//                for (roi.x=0; roi.x<rgb.cols; roi.x+=sampleSize.width) {
//                    cv::Scalar mean, stddev;
//
//                    cv::Rect adjustedROI = roi;
//                    adjustedROI.width = fmin(roi.width, normals.cols - roi.x);
//                    adjustedROI.height = fmin(roi.height, normals.rows - roi.y);
//
//                    cv::meanStdDev(normals(adjustedROI), mean, stddev);
//
//                    auto direction = CBP_AnalysisUtil::colorToDirection(mean);
//                    auto angle = CBP_MatrixUtil::angleBetweenVectors(direction, up);
//
//                    int objectClass = 0;
//
//                    if (angle > nonFloorAngleThreshold) {
//                        objectClass |= 1;
//                        cv::rectangle(nonFloorMask, adjustedROI, cv::Scalar::all(255), CV_FILLED);
//                    }
//
//                    if (angle < nonWallAngleThreshold) {
//                        objectClass |= 2;
//                        cv::rectangle(nonWallMask, adjustedROI, cv::Scalar::all(255), CV_FILLED);
//                    }
//
//#if DEBUG_NORMALS
//                    if (objectClass > 0) {
//                        cv::rectangle(normalsDebug, adjustedROI, cv::Scalar(255 * (objectClass & 1), 255 * (objectClass & 2), 0));
//                    }
//#endif
//
//                }
//
//            }
            
#if DEBUG_NORMALS
            cv::hconcat(rgb, normalsDebug, normalsDebug);
            Diagnostics::SaveDiagnosticImage(false, normalsDebug, "normals");
#endif
            
#if FLOOD_FILL
            std::vector<cv::Point> nzPoints;
            cv::findNonZero(mask, nzPoints);
            
            std::vector<cv::Point> seeds;
            seeds.reserve(nzPoints.size());
            
            float scaleX = float(rgb.cols) / float(semantic.cols);
            float scaleY = float(rgb.rows) / float(semantic.rows);
            for (const auto &point : nzPoints) {
                seeds.push_back(cv::Point(scaleX * float(point.x), scaleY * float(point.y)));
            }
            
            cv::Size unitSize(10,10);
            
            //auto start = sys_usec_time();
            int numFilled = floodFill(rgb, unitSize, mask, seeds, [rgb](const cv::Rect &indexRoi, const cv::Rect &siblingRoi) {
                cv::Scalar meanA, meanB, stddevA, stddevB;
                cv::meanStdDev(rgb(indexRoi), meanA, stddevA);
                cv::meanStdDev(rgb(siblingRoi), meanB, stddevB);

                double totalMean = 0;
                double totalStdDev = 0;
                for (int i=0; i<3; i++) {
                    totalMean += powf(meanA[i] - meanB[i], 2.0);
                    totalStdDev += powf(stddevA[i] - stddevB[i], 2.0);
                }

                double avgMean = sqrt(totalMean);
                double avgStdDev = sqrt(totalStdDev);

                return (avgMean < 5 && avgStdDev <= 15);
            });
#endif
            
            //CBLog("Took %.3f seconds", seconds_elapsed(start));
            
//            cv::Mat markers = cv::Mat::zeros(rgb.rows, rgb.cols, CV_32S);
//
//            cv::Mat floor = 255 - nonfloorMask;
//            cv::distanceTransform(floor, floor, cv::DIST_L2, 3, CV_8U);
//            cv::normalize(floor, floor, 0, 255, cv::NORM_MINMAX);
//            markers.setTo(80, floor > 15);
//
//            cv::Mat wall = 255 - nonWallMask;
//            cv::distanceTransform(wall, wall, cv::DIST_L2, 3, CV_8U);
//            cv::normalize(wall, wall, 0, 255, cv::NORM_MINMAX);
//            markers.setTo(130, wall > 5);
//
//            //edges are sort of off a lot, so let watershed solve them
//            int borderThickness = 20;
//            cv::rectangle(markers, cv::Point(0,0), cv::Point(borderThickness,markers.rows), cv::Scalar(0), CV_FILLED);
//            cv::rectangle(markers, cv::Point(markers.cols - borderThickness, 0), cv::Point(markers.cols,markers.rows), cv::Scalar(0), CV_FILLED);
//
//            Diagnostics::SaveDiagnosticImage(0, markers, "markers");
//            cv::watershed(rgb, markers);
//            markers.convertTo(markers,CV_8U);
//
//            Diagnostics::SaveDiagnosticImage(0, markers, "markers-after");

            //watershed
            //ImageProcessing::refineMask(nonFloorMask, rgb, 20, 20, nonFloorMask);
            //nonWallMask.setTo(0, nonFloorMask > 0);
        //}
    };
    
    CBP_SemanticAnalyzer::CBP_SemanticAnalyzer() : CBP_AreaAnalyzer("CBP_SemanticAnalyzer") {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
        setPriority(CBThreadPriorityModerate);
    }
    
    CBP_SemanticAnalyzer::~CBP_SemanticAnalyzer() {
        
    }
    
    void CBP_SemanticAnalyzer::initialize() {
        m_pImpl->_initialize();
    }
    
    area_run_params CBP_SemanticAnalyzer::getParams() const {
        return m_pImpl->m_params;
    }
    
    bool CBP_SemanticAnalyzer::analyze(cbar::CBAR_VideoFramePtr frame) {
        return m_pImpl->_analyze(frame);
    }
};
