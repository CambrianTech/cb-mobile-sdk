//
//  CBP_SurfaceContourAdjuster.cpp
//  Cambrian
//
//  Created by Joel Teply on 3/10/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#include "CBP_GroundSurfaceAnalyzer.hpp"

#include <cbar/pipeline/rendering/rendering.h>
#include <cbcommon/CB_Config.h>
#include <imaging/ImageProcessing.h>
#include <cbar/pipeline/util/util.h>
#include <imaging/Imaging.h>

#include <cbar/pipeline/machine-learning/machine-learning.h>
#include <cbar/pipeline/analysis/CBP_PlaneRansac.hpp>

#define COMBINE_SURFACES 1
#define MAX_VERT_LENGTH 150

using namespace imaging;

namespace cbpipe {

    struct CBP_GroundSurfaceAnalyzer::Impl
    {
        Impl(CBP_GroundSurfaceAnalyzer *parent) : m_parent(parent) {
            
        }
        
        ~Impl() {}
        
        CBP_GroundSurfaceAnalyzer *m_parent;
        int64_t m_lastUpdateFrame = -1;
        
        Eigen::Vector2f m_direction = Eigen::Vector2f::Zero();
        std::vector<float> m_directionHistory;
        std::vector<cv::Point2f> m_basisLines;
        Eigen::Vector3f m_normal = Eigen::Vector3f(0,1,0);
        
        Eigen::Vector2f getPrimaryDirection() {
            if (m_direction.isZero()) {
                auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return m_direction;
                auto heading = renderer->getLastHeading();
                return Eigen::Vector2f(heading.x(), heading.z());
            }
            return m_direction;
        }
        
