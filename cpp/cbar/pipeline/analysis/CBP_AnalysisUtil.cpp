//
//  CBP_AnalysisUtil.cpp
//  Cambrian
//
//  Created by Joel Teply on 4/21/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#include "CBP_AnalysisUtil.hpp"

#include <imaging/ImageProcessing.h>
#include <imaging/Coloring.h>
#include <cbar/pipeline/util/util.h>
#include <imaging/Imaging.h>

using namespace imaging;

namespace cbpipe {
    
    cv::Vec3b CBP_AnalysisUtil::getMostCommonColor(const cv::Mat &src, const cv::Mat &mask, int K) {
        
        std::vector<cv::Vec3b> points;
        std::vector<cv::Point> locations;
        for( int y = 0; y < src.rows; y++) {
            for( int x = 0; x < src.cols; x++) {
                if ( (int)mask.at<unsigned char>(y,x) != 0 ) {
                    points.push_back(src.at<cv::Vec3b>(y,x));
                    locations.push_back(cv::Point(x,y));
                }
            }
        }
        
        if (points.size() < K) return cv::Vec3b();
        
        cv::Mat kmeanPoints(int(points.size()), 3, CV_32F);
        for( int y = 0; y < points.size(); y++ ) {
            for( int z = 0; z < 3; z++) {
                kmeanPoints.at<float>(y, z) = points[y][z];
            }
        }
        
        cv::Mat labels;
        cv::Mat centers;
        cv::kmeans(kmeanPoints, K, labels, cv::TermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS, 10, 0.1), 10, cv::KMEANS_PP_CENTERS, centers);
        
        int classes[K];
        int max = -1; int index = -1;
        memset(classes, 0, sizeof(classes[0]) * K);
        int * labels_ptr = labels.ptr<int>(0);
        for (int i = 0; i < labels.rows; ++i)
            classes[*labels_ptr++]++;
        for (int i = 0; i < K; ++i) {
            if (classes[i] > max) {
                max = classes[i];
                index = i;
            }
        }
        
