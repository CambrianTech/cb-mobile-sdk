//
//  CBP_Triangulator.cpp
//  Cambrian
//
//  Created by Joel Teply on 2/8/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#include "CBP_Triangulator.hpp"

#include <vector>
#include <unordered_set>
#include <opencv2/opencv.hpp>
#include <imaging/ImageProcessing.h>
#include <cbar/pipeline/util/util.h>

#define REAL double
#define VOID void
#include "triangle.h"

int triunsuitable(REAL* triorg, REAL* tridest, REAL* triapex, REAL area)
{
    if (area < 10) return 0;
    
    REAL dxoa, dxda, dxod;
    REAL dyoa, dyda, dyod;
    REAL oalen, dalen, odlen;
    REAL maxlen;
    
    dxoa = triorg[0] - triapex[0];
    dyoa = triorg[1] - triapex[1];
    dxda = tridest[0] - triapex[0];
    dyda = tridest[1] - triapex[1];
    dxod = triorg[0] - tridest[0];
    dyod = triorg[1] - tridest[1];
    /* Find the squares of the lengths of the triangle's three edges. */
    oalen = dxoa * dxoa + dyoa * dyoa;
    dalen = dxda * dxda + dyda * dyda;
    odlen = dxod * dxod + dyod * dyod;
    /* Find the square of the length of the longest edge. */
    maxlen = (dalen > oalen) ? dalen : oalen;
    maxlen = (odlen > maxlen) ? odlen : maxlen;
    
    if (maxlen > 0.05 * (triorg[0] * triorg[0] + triorg[1] * triorg[1]) + 0.02) {
        return 1;
    } else {
        return 0;
    }
}

namespace cbpipe {
    
    struct CBP_Triangulator::Impl {
        Impl() {
            
        }
        
        ~Impl() {
            
        }
        
        // Boundary vertices.
        std::vector<cv::Point> m_boundaryPoints;
        
        // Extra points.
        std::vector<std::vector<cv::Point>> m_holes;
        std::vector<REAL> m_holeIdentifiers;
        
        // Flags.
        bool m_allowBoundaryRefinement = false;
        bool m_allowEdgeRefinement = true;
        bool m_constrainAngle = true;
        bool m_planarGraph = true;
        
        double m_maxArea = -1;
        int m_allowSteinerPoints = -1;
        bool m_recordOrder = false;
        bool m_verbose = false;
        
        triangulateio m_in, m_out;
        
        bool m_debugEnabled = false;
        
        // Assign values to the Triangle variables.
        void assignValues(triangulateio &in) {
            
            int totalPoints = 0;
            totalPoints += (int) m_boundaryPoints.size();
            
            for (const auto &hole : m_holes) {
                totalPoints += hole.size();
            }
            
            in.numberofpoints = totalPoints;
            in.numberofpointattributes = m_recordOrder ? 1 : 0;
            
            in.pointlist = (double *) malloc(in.numberofpoints * 2 * sizeof(double));
            in.pointmarkerlist = (int *) malloc(in.numberofpoints * sizeof(int));
            
            if (m_recordOrder)
                in.pointattributelist = (double *) malloc(in.numberofpointattributes * in.numberofpoints * sizeof(double));
            
            in.numberofsegments = totalPoints;
            in.segmentlist = (int *) malloc(in.numberofsegments * 2 * sizeof(int));
            in.segmentmarkerlist = (int *) malloc(in.numberofsegments * sizeof(int));
            
            if (m_recordOrder) {
                for (int i = 0; i < in.numberofpoints; ++i)
                    in.pointattributelist[i] = i;
            }
            
            size_t numOuterBoundaryPoints = m_boundaryPoints.size();
            for (int i = 0; i < numOuterBoundaryPoints; ++i) {
                in.pointlist[2 * i] = m_boundaryPoints[i].x;
                in.pointlist[2 * i + 1] = m_boundaryPoints[i].y;
                
                in.pointmarkerlist[i] = 1;
                in.segmentmarkerlist[i] = 1;
                
                in.segmentlist[2 * i] = i;
                
                if (i == numOuterBoundaryPoints - 1) in.segmentlist[2 * i + 1] = 0;
                else in.segmentlist[2 * i + 1] = i + 1;
            }
            
            size_t humHoles = m_holes.size();
            size_t holeStart = numOuterBoundaryPoints;
            for (int i = 0; i < humHoles; ++i) {
                const auto &hole = m_holes[i];
                int numHolePoints = int(hole.size());
                for (int j = 0; j < numHolePoints; ++j) {
                    int pointIndex = int(j + holeStart);
                    in.pointlist[2 * pointIndex] = hole[j].x;
                    in.pointlist[2 * pointIndex + 1] = hole[j].y;
                    
                    in.pointmarkerlist[pointIndex] = i + 2;
                    in.segmentmarkerlist[pointIndex] = i + 2;
                    
                    in.segmentlist[2 * pointIndex] = pointIndex;
                    
                    if (j == numHolePoints - 1) in.segmentlist[2 * pointIndex + 1] = holeStart;
                    else in.segmentlist[2 * pointIndex + 1] = pointIndex + 1;
                }
                holeStart += numHolePoints;
            }
            
            in.holelist = &m_holeIdentifiers[0];
            in.numberofholes = int(humHoles);
            
            in.numberofregions = 0;
        }
        