        bool analyze(cbar::CBAR_VideoFramePtr frame) {
            
            auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return false;
            auto surfaceAnalyzer = renderer->getAnalyzerOfType<CBP_SurfaceAnalyzer>(); if (!surfaceAnalyzer) return false;
            auto tracker = renderer->getAnalyzerOfType<CBP_FeatureTracker>(); if (!tracker) return false;
            auto planer = renderer->getAnalyzerOfType<CBP_PlaneAnalyzer>(); if (!planer) return false;
            if (!planer->hasGroundPlane()) return false;
            
            auto frameIndex = surfaceAnalyzer->getLastUpdatedIndex();
            if (m_lastUpdateFrame == frameIndex) return false;
            
            auto accumulators = surfaceAnalyzer->getAccumulators();
            
            auto normalsFrameIndex = accumulators[acc_normals]->getFrameIndex();
            auto semanticFrameIndex = accumulators[acc_semantic]->getFrameIndex();
            auto linesFrameIndex = accumulators[acc_horiz_lines]->getFrameIndex();
            
            if (normalsFrameIndex < 0 || semanticFrameIndex < 0 || linesFrameIndex < 0) return false;
            
            std::vector<cv::Mat>mergedNormals;
            std::vector<cv::Vec4f>empty;
            cv::Rect normalsRoi;
            cv::Mat normalsBounds;
            accumulators[acc_normals]->getSurfaceData(mergedNormals, empty, normalsBounds, normalsRoi);
            cv::Rect2f normalsExtents = accumulators[acc_normals]->get3DExtents(normalsRoi);
            if (!mergedNormals.size()) return false;
            cv::Mat normals = mergedNormals[0];
            
            std::vector<cv::Mat>mergedSemantic;
            cv::Rect semanticRoi;
            cv::Mat semanticBounds;
            accumulators[acc_semantic]->getSurfaceData(mergedSemantic, empty, semanticBounds, semanticRoi);
            if (!mergedSemantic.size()) return false;
            
            cv::Rect2f semanticExtents = accumulators[acc_semantic]->get3DExtents(semanticRoi);
            cv::Mat mergedProb = mergedSemantic[0];
            cv::Mat mergedBW = mergedSemantic[1];
            
            std::vector<cv::Mat>linesImages;
            cv::Rect linesRoi;
            cv::Mat linesBounds;
            std::vector<cv::Vec4f>h_lines, v_lines;
            
            float lines_ppm = accumulators[acc_horiz_lines]->getPixelsPerMeter();
            accumulators[acc_horiz_lines]->getSurfaceData(linesImages, h_lines, linesBounds, linesRoi);
            accumulators[acc_vert_lines]->getSurfaceData(linesImages, v_lines, linesBounds, linesRoi);
            
            if (h_lines.size() < 5) return false;
            
            std::vector<cv::Vec4f>h_lines3d;
            h_lines3d.reserve(h_lines.size());
            for (const auto &line : h_lines) {
                h_lines3d.push_back(cv::Vec4f((line[0] + linesRoi.x) / lines_ppm,
                                            (line[1] + linesRoi.y) / lines_ppm,
                                            (line[2] + linesRoi.x) / lines_ppm,
                                            (line[3] + linesRoi.y) / lines_ppm));
            }
            
            std::vector<cv::Vec4f>v_lines3d;
            v_lines3d.reserve(v_lines.size());
            for (const auto &line : v_lines) {
                v_lines3d.push_back(cv::Vec4f((line[0] + linesRoi.x) / lines_ppm,
                                              (line[1] + linesRoi.y) / lines_ppm,
                                              (line[2] + linesRoi.x) / lines_ppm,
                                              (line[3] + linesRoi.y) / lines_ppm));
            }
            
            cv::Rect2f linesExtents = accumulators[acc_horiz_lines]->get3DExtents(linesRoi);
            
            cv::Rect2f combinedExtents = normalsExtents & semanticExtents & linesExtents;
            
            if (!combinedExtents.area()) return false;
            
            semanticRoi = accumulators[acc_semantic]->get2DExtents(combinedExtents, semanticRoi);
            normalsRoi = accumulators[acc_normals]->get2DExtents(combinedExtents, normalsRoi);
            linesRoi = accumulators[acc_horiz_lines]->get2DExtents(combinedExtents, linesRoi);

            mergedProb = mergedProb(semanticRoi);
            mergedBW = mergedBW(semanticRoi);
            normals = normals(normalsRoi);
            
            cv::resize(normals, normals, mergedProb.size());
            cv::resize(mergedBW, mergedBW, mergedProb.size());
            
            
            cv::Mat debug;
#if DEBUG_SURFACES
            debug = mergedBW.clone();
            
            if (debug.channels() == 1) cv::cvtColor(debug, debug, CV_GRAY2RGB);
            debug = Imaging::blend_multiply(normals, debug);
#endif
            auto workingSize = mergedBW.size();
            std::vector<cv::Vec4f> h_linesScaled, v_linesScaled;
            float semantic_ppm = accumulators[acc_semantic]->getPixelsPerMeter();
            cv::Point2f linesOffset = combinedExtents.tl() * -semantic_ppm;
            
            for (const auto &line : h_lines3d) {
                auto pointA = cv::Point2f(line[0] * semantic_ppm, line[1] * semantic_ppm) + linesOffset;
                auto pointB = cv::Point2f(line[2] * semantic_ppm, line[3] * semantic_ppm) + linesOffset;
                h_linesScaled.push_back(cv::Vec4f(pointA.x, pointA.y, pointB.x, pointB.y));
#if DEBUG_SURFACES
                cv::line(debug, pointA, pointB, cv::Scalar(0,255,255), 1, cv::LINE_AA);
#endif
            }
            
            float maxVLengthSq = MAX_VERT_LENGTH * MAX_VERT_LENGTH;
            for (const auto &line : v_lines3d) {
                const auto pointA = cv::Point2f(line[0] * semantic_ppm, line[1] * semantic_ppm) + linesOffset;
                const auto pointB = cv::Point2f(line[2] * semantic_ppm, line[3] * semantic_ppm) + linesOffset;
                
                if (Geometry::euclideanDistanceSq(pointA, pointB) > maxVLengthSq) continue;

                v_linesScaled.push_back(cv::Vec4f(pointA.x, pointA.y, pointB.x, pointB.y));
#if DEBUG_SURFACES
                cv::line(debug, pointA, pointB, cv::Scalar(255,255,0), 1, cv::LINE_AA);
#endif
            }
            
//            Eigen::Matrix4f worldTransform = tracker->getWorldTransform(frame->frameIndex);
//            Eigen::Matrix4f camera = worldTransform.inverse(); //IMPORTANT: two steps required to prevent RELEASE ONLY crash from optimizer
//            auto cameraPos = camera.col(3);
//            auto cameraPosition = Eigen::Vector3f(cameraPos.x(), cameraPos.y(), cameraPos.z());
//            Eigen::Vector3f planeBeneathMe = Eigen::Vector3f(cameraPosition.x(), 0, cameraPosition.z());
//            cv::Point2f cameraImagePoint = pointInMask(planeBeneathMe, semanticRoi, mergedBW.size());
            
            std::vector<cv::Mat> surfaces;
            std::vector<int> counts;
            std::vector<cv::Point2f> directions;
            std::vector<cv::Scalar> colors;
            bool success = CBP_AnalysisUtil::kmeansNormals(normals, cv::Size(200,200), surfaces, counts, directions, colors, &debug);
            
            if (!success) return false;
            
            int maxDirectionIndex = (int) std::distance(counts.begin(), std::max_element(counts.begin(), counts.end()));
            cv::Point2f maxDirection = directions[maxDirectionIndex];
            
            auto verticalPlanes = planer->getPlanes(PlaneAnchorAlignmentVertical);
            
            std::vector<cv::Vec4f> allLines = h_linesScaled;
            allLines.insert(allLines.end(), v_linesScaled.begin(), v_linesScaled.end());
            
            auto heading = tracker->getHeading(normalsFrameIndex);
            
            success = findDirection(allLines, maxDirection, heading, surfaceAnalyzer->getLastContour());
            if (!success) return false;
            
            m_lastUpdateFrame = frameIndex;

//            //Now match surfaces and directions to lines
            auto ransacStart = sys_usec_time();
            CBP_PlaneRansac ransac(workingSize, h_linesScaled, surfaces, directions, colors);
            
            std::vector<cv::Vec4f> baselines;
            ransac.solve(100, debug, baselines);
            //CBLog("CBP_PlaneRansac took %.2f seconds and found %lu candidate planes", seconds_elapsed(ransacStart), baselines.size());
            
            
            if (auto wallFinder = renderer->getAnalyzerOfType<CBP_WallFinder>()) {
                std::vector<CBP_WallFinder::candidate_wall> walls;
                
                for (int i=0; i<baselines.size(); i+=2) {
                    CBP_WallFinder::candidate_wall wall;
                    wall.lineA = baselines[i];
                    wall.lineB = baselines[i+1];
                    walls.push_back(wall);
                }
                
                if (walls.size()) {
                    int64_t frameIndex = frame->frameIndex;
                    std::thread([wallFinder, walls, frameIndex](){
                        wallFinder->candidateWallsFound(walls, frameIndex);
                    }).detach();
                }
                
            }

            if (!debug.empty()) {
                Eigen::Vector2f directionMeter = m_direction.normalized() * semantic_ppm;
                cv::Point2f center = cv::Point2f(workingSize.width / 2.0f, workingSize.height / 2.0f);
                cv::arrowedLine(debug, center, cv::Point(center.x + directionMeter.x(), center.y + directionMeter.y()), cv::Scalar(0,0,255), 15);
                
                for (const auto &baseline: baselines) {
                    //std::cout << baseline << std::endl;
                    cv::line(debug, cv::Point2f(baseline[0], baseline[1]), cv::Point2f(baseline[2], baseline[3]), cv::Scalar(255,0,0), 10);
                }
                
                //std::cout << std::endl;
                Diagnostics::SaveDiagnosticImage(false, debug, "debug_(%d)", planer->secondsStable() > 0);
            }

            return true;
        }
        
