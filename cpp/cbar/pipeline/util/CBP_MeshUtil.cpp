//
//  CBP_MeshUtil.cpp
//  Cambrian
//
//  Created by Joel Teply on 1/29/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#include "CBP_MeshUtil.hpp"
#include <imaging/ImageProcessing.h>
#include <cbar/pipeline/util/CBP_Triangulator.hpp>

#define LOG_POLYGON 0

using namespace imaging;

namespace cbpipe {
    
    bool isInsideTriangle(const cv::Point &A, const cv::Point &B, const cv::Point &C, const cv::Point &P) {
        float ax, az, bx, bz, cx, cz, apx, apz, bpx, bpz, cpx, cpz;
        float cCROSSap, bCROSScp, aCROSSbp;
        
        ax = C.x - B.x;
        az = C.y - B.y;
        bx = A.x - C.x;
        bz = A.y - C.y;
        cx = B.x - A.x;
        cz = B.y - A.y;
        apx = P.x - A.x;
        apz = P.y - A.y;
        bpx = P.x - B.x;
        bpz = P.y - B.y;
        cpx = P.x - C.x;
        cpz = P.y - C.y;
        
        aCROSSbp = ax * bpz - az * bpx;
        cCROSSap = cx * apz - cz * apx;
        bCROSScp = bx * cpz - bz * cpx;
        
        return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
    }
    
    float area(const std::vector<cv::Point> &points) {
        int n = (int) points.size();
        float A = 0.0f;
        for (int p = n - 1, q = 0; q < n; p = q++) {
            cv::Point pval = points[p];
            cv::Point qval = points[q];
            A += pval.x * qval.y - qval.x * pval.y;
        }
        return (A * 0.5f);
    }
    
    bool snip(const std::vector<cv::Point> &points, int u, int v, int w, int n, const std::vector<int> &V) {
        int p;
        const cv::Point &A = points[V[u]];
        const cv::Point &B = points[V[v]];
        const cv::Point &C = points[V[w]];
        if (FLT_EPSILON > (((B.x - A.x) * (C.y - A.y)) - ((B.y - A.y) * (C.x - A.x))))
            return false;
        for (p = 0; p < n; p++) {
            if ((p == u) || (p == v) || (p == w))
                continue;
            const cv::Point &P = points[V[p]];
            if (isInsideTriangle(A, B, C, P))
                return false;
        }
        return true;
    }
    
    std::vector<int> generateMeshFromContour(const std::vector<cv::Point> &points, int startIndex) {
        std::vector<int> indices;
        
        int n = (int) points.size();
        if (n < 3)
            return indices;
        
        std::vector<int> V(n);
        if (area(points) > 0) {
            for (int v = 0; v < n; v++)
                V[v] = v;
        }
        else {
            for (int v = 0; v < n; v++)
                V[v] = (n - 1) - v;
        }
        
        int nv = n;
        int count = 2 * nv;
        for (int m = 0, v = nv - 1; nv > 2;) {
            if ((count--) <= 0)
                return indices;
            
            int u = v;
            if (nv <= u)
                u = 0;
            v = u + 1;
            if (nv <= v)
                v = 0;
            int w = v + 1;
            if (nv <= w)
                w = 0;
            
            if (snip(points, u, v, w, nv, V)) {
                int a, b, c, s, t;
                a = V[u];
                b = V[v];
                c = V[w];
                indices.push_back(a + startIndex);
                indices.push_back(b + startIndex);
                indices.push_back(c + startIndex);
                m++;
                for (s = v, t = v + 1; t < nv; s++, t++)
                    V[s] = V[t];
                nv--;
                count = 2 * nv;
            }
        }
        
        std::reverse(indices.begin(), indices.end());
        return indices;
    }
    
    struct polygon {
        std::vector<cv::Point> boundaryPoints;
        std::vector<std::vector<cv::Point>> holes;
    };
    
    
    
    cv::Rect2f CBP_MeshUtil::createPolygonsForMask(const cv::Mat &mask,
                                                   std::vector<Eigen::Vector3f> &vertices,
                                                   std::vector<int> &indices,
                                                   std::vector<std::vector<cv::Point>> &boundaries,
                                                   float scale, cv::Point2f offset,
                                                   double polyEpsilon, double minHoleArea, double minOuterArea) {
        std::vector<cv::Vec4i> hierarchy;
        std::vector<std::vector<cv::Point>> planeContours;
        int flags = CV_RETR_EXTERNAL;
        if (minHoleArea < FLT_MAX) {
            flags = CV_RETR_TREE;
        }
        cv::findContours(mask, planeContours, hierarchy, flags, CV_CHAIN_APPROX_TC89_KCOS);
        
        //can make these defaults
        std::map<int, polygon> polygons;

        for (int i=0; i<planeContours.size(); i++) {
            const auto &contour = planeContours[i];
            if (contour.size() < 3) continue;
            
            double area = cv::contourArea(contour);
            int parentContour = hierarchy[i][3];
            
            if ((parentContour >= 0 && area < minHoleArea) || (parentContour < 0 && area < minOuterArea)) continue;
            
            if (parentContour >=0) {
                polygons[parentContour].holes.push_back(contour);
            } else {
                polygon poly;
                
                if (polyEpsilon > 0) {
                    cv::approxPolyDP(contour, poly.boundaryPoints, polyEpsilon, false);
                }
                else {
                    poly.boundaryPoints = contour;
                }
                boundaries.push_back(poly.boundaryPoints);
                polygons[i] = poly;
            }
        }
        
        cv::Rect2f roi3d;
        for (auto const& itr : polygons) {
            CBP_Triangulator triangulator;
            
#if LOG_POLYGON
            triangulator.setDebuggingEnabled(true);
#endif
            
            triangulator.setMaxArea(200);
            //triangulator.setMinRadians(0.3);
            
            triangulator.setBoundaryContour(itr.second.boundaryPoints);
            
            for (auto const& hole : itr.second.holes) {
                triangulator.addHole(hole);
            }
            
            std::vector<Eigen::Vector3f> newVertices;
            std::vector<int> newIndices;
            roi3d |= triangulator.triangulate(newVertices, newIndices, Eigen::Vector3f(offset.x, 0, offset.y), scale, vertices.size());
            
            vertices.insert(vertices.end(), newVertices.begin(), newVertices.end());
            indices.insert(indices.end(), newIndices.begin(), newIndices.end());
        }
        
        return roi3d;
    }
    
    void CBP_MeshUtil::drawPolygon(cv::Mat &dest, const std::vector<Eigen::Vector3f> &vertices, const std::vector<int> &indices,
                                   const cv::Rect2f &roi, float scale, cv::Scalar color) {
        dest = cv::Mat::zeros(int(scale * roi.height), int(scale * roi.width), CV_8UC3);
        
        cv::Point offset = cv::Point(scale * roi.x, scale * roi.y);
        size_t numIndices = indices.size();
        for (int i=0; i<numIndices; i++) {

            const Eigen::Vector3f verticeA = scale * vertices[indices[i]];
            const Eigen::Vector3f verticeB = scale * vertices[indices[(i+1) % numIndices]];
            
            cv::Point pointA = cv::Point(verticeA.x(), verticeA.z()) - offset;
            cv::Point pointB = cv::Point(verticeB.x(), verticeB.z()) - offset;
            cv::line(dest, pointA, pointB, color);
            cv::circle(dest, pointA, 5, color, cv::FILLED, cv::LINE_AA);
        }
    }
}