        cv::Vec3b tempColor;
        tempColor[0] = centers.at<float>(index, 0);
        tempColor[1] = centers.at<float>(index, 1);
        tempColor[2] = centers.at<float>(index, 2);
        return tempColor;
    }
    
    cv::Point CBP_AnalysisUtil::pointInMask(const Eigen::Vector3f &point3d, const cv::Rect2f &extents3d, const cv::Size &maskSize) {
        cv::Point2f pointXY(point3d.x(), point3d.z());
        cv::Point2f pointNormalized((pointXY.x - extents3d.x) / extents3d.width, (pointXY.y - extents3d.y) / extents3d.height);
        
        return cv::Point(pointNormalized.x * float(maskSize.width), pointNormalized.y * float(maskSize.height));
    }
    
    inline float scaleComponent(float color, float range=255.0f) {
        return 2.0 * (float(color)/float(range)) - 1.0;
    }
    
    inline float descaleComponent(float direction, float range=255.0f) {
        return 0.5f * (direction + 1.0f) * range;
    }
    
    Eigen::Vector3f CBP_AnalysisUtil::colorToDirection(const cv::Scalar &color) {
        Eigen::Vector3f direction;
        direction.x() = scaleComponent(color[0]);
        direction.y() = scaleComponent(color[1]);
        direction.z() = scaleComponent(color[2]);
        return direction.normalized();
    }
    
    cv::Scalar CBP_AnalysisUtil::directionToColor(const Eigen::Vector3f &direction) {
        cv::Scalar color;
        color[0] = descaleComponent(direction.x());
        color[1] = descaleComponent(direction.y());
        color[2] = descaleComponent(direction.z());
        return color;
    }
    
    bool CBP_AnalysisUtil::kmeansNormals(const cv::Mat &normals,
                                         const cv::Size &workingSize,
                                         std::vector<cv::Mat>&surfaces,
                                         std::vector<int>&counts,
                                         std::vector<cv::Point2f>&directions,
                                         std::vector<cv::Scalar>&colors,
                                         cv::Mat *debug) {
        
        cv::Mat normalsResized;
        cv::resize(normals, normalsResized, workingSize);
        
        cv::Mat kmeans, centers, labels;
        int K = ImageProcessing::kmeansColor(normalsResized, kmeans, 0, &centers, &labels);
        
        int blackIndex = 0;
        Eigen::Vector3f up(0,1,0);
        std::map<int, Eigen::Vector3f> orthagonalDirections;
        for (int i=0; i<centers.rows; i++) {
            float r=centers.at<float>(i, 0);
            float g=centers.at<float>(i, 1);
            float b=centers.at<float>(i, 2);
            
            double length = (r + g + b) / 3.0;
            if (length < 0.2f) {
                blackIndex = i;
            } else {
                auto kDirection3D = colorToDirection(cv::Scalar(r,g,b));
                double angle = CBP_MatrixUtil::angleBetweenVectors(up, kDirection3D);
                double orthagonalAngle = fabs(angle - M_PI_2);
                
                if (orthagonalAngle < 0.33 * M_PI_2) {
                    orthagonalDirections[i] = kDirection3D;
                }
            }
        }
        
        std::vector<cv::Mat>allSurfaces;
        for (int i=0; i<K; i++) allSurfaces.push_back(cv::Mat::zeros(labels.rows, labels.cols, CV_8UC1));
        std::vector<int>allCounts((int)allSurfaces.size());
        
        for (int y=0; y<labels.rows; y++) {
            for (int x=0; x<labels.cols; x++) {
                cv::Point point = cv::Point(x,y);
                int label = labels.at<int>(point);
                allSurfaces[label].at<uchar>(point) = 255;
                allCounts[label] ++;
            }
        }
        
        //combine like surfaces
        std::vector<cv::Mat>filteredSurfaces;
        std::vector<int> filteredCounts;
        //double angleSimilar = M_PI_4 * 0.66;
        std::vector<int> allMatches;
        int minArea = labels.rows * labels.cols / 50;//1/nth of area
        
        for (auto itrA : orthagonalDirections) {
            if (std::find(allMatches.begin(), allMatches.end(), itrA.first) != allMatches.end()) continue; //already matched
            
            if (allCounts[itrA.first] < minArea) continue;
            
            cv::Mat surface = allSurfaces[itrA.first];
            
#if COMBINE_SURFACES
            std::vector<int> matches;
            for (auto itrB : orthagonalDirections) {
                if (itrA.first >= itrB.first) continue;
                
                if (CBP_MatrixUtil::angleBetweenVectors(itrA.second, itrB.second) < angleSimilar) {
                    matches.push_back(itrB.first);
                }
            }
            
            if (matches.size()) {
                for (auto match : matches) {
                    surface += allSurfaces[match];
                }
                allMatches.insert(allMatches.end(), matches.begin(), matches.end());
            }
#endif
            
            filteredCounts.push_back(allCounts[itrA.first]);
            filteredSurfaces.push_back(surface);
        }
        
        for (int i=0; i<filteredSurfaces.size(); i++) {
            const auto &surface = filteredSurfaces[i];
            const int &count = filteredCounts[i];
            
            cv::Scalar modeColor = getMostCommonColor(normalsResized, surface, 5);
            if (modeColor == cv::Scalar()) continue;
            
            auto mode = colorToDirection(modeColor);
            
            cv::Scalar mean, stddev;
            cv::meanStdDev(normalsResized, mean, stddev, surface);
            
            surfaces.push_back(surface);
            
            auto direction3d = colorToDirection(mean);
            auto direction = cv::Point2f(direction3d.x(), direction3d.z());
            directions.push_back(direction);
            colors.push_back(mean);
            counts.push_back(count);
            
            if (debug && !debug->empty()) {
                //draw outlines
                //                    cv::Mat debugSurface;
                //                    cv::resize(surface, debugSurface, debug.size());
                //                    cv::blur(debugSurface, debug, cv::Size(5,5));
                //                    ImageProcessing::drawPolygons(debugSurface, debug, mean, 5, 31);
                
                cv::Moments m = cv::moments(surface, false);
                cv::Point centerPoint(debug->cols * (m.m10/m.m00) / surface.cols, debug->rows * (m.m01/m.m00) / surface.cols);
                double drawLength = 80.0f;
                cv::arrowedLine(*debug, centerPoint, cv::Point(centerPoint.x + direction.x * drawLength, centerPoint.y + direction.y * drawLength), cv::Scalar(255,255,20), 5, cv::LINE_AA);
                
                cv::arrowedLine(*debug, cv::Point(20,20) + centerPoint, cv::Point(20,20) + cv::Point(centerPoint.x + mode.x() * drawLength, centerPoint.y + mode.z() * drawLength), cv::Scalar(255,0,255), 5, cv::LINE_AA);
                
                
                //Diagnostics::SaveDiagnosticImage(false, surface, "surface_%d", surfaces.size());
                
            }
            
        }
        
        //CBLog("Detected %d surfaces", filteredSurfaces.size());
        return filteredSurfaces.size() > 0 && directions.size() > 0;
    }
    
    std::vector<cv::Point2f> CBP_AnalysisUtil::getLineContourIntersections(const cv::Vec4f &line, const std::vector<cv::Point> &contour) {
        
        
        cv::Point2f lineA = cv::Point2f(line[0], line[1]);
        cv::Point2f lineB = cv::Point2f(line[2], line[3]);
        
        auto numPoints = contour.size();
        std::vector<cv::Point2f> intersections;
        for (int i=0; i<numPoints; i++) {
            cv::Point2f contourPointA = contour[i];
            cv::Point2f contourPointB = contour[(i+1) % numPoints];
            
            cv::Point2f intersection;
            if (Geometry::segmentsIntersect(contourPointA, contourPointB, lineA, lineB, intersection)) {
                intersections.push_back(intersection);
            }
        }
        
        return intersections;
    }
    
};