        bool findDirection(const std::vector<cv::Vec4f> &detectedLines,
                           const cv::Point2f &primaryNormalsDirection,
                           const Eigen::Vector3f &heading,
                           const std::vector<cv::Point> &lastMaskContour) {
            
            std::vector<float> angles;
            
//            if (lastMaskContour.size()) {
//                std::vector<cv::Point> points;
//                cv::approxPolyDP(lastMaskContour, points, 10.0, true);
//                for (int i=0; i<points.size(); i++) {
//                    const auto pointA = points[i];
//                    const auto pointB = points[(i + 1) % points.size()];
//                    if (Geometry::euclideanDistance(pointA, pointB) < 50) continue;
//
//                    cv::Vec4f line = cv::Vec4f(pointA.x, pointA.y, pointB.x, pointB.y);
//                    float theta = fmod(Geometry::angleOfLine(line), M_PI_2);
//                    angles.push_back(theta);
//                }
//            }

            if (angles.size() < 5) {
                for (const auto &line : detectedLines) {
                    float theta = fmod(Geometry::angleOfLine(line), M_PI_2);
                    angles.push_back(theta);
                }
            }
            
            int K = 5;
            cv::Mat labels;
            cv::Mat centers;
            cv::kmeans(angles, K, labels, cv::TermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 10, 0.1), 3, cv::KMEANS_PP_CENTERS, centers);
            