        // Assign flags and triangulate.
        bool runTriangulation(triangulateio &mid, triangulateio &out) const {
            
            out.pointlist = (double *) NULL;
            out.pointmarkerlist = (int *) NULL;
            
            if (m_recordOrder) out.pointattributelist = (double *) NULL;
            
            out.trianglelist = (int *) NULL;
            out.triangleattributelist = (double *) NULL;
            
            out.segmentlist = (int *) NULL;
            out.segmentmarkerlist = (int *) NULL;
            
            out.edgelist = (int *) NULL;
            out.edgemarkerlist = (int *) NULL;
            
            std::ostringstream flags_stream;
            
            if (!m_verbose) flags_stream << "Q";
            else flags_stream << "V";
            
            flags_stream << "ez";
            
            if (!m_allowEdgeRefinement) flags_stream << "YY";
            else if (!m_allowBoundaryRefinement) flags_stream << "Y";
            
            if (m_constrainAngle) flags_stream << "q";
            
            if (m_planarGraph) flags_stream << "p";
            
            if (m_allowSteinerPoints >= 0) flags_stream << "S" << m_allowSteinerPoints;
            
            if (m_maxArea > 0.0) flags_stream << "a" << m_maxArea;

            const std::string flags = flags_stream.str();
            
            try {
                ::triangulate(const_cast<char*>(flags.c_str()), &mid, &out, 0);
            } catch (int) {
                return false;
            }
            return true;
        }
        
        // Set/save vertices and faces obtained from Triangle.
        std::vector<cv::Vec6f> getResults(const triangulateio &out) const {
            
            std::vector<cv::Vec6f> triangles;
            triangles.reserve(out.numberoftriangles);
            for (int i = 0; i < out.numberoftriangles; ++i) {
                const int* tri_p = &out.trianglelist[3 * i];
                cv::Point2f p[3];
                for (int j = 0; j < 3; ++j) {
                    const REAL* ptr = &out.pointlist[2 * tri_p[j]];
                    p[j] = cv::Point2f(ptr[0], ptr[1]);
                }
                if (!std::isfinite(p[0].x) || !std::isfinite(p[0].y) ||
                    !std::isfinite(p[1].x) || !std::isfinite(p[1].y) ||
                    !std::isfinite(p[2].x) || !std::isfinite(p[2].y))
                    continue;
//                if (cloud->isValid(p[0].x, p[0].y) == false &&
//                    cloud->isValid(p[1].x, p[1].y) == false &&
//                    cloud->isValid(p[2].x, p[2].y) == false)
//                    continue;
                
                cv::Vec6f t;
                for (int j = 0; j < 3; ++j) {
                    const REAL* ptr = &out.pointlist[2 * tri_p[j]];
                    t[2 * j + 0] = ptr[0];
                    t[2 * j + 1] = ptr[1];
                }
                triangles.push_back(t);
            }
            return triangles;
        }
        
        // Generate triangulation.
        std::vector<cv::Vec6f> process() {
            
            triangulateio out;
            triangulateio in;

            assignValues(in);
            
            bool success = runTriangulation(in, out);
            
            freeIn(in);
            
            std::vector<cv::Vec6f> triangles;
            if (!success) {
                return triangles;
            }
            triangles = getResults(out);
            
            freeOut(out);
            
            return triangles;
        }
        
        inline void handlePoint(cv::Point point2d, std::vector<cv::Point> &pointMap,
                                std::vector<Eigen::Vector3f> &vertices, std::vector<int> &indices,
                                size_t indexOffset, const Eigen::Vector3f &offset, REAL scale,
                                cv::Point2f &minPoint, cv::Point2f &maxPoint) {
            
            const auto ita = std::find(pointMap.begin(), pointMap.end(), point2d);
            if (ita != pointMap.end()) {
                indices.push_back(int(indexOffset) + int(std::distance(pointMap.begin(), ita)));
            } else {
                indices.push_back(int(indexOffset) + int(pointMap.size()));
                
                Eigen::Vector3f point3D = Eigen::Vector3f(REAL(point2d.x) / scale, 0, REAL(point2d.y) / scale) + offset;
                vertices.push_back(point3D);
                pointMap.push_back(point2d);
                
                minPoint.x = fmin(minPoint.x, point3D.x());
                minPoint.y = fmin(minPoint.y, point3D.z());
                maxPoint.x = fmax(maxPoint.x, point3D.x());
                maxPoint.y = fmax(maxPoint.y, point3D.z());
            }
        }
        