            std::vector<double>distanceCounts(K);
            
            cv::Point2f neutral_direction = cv::Point2f(0.0f, 1.0f);
            
            float normalsAngle = fmod(Geometry::angleOfLine(cv::Vec4f(0,0, primaryNormalsDirection.x, primaryNormalsDirection.y)), M_PI_2);
            float headingAngle = fmod(Geometry::angleOfLine(cv::Vec4f(0,0, heading.x(), heading.z())), M_PI_2);
            
            for (int y=0; y<labels.rows; y++) {
                int label = labels.at<int>(y);
                const auto &line = detectedLines[y];
                double length = Geometry::euclideanDistanceSq(line);
                
                float theta = centers.at<float>(label);
                
                float shortestDistance = Geometry::angleDifference(theta, normalsAngle);
                float headingAngleDiff = heading.isZero() ? 0.0f : Geometry::angleDifference(theta, headingAngle);
                
                distanceCounts[label] += length / (0.1f + shortestDistance + 5.0 * headingAngleDiff);
            }
            int index = (int) std::distance(distanceCounts.begin(), std::max_element(distanceCounts.begin(), distanceCounts.end()));
            
            float theta = centers.at<float>(index);

            m_directionHistory.insert(m_directionHistory.begin(), theta);
            if (m_directionHistory.size() > 3) {
                
                if (m_directionHistory.size() > 10) {
                    m_directionHistory.pop_back();
                }
                
                cv::kmeans(m_directionHistory, fmin(m_directionHistory.size()-1, 5), labels, cv::TermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 10, 0.1), 3, cv::KMEANS_PP_CENTERS, centers);
                
                std::vector<int>counts(labels.rows);
                for (int y=0; y<labels.rows; y++) {
                    int label = labels.at<int>(y);
                    counts[label] += 1;
                }
                int highestIndex = (int) std::distance(counts.begin(), std::max_element(counts.begin(), counts.end()));
                
                theta = centers.at<float>(highestIndex);
            }

            cv::Point2f dir = rotatePoint(neutral_direction, cv::Point2f(0.0, 0.0f), theta);
            m_direction = Eigen::Vector2f(dir.x, dir.y);

            return true;
        }
        
        inline cv::Point pointInMask(const Eigen::Vector3f &point3d, const cv::Rect2f &extents3d, const cv::Size &maskSize) {
            cv::Point2f pointXY(point3d.x(), point3d.z());
            cv::Point2f pointNormalized((pointXY.x - extents3d.x) / extents3d.width, (pointXY.y - extents3d.y) / extents3d.height);
            
            return cv::Point(pointNormalized.x * float(maskSize.width), pointNormalized.y * float(maskSize.height));
        }
        
        cv::Point getClosestPoint(cv::Vec4f& L,  cv::Point& P)
        {
            cv::Point CP;
            double APx = P.x - L[0];
            double APy = P.y - L[1];
            double ABx = L[2] - L[0];
            double ABy = L[3] - L[1];
            double magAB2 = ABx*ABx + ABy*ABy;
            double ABdotAP = ABx*APx + ABy*APy;
            double t = ABdotAP / magAB2;
            
            if ( t < 0)
            {
                CP.x = L[0];
                CP.y = L[1];
            }
            else if (t > 1)
            {
                CP.x = L[2];
                CP.y = L[3];
            }
            else
            {
                CP.x = L[0] + ABx*t;
                CP.y = L[1] + ABy*t;
            }
            
            return CP;
            
        }
        
        void filterLines(const std::vector<cv::Vec4f> detectedLines, std::vector<cv::Vec4f> &goodLines, std::vector<cv::Point2f> &orientation)
        {
            for (unsigned int i=0; i<detectedLines.size(); i++)
            {
                int x0 = int(detectedLines[i][0]);
                int y0 = int(detectedLines[i][1]);
                cv::Point2f p0 = cv::Point2f(x0, y0);
                int x1 = int(detectedLines[i][2]);
                int y1 = int(detectedLines[i][3]);
                cv::Point2f p1 = cv::Point2f(x1, y1);
                
                cv::Point2f v1 = p1 - p0;
                if(v1.y<0)
                {v1 = -v1;}
                v1 = v1/std::sqrt(v1.dot(v1));
                
                orientation.push_back(v1);
                goodLines.push_back(detectedLines[i]);
            }
        }
        