        cv::Rect2f triangulate(std::vector<Eigen::Vector3f> &vertices, std::vector<int> &indices,
                               const Eigen::Vector3f &offset, float scale, size_t indexOffset) {
            
            cv::Point2f minPoint = cv::Point2f(FLT_MAX, FLT_MAX);
            cv::Point2f maxPoint = cv::Point2f(-FLT_MAX, -FLT_MAX);
            
            std::vector<cv::Vec6f> triangles = process();
            
            if (!triangles.size()) {
                return cv::Rect2f();
            }

            std::vector<cv::Point> pointMap;
            
            for (const auto &triangle : triangles) {
                for (int i=0; i<6; i+=2) {
                    cv::Point point2d = cv::Point(triangle[i], triangle[i+1]);
                    handlePoint(point2d, pointMap, vertices, indices, indexOffset, offset, scale, minPoint, maxPoint);
                }
            }
            
            return cv::Rect2f(minPoint.x, minPoint.y, maxPoint.x - minPoint.x, maxPoint.y - minPoint.y);
        }
        
        // Free memory.
        void freeIn(triangulateio &in) const {
            
            trifree(in.pointlist);
            trifree(in.pointmarkerlist);
            
            trifree(in.segmentlist);
            trifree(in.segmentmarkerlist);
            
            if (m_recordOrder) trifree(in.pointattributelist);
        }
        
        void freeOut(triangulateio &out) const {
            
            trifree(out.pointlist);
            trifree(out.pointmarkerlist);
            
            if (m_recordOrder) trifree(out.pointattributelist);
            
            trifree(out.trianglelist);
            trifree(out.triangleattributelist);
            
            trifree(out.segmentlist);
            trifree(out.segmentmarkerlist);
            
            trifree(out.edgelist);
            trifree(out.edgemarkerlist);
        }
    };
    
    CBP_Triangulator::CBP_Triangulator() {
        p_ = std::unique_ptr<Impl>(new Impl());
    }
    
    CBP_Triangulator::~CBP_Triangulator() {
        
    }
    
    double CBP_Triangulator::maxArea() const {
        return p_->m_maxArea;
    }
    
    void CBP_Triangulator::setMaxArea(double area) {
        p_->m_maxArea = area;
    }
    
    double CBP_Triangulator::minRadians() const {
        return p_->m_constrainAngle;
    }
    
    void CBP_Triangulator::setMinRadians(double radians) {
        p_->m_constrainAngle = radians;
    }
    
    bool CBP_Triangulator::debuggingEnabled() const {
        return p_->m_debugEnabled;
    }
    
    void CBP_Triangulator::setDebuggingEnabled(bool enabled) {
        p_->m_debugEnabled = enabled;
    }
    
    void CBP_Triangulator::setBoundaryContour(const std::vector<cv::Point>& contour) {
        if (contour.size() < 3)
            return;
        
        p_->m_boundaryPoints = contour;
    }
    
    //not fully implemented
    void CBP_Triangulator::addHole(const std::vector<cv::Point>& contour) {
        if (contour.size() < 3)
            return;
        
        const cv::Point2f sumPoint = contour[0] + contour[1] + contour[2];
        const cv::Point2f ptInsideHole = sumPoint / 3.0f;
        
        if (cv::pointPolygonTest(contour, ptInsideHole, false) > 0) {
            p_->m_holes.push_back(contour);
            p_->m_holeIdentifiers.push_back(ptInsideHole.x);
            p_->m_holeIdentifiers.push_back(ptInsideHole.y);
        }
        //could find a better one here if above invalid:
    }
    
    cv::Rect2f CBP_Triangulator::triangulate(std::vector<Eigen::Vector3f> &vertices, std::vector<int> &indices,
                                             const Eigen::Vector3f &offset, float scale, size_t indexOffset) {
        auto roi3d = p_->triangulate(vertices, indices, offset, scale, indexOffset);
        
        if (p_->m_debugEnabled) {
            cv::Mat debug;
            CBP_MeshUtil::drawPolygon(debug, vertices, indices, roi3d, 400, cv::Scalar(100,100,255));
            
            //kind of inefficient, but debugging, so ¯\_(ツ)_/¯
            cv::Rect2f roi2d = cv::boundingRect(p_->m_boundaryPoints);
            
            cv::Scalar boundaryColor = cv::Scalar(255,100,100);
            
            size_t numIndices = p_->m_boundaryPoints.size();
            for (int j=0; j<numIndices; j++) {
                cv::Point2f pointA = p_->m_boundaryPoints[j];
                cv::Point2f pointB = p_->m_boundaryPoints[(j+1) % numIndices];
                
                pointA = (pointA - roi2d.tl());
                pointA.x = float(debug.cols) * pointA.x / roi2d.width;
                pointA.y = float(debug.rows) * pointA.y / roi2d.height;
                
                pointB = (pointB - roi2d.tl());
                pointB.x = float(debug.cols) * pointB.x / roi2d.width;
                pointB.y = float(debug.rows) * pointB.y / roi2d.height;
                
                cv::line(debug, pointA, pointB, boundaryColor, 3);
            }
            
            Diagnostics::SaveDiagnosticImage(false, debug, "polygon", scale);
        }
        
        return roi3d;
    }
};