//        void perpendicularLines(int K, cv::Vec2i &goodClusters, const std::vector<cv::Vec4f> &goodLines,
//                                const std::vector<cv::Point2f> &orientation, cv::Mat &centers, cv::Mat &labels)
//        {
//            auto start = sys_usec_time();
//
//            double  compactness = cv::kmeans(orientation, K, labels, cv::TermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 10, 0.1),
//                                             3, cv::KMEANS_PP_CENTERS, centers);
//
//            CBLog("kmeans took %.2f seconds with compactness  %f", seconds_elapsed(start), compactness);
//
//            float perpendicularity = 1;
//
//            for ( int i=0; i<K; i++ )
//            {
//                cv::Point2f direction1;
//                direction1.x = centers.at<float>( i,0 );
//                direction1.y = centers.at<float>( i,1 );
//                direction1 = direction1/std::sqrt(direction1.dot(direction1));
//                float perpdot0 = abs(direction1.dot(cv::Point(1,0)));
//                float perpdot1 = abs(direction1.dot(cv::Point(0,1)));
//                if(perpdot0 < .05 or perpdot1 < .05) continue;
//
//                for ( int j=0; j<i; j++ )
//                {
//                    cv::Point2f direction2;
//                    direction2.x = centers.at<float>( j,0 );
//                    direction2.y = centers.at<float>( j,1 );
//                    float perpdot20 = abs(direction2.dot(cv::Point(1,0)));
//                    float perpdot21 = abs(direction2.dot(cv::Point(0,1)));
//                    if(perpdot20 < .03 or perpdot21 < .03) continue;
//
//                    direction2 = direction2/std::sqrt(direction2.dot(direction2));
//
//                    auto perp = abs(direction1.dot(direction2));
//                    //                    CBLog("perpendicularity %f", perp);
//
//                    if(perp < .1 and perp < perpendicularity){
//
//                        perpendicularity = perp;
//                        m_basisLines.clear();
//                        m_basisLines.push_back(direction1);
//                        m_basisLines.push_back(direction2);
//
//                        cv::Point2f neutral_direction = cv::Point2f(0.0f, 1.0f);
//                        float theta = fmod(Geometry::angleOfPoints(neutral_direction, direction1), M_PI_2);
//
//                        m_directionHistory.insert(m_directionHistory.begin(), theta);
//                        if (m_directionHistory.size() > 3) {
//                            cv::Scalar mean, stddev;
//                            cv::meanStdDev(m_directionHistory, mean, stddev);
//                            CBLog("mean: %.2f, stddev: %.2f", mean[0], stddev[0]);
//                            if (m_directionHistory.size() > 10) {
//                                m_directionHistory.pop_back();
//                            }
//                        }
//
//                        cv::Point2f dir = rotatePoint(neutral_direction, cv::Point2f(0.0, 0.0f), theta);
//                        m_direction = Eigen::Vector2f(dir.x, dir.y);
//
//                        goodClusters[0] = i;
//                        goodClusters[1] = j;
//                    }
//                }
//            }
//
//            CBLog("perpendicularity   (%i , %i), %f", goodClusters[0],goodClusters[1], perpendicularity);
//
//        }
        
        cv::Point2f rotate2d(const cv::Point2f& inPoint, const double& angRad)
        {
            cv::Point2f outPoint;
            //CW rotation
            outPoint.x = std::cos(angRad)*inPoint.x - std::sin(angRad)*inPoint.y;
            outPoint.y = std::sin(angRad)*inPoint.x + std::cos(angRad)*inPoint.y;
            return outPoint;
        }
        
        cv::Point2f rotatePoint(const cv::Point2f& inPoint, const cv::Point2f& center, const double& angRad)
        {
            return rotate2d(inPoint - center, angRad) + center;
        }
        
        double m_constantMean = 5.0f;
        double m_constantStdDev = 15.0f;
        
        /*
        void removeThings() {
            cv::Mat mask = cv::Mat::zeros(256, 256, CV_8UC1);
            mask.setTo(255, result.outputs[result_index_semantic] < 5);
            Accelerated::roughErode(mask, mask, cv::Size(20,20));
            
            std::vector<cv::Point> nzPoints;
            cv::findNonZero(mask, nzPoints);
            
            if (nzPoints.size()) {
                
                std::vector<cv::Point> seeds;
                seeds.reserve(nzPoints.size());
                cv::Mat src;
                cv::resize(frame->uprightRGBImage(), src, cv::Size(512,512));
                float scaleX = float(src.cols) / float(result.outputs[result_index_semantic].cols);
                float scaleY = float(src.rows) / float(result.outputs[result_index_semantic].rows);
                for (const auto &point : nzPoints) {
                    seeds.push_back(cv::Point(scaleX * float(point.x), scaleY * float(point.y)));
                }
                
                cv::Mat onMask;
                cv::Size unitSize(20,20);
                
                double meanThreshold =  m_constantMean;
                double stddevThreshold = m_constantStdDev;
                
                auto start = sys_usec_time();
                int numFilled = floodFill(src, unitSize, onMask, seeds, [meanThreshold, stddevThreshold](const cv::Mat & image, const cv::Rect &indexRoi, const cv::Rect &siblingRoi) {
                    
                    cv::Scalar meanA, meanB, stddevA, stddevB;
                    cv::meanStdDev(image(indexRoi), meanA, stddevA);
                    cv::meanStdDev(image(siblingRoi), meanB, stddevB);
                    
                    double totalMean = 0;
                    double totalStdDev = 0;
                    for (int i=0; i<3; i++) {
                        totalMean += powf(meanA[i] - meanB[i], 2.0);
                        totalStdDev += powf(stddevA[i] - stddevB[i], 2.0);
                    }
                    
                    double avgMean = sqrt(totalMean);
                    double avgStdDev = sqrt(totalStdDev);
                    
                    return avgMean < meanThreshold && avgStdDev <= stddevThreshold;
                });
                
                if (numFilled) {
                    CBLog("Flood fill took %.4f seconds", seconds_elapsed(start));
                    cv::resize(mask, mask, src.size(), 0, 0, cv::INTER_NEAREST);
                    Diagnostics::SaveDiagnosticMask(mask, src, "before");
                    
                    cv::resize(onMask, onMask, src.size(), 0, 0, cv::INTER_NEAREST);
                    Diagnostics::SaveDiagnosticMask(onMask, src, "filled");
                }
            }
        }
         */
    };
    
    CBP_GroundSurfaceAnalyzer::CBP_GroundSurfaceAnalyzer() : CBP_AreaAnalyzer("CBP_GroundSurfaceAnalyzer") {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
        
        setPriority(CBThreadPriorityLowest);
    }
    
    CBP_GroundSurfaceAnalyzer::~CBP_GroundSurfaceAnalyzer() {
        
    }
    
    bool CBP_GroundSurfaceAnalyzer::analyze(cbar::CBAR_VideoFramePtr frame) {
        return m_pImpl->analyze(frame);
    }
    
    Eigen::Vector2f CBP_GroundSurfaceAnalyzer::getPrimaryDirection() const {
        return m_pImpl->getPrimaryDirection();
    }
};
