//
//  Geometry.cpp
//
//
//  Created by Joel Teply on 6/14/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "Geometry.h"

namespace imaging {
    
    static float M_2PI = (2.0f * M_PI);
    
    float Geometry::normalizeAngle(float angle)
    {
        angle = fmod(angle + M_PI, M_2PI);
        if (angle < 0)
            angle += M_2PI;
        return angle - M_PI;
    }
    
    float Geometry::_angleDifference(float angleA, float angleB)
    {
        angleA = normalizeAngle(angleA);
        angleB = normalizeAngle(angleB);
        
        float angleDiffA = angleA - angleB;
        float angleDiffB = angleB - angleA;
        
        if (fabs(angleDiffA) < fabs(angleDiffB)) {
            return angleDiffA;
        }
        return angleDiffB;
        
        return std::min(M_2PI - fabs(angleA - angleB), fabs(angleA - angleB));
    }
    
    float Geometry::angleDifference(float angleA, float angleB)
    {
        float differenceA = angleA - angleB;
        float differenceB = angleA - (2 * M_PI + angleB);
        float differenceC = (2 * M_PI + angleA) - angleB;
        
        //this is serious lazy shit here
        if (fabs(differenceA) < fabs(differenceB) && fabs(differenceA) < fabs(differenceC)) {
            return differenceA;
        }
        if (fabs(differenceB) < fabs(differenceC)) {
            return differenceB;
        }
        return differenceC;
    }
    
    float Geometry::lineAngleDifference(const cv::Vec4f &lineA, const cv::Vec4f &lineB) {
        
        Eigen::Vector2f vectorA = Eigen::Vector2f(lineA[0] - lineA[2], lineA[1] - lineA[3]).normalized();
        Eigen::Vector2f vectorB = Eigen::Vector2f(lineB[0] - lineB[2], lineB[1] - lineB[3]).normalized();
        
        return acos(vectorA.dot(vectorB));
    }
    
    float Geometry::linePerpendicularity(const cv::Vec4f &lineA, const cv::Vec4f &lineB) {
        float angleBetween = lineAngleDifference(lineA, lineB);
        return fabs(angleBetween - M_PI_2) / M_PI_2;
    }
    
    float Geometry::lineParallelism(const cv::Vec4f &lineA, const cv::Vec4f &lineB) {
        float angleBetween = lineAngleDifference(lineA, lineB);
        return fabs(angleBetween) / M_PI_2;
    }
    
    cv::Rect Geometry::rectAtPoint(const cv::Point &point, int radius, const cv::Mat &cvMat)
    {
        if (radius > cvMat.cols / 2) radius = cvMat.cols / 2;
        if (radius > cvMat.rows / 2) radius = cvMat.rows / 2;
        //mean
        int xStart = point.x - radius;
        
        if (xStart < 0)
            xStart = 0;
        if (xStart + 2 * radius >= cvMat.cols)
            xStart = cvMat.cols - 2 * radius - 1;
        
        int yStart = point.y - radius;
        if (yStart < 0)
            yStart = 0;
        if (yStart + 2 * radius >= cvMat.rows)
            yStart = cvMat.rows - 2 * radius - 1;
        
        //printf("radius=%d, %d,%d,%d,%d (%d x %d)\n", radius, xStart, yStart, radius * 2, radius * 2, cvMat.cols, cvMat.rows);
        cv::Rect roi(xStart, yStart, radius * 2, radius * 2);
        return roi;
    }
    
    float Geometry::angleAtVertex(const cv::Point2f &vertex, const cv::Point2f &pointA, const cv::Point2f &pointB)
    {
        float angleBA = atan2f(vertex.y - pointA.y,
                               vertex.x - pointA.x);
        
        float angleBC = atan2f(vertex.y - pointB.y,
                               vertex.x - pointB.x);
        
        float angle = angleBA - angleBC;
        
        if (angle > 2.0f * M_PI)
            angle = angle - 2.0f * M_PI;
        if (angle < 0.0f)
            angle = angle + 2.0f * M_PI;
        
        return angle;
    }
    
    float Geometry::angleOfPoints(const cv::Point2f &pointA, const cv::Point2f &pointB) {
        double deltaX = pointA.x - pointB.x;
        double deltaY = pointA.y - pointB.y;
        double angleInRadians = atan2(deltaY, deltaX);
        if (angleInRadians < 0) angleInRadians = angleInRadians + M_2PI;
        return angleInRadians;
    }
    
    float Geometry::angleOfLine(const cv::Vec4f &line) {
        double deltaX = line[0] - line[2];
        double deltaY = line[1] - line[3];
        double angleInRadians = atan2(deltaY, deltaX);
        if (angleInRadians < 0) angleInRadians = angleInRadians + M_2PI;
        return angleInRadians;
    }
    
    cv::Point2f Geometry::rotatePoint(const cv::Point2f &point, const cv::Point2f &center, float theta)
    {
        float c = cosf(theta);
        float s = sinf(theta);
        
        return cv::Point2f(c * (point.x - center.x) - s * (point.y - center.y) + center.x,
                           s * (point.x - center.x) + c * (point.y - center.y) + center.y);
        
    }
    
    float Geometry::slope(const cv::Point2f &pointA, const cv::Point2f &pointB)
    {
        float xDiff = pointB.x - pointA.x;
        float yDiff = pointB.y - pointA.y;
        
        return yDiff / xDiff;
    }
    
    float Geometry::euclideanDistance(const cv::Vec4f &segment)
    {
        return sqrtf(euclideanDistanceSq(segment));
    }
    
    float Geometry::euclideanDistanceSq(const cv::Vec4f &segment)
    {
        float xDiff = segment[0] - segment[2];
        float yDiff = segment[1] - segment[3];
        
        return powf(xDiff, 2.0f) + powf(yDiff, 2.0f);
    }
    
    float Geometry::euclideanDistance(const cv::Point2f &pointA, const cv::Point2f &pointB)
    {
        return sqrtf(euclideanDistanceSq(pointA, pointB));
    }
    
    float Geometry::euclideanDistanceSq(const cv::Point2f &pointA, const cv::Point2f &pointB)
    {
        float xDiff = pointB.x - pointA.x;
        float yDiff = pointB.y - pointA.y;
        
        return powf(xDiff, 2.0f) + powf(yDiff, 2.0f);
    }
    
    float Geometry::manhattanDistance(const cv::Point2f &pointA, const cv::Point2f &pointB)
    {
        return sqrtf(manhattanDistanceSq(pointA, pointB));
    }
    
    float Geometry::manhattanDistanceSq(const cv::Point2f &pointA, const cv::Point2f &pointB)
    {
        float xDist = pointA.x - pointB.x;
        float yDist = pointA.y - pointB.y;
        
        return fabs(xDist) + fabs(yDist);
    }
    
    float Geometry::vectorMagnitude(const cv::Point2f &point)
    {
        return sqrt(point.x * point.x + point.y * point.y);
    }
    
    float Geometry::euclideanDistance(const Eigen::Vector3f &pointA, const Eigen::Vector3f &pointB) {
        return (pointB-pointA).lpNorm<2>();
    }
    
    float Geometry::manhattanDistance(const Eigen::Vector3f &pointA, const Eigen::Vector3f &pointB) {
        return (pointB-pointA).lpNorm<1>();
    }
    
    cv::Point2f Geometry::midpoint(const cv::Point2f &pointA, const cv::Point2f &pointB)
    {
        cv::Point result;
        
        result.x = (pointA.x + pointB.x) / 2.0f;
        result.y = (pointA.y + pointB.y) / 2.0f;
        return result;
    }
    
    cv::Point2f Geometry::pointOnLine(const cv::Point2f &startPoint, const cv::Point2f &endPoint, float percentageToEndpoint)
    {
        cv::Point result;
        float oneMinus = (1.0f - percentageToEndpoint);
        
        result.x = (oneMinus * startPoint.x + percentageToEndpoint * endPoint.x);
        result.y = (oneMinus * startPoint.y + percentageToEndpoint * endPoint.y);
        
        return result;
    }
    
    std::vector<cv::Point2f> Geometry::generatePointsOnLine(const cv::Point2f &startPoint, const cv::Point2f &endPoint, int numPoints)
    {
        std::vector<cv::Point2f> points;
        
        for (int i=0; i<numPoints; i++) {
            float percentageToEndpoint = float(i) / float(numPoints);
            float oneMinus = (1.0f - percentageToEndpoint);
            
            cv::Point2f result;
            result.x = (oneMinus * startPoint.x + percentageToEndpoint * endPoint.x);
            result.y = (oneMinus * startPoint.y + percentageToEndpoint * endPoint.y);
            
            points.push_back(result);
        }
        
        return points;
    }
    
    bool Geometry::isWithinSize(const cv::Point2f &point, const cv::Size &size)
    {
        if (point.x < 0 || point.y < 0
            || point.x >= size.width
            || point.y >= size.height)
            return false;
        return true;
    }
    
    bool Geometry::isWithinRect(const cv::Point2f &point, const cv::Rect &rect)
    {
        if (point.x < rect.x || point.y < rect.y
            || point.x > rect.width + rect.x
            || point.y > rect.height + rect.y)
            return false;
        cv::Scalar test;
        return true;
    }
    
    void Geometry::cartesianToPolar(const cv::Point2f &point, float *radius, float *radians)
    {
        *radius = (double)hypotf((float)point.x, (float)point.y);                       //radius /* better than sqrt(x*x+y*y) */
        *radians = (double) atan2f((float)point.y, (float)point.x);      //theta
    }
    
    cv::Point2f Geometry::polarToCartesian(float radius, float radians)
    {
        cv::Point2f cartesian;
        
        cartesian.x = radius * cosf(radians);
        cartesian.y = radius * sinf(radians);
        return cartesian;
    }
    
    float Geometry::radiansDifference(double angleA, double angleB)
    {
        return MIN((2.0f * M_PI) - fabs(angleA - angleB), fabs(angleA - angleB));
    }
    
    float Geometry::degreesDifference(double degreesA, double degreesB)
    {
        return toDegrees(radiansDifference(toRadians(degreesA), toRadians(degreesB)));
    }
    
    /*
     
     def point_segment_distance(px, py, x1, y1, x2, y2):
     dx = x2 - x1
     dy = y2 - y1
     if dx == dy == 0:  # the segment's just a point
     return math.hypot(px - x1, py - y1)
     
     # Calculate the t that minimizes the distance.
     t = ((px - x1) * dx + (py - y1) * dy) / (dx * dx + dy * dy)
     
     # See if this represents one of the segment's
     # end points or a point in the middle.
     if t < 0:
     dx = px - x1
     dy = py - y1
     elif t > 1:
     dx = px - x2
     dy = py - y2
     else:
     near_x = x1 + t * dx
     near_y = y1 + t * dy
     dx = px - near_x
     dy = py - near_y
     
     return math.hypot(dx, dy)
     
     */
    
    double pointSegmentDistance(const cv::Point& p0, const cv::Point& p1, const cv::Point& p2) {
        double dx = p2.x - p1.x;
        double dy = p2.y - p1.y;
        
        if (dx == 0 && dy == 0) // the segment's just a point
        {
            return hypot(p0.x - p1.x, p0.y - p1.y);
        }
        
        //Calculate the t that minimizes the distance.
        double t = ((p0.x - p1.x) * dx + (p0.y - p1.y) * dy) / (dx * dx + dy * dy);
        
        //See if this represents one of the segment's
        //end points or a point in the middle.
        if (t < 0) {
            dx = p0.x - p1.x;
            dy = p0.y - p1.y;
        }
        else if (t > 1) {
            dx = p0.x - p2.x;
            dy = p0.y - p2.y;
        }
        else {
            double near_x = p1.x + t * dx;
            double near_y = p1.y + t * dy;
            dx = p0.x - near_x;
            dy = p0.y - near_y;
        }
        
        return hypot(dx, dy);
    }
    
    //x11, y11, x12, y12, x21, y21, x22, y22
    double Geometry::distanceBetweenSegments(const cv::Point2f& p11, const cv::Point2f& p12,
                                             const cv::Point2f& p21, const cv::Point2f& p22)
    {
        cv::Point2f intersection;
        if (segmentsIntersect(p11, p12, p21, p22, intersection)){
            return 0;
        }
        //find closest two points
        double shortestDistance = INT_MAX;
        
        //distances.append(point_segment_distance(x11, y11, x21, y21, x22, y22))
        { double distance = pointSegmentDistance(p11, p21, p22);
            if (distance < shortestDistance) shortestDistance = distance;}
        
        //distances.append(point_segment_distance(x12, y12, x21, y21, x22, y22))
        { double distance = pointSegmentDistance(p12, p21, p22);
            if (distance < shortestDistance) shortestDistance = distance;}
        
        //distances.append(point_segment_distance(x21, y21, x11, y11, x12, y12))
        { double distance = pointSegmentDistance(p21, p11, p12);
            if (distance < shortestDistance) shortestDistance = distance;}
        
        //distances.append(point_segment_distance(x22, y22, x11, y11, x12, y12))
        { double distance = pointSegmentDistance(p22, p11, p12);
            if (distance < shortestDistance) shortestDistance = distance;}
        
        return shortestDistance;
    }
    
    inline double dot(const cv::Point& a,const cv::Point& b) { return (a.x*b.x) + (a.y*b.y); }
    inline double perpDot(const cv::Point& a,const cv::Point& b) { return (a.y*b.x) - (a.x*b.y); }
    
    bool Geometry::segmentsIntersect(const cv::Point2f& p0, const cv::Point2f& p1,
                                     const cv::Point2f& p2, const cv::Point2f& p3,
                                     cv::Point2f& intersection)
    {
        float s02_x, s02_y, s10_x, s10_y, s32_x, s32_y, s_numer, t_numer, denom, t;
        s10_x = p1.x - p0.x;
        s10_y = p1.y - p0.y;
        s32_x = p3.x - p2.x;
        s32_y = p3.y - p2.y;
        
        denom = s10_x * s32_y - s32_x * s10_y;
        if (denom == 0)
            return false; // Collinear
        bool denomPositive = denom > 0;
        
        s02_x = p0.x - p2.x;
        s02_y = p0.y - p2.y;
        s_numer = s10_x * s02_y - s10_y * s02_x;
        if ((s_numer < 0) == denomPositive)
            return false; // No collision
        
        t_numer = s32_x * s02_y - s32_y * s02_x;
        if ((t_numer < 0) == denomPositive)
            return false; // No collision
        
        if (((s_numer > denom) == denomPositive) || ((t_numer > denom) == denomPositive))
            return false; // No collision
        // Collision detected
        t = t_numer / denom;
        
        
        intersection.x = p0.x + (t * s10_x);
        intersection.y = p0.y + (t * s10_y);
        
        return true;
    }
    
    void Geometry::getPointsOnSegment(const cv::Point &point1, const cv::Point &point2, std::vector<cv::Point> &points) {
        
        int x = point1.x;
        int y = point1.y;
        int x2 = point2.x;
        int y2 = point2.y;
        
        
        int w = x2 - x;
        int h = y2 - y;
        int dx1 = 0, dy1 = 0, dx2 = 0, dy2 = 0 ;
        if (w<0) dx1 = -1 ; else if (w>0) dx1 = 1 ;
        if (h<0) dy1 = -1 ; else if (h>0) dy1 = 1 ;
        if (w<0) dx2 = -1 ; else if (w>0) dx2 = 1 ;
        int longest = abs(w) ;
        int shortest = abs(h) ;
        if (!(longest>shortest)) {
            longest = abs(h) ;
            shortest = abs(w) ;
            if (h<0) dy2 = -1 ; else if (h>0) dy2 = 1 ;
            dx2 = 0 ;
        }
        int numerator = longest >> 1 ;
        for (int i=0;i<=longest;i++) {
            points.push_back(cv::Point(x, y));
            numerator += shortest ;
            if (!(numerator<longest)) {
                numerator -= longest ;
                x += dx1 ;
                y += dy1 ;
            } else {
                x += dx2 ;
                y += dy2 ;
            }
        }
    }
    
    void Geometry::getPerpendicularPointsForLine(const cv::Point& p1, const cv::Point& p2, int atDistance,
                                                 cv::Point& out1, cv::Point& out2) {
        cv::Point2f M = cv::Point2f((p1.x + p2.x) / 2.0f, (p1.y + p2.y) / 2.0f);
        cv::Point2f p = cv::Point2f((p1.x - p2.x), (p1.y - p2.y));
        cv::Point2f n = cv::Point2f(-p.y, p.x);
        
        int norm_length = sqrt((n.x * n.x) + (n.y * n.y));
        n.x /= norm_length;
        n.y /= norm_length;
        
        out1 = cv::Point(M.x + atDistance * n.x, M.y + atDistance * n.y);
        out2 = cv::Point(M.x - atDistance * n.x, M.y - atDistance * n.y);
    }
    
    void Geometry::getParallelLines(const cv::Point& p1, const cv::Point& p2, int atDistance,
                                    cv::Point& outLine11, cv::Point& outLine12,
                                    cv::Point& outLine21, cv::Point& outLine22) {
        cv::Point2f p = cv::Point2f((p1.x - p2.x), (p1.y - p2.y));
        cv::Point2f n = cv::Point2f(-p.y, p.x);
        
        int norm_length = sqrt((n.x * n.x) + (n.y * n.y));
        n.x /= norm_length;
        n.y /= norm_length;
        
        outLine11 = cv::Point(p1.x + atDistance * n.x, p1.y + atDistance * n.y);
        outLine12 = cv::Point(p2.x + atDistance * n.x, p2.y + atDistance * n.y);
        
        outLine21 = cv::Point(p1.x - atDistance * n.x, p1.y - atDistance * n.y);
        outLine22 = cv::Point(p2.x - atDistance * n.x, p2.y - atDistance * n.y);
    }
    
    cv::Vec4f Geometry::scaleLineSegment(const cv::Vec4f &line, float scale) {
        cv::Vec4f extendedLine;
        float dx = line[0] - line[2];
        float dy = line[1] - line[3];
        
        float dXScale = dx * scale;
        float dYScale = dy * scale;
        
        extendedLine[0] = line[0] - dXScale;
        extendedLine[1] = line[1] - dYScale;
        extendedLine[2] = line[2] + dXScale;
        extendedLine[3] = line[3] + dYScale;
        
        return extendedLine;
    }
    
    void Geometry::scaleLineSegment(cv::Point2f &pointA, cv::Point2f &pointB, float scale) {
        float dx = pointA.x - pointB.x;
        float dy = pointA.y - pointB.y;
        
        float dXScale = dx * scale;
        float dYScale = dy * scale;
        
        pointA.x = pointA.x - dXScale;
        pointA.y = pointA.y - dYScale;
        pointB.x = pointB.x + dXScale;
        pointB.y = pointB.y + dYScale;
    }
    
    //    vec2d calculate_perp_point(vec2d A, vec2d B, float distance)
    //    {
    //        vec2d M = (A + B) / 2;
    //        vec2d p = A - B;
    //        vec2d n = (-p.y, p.x);
    //        int norm_length = sqrt((n.x * n.x) + (n.y * n.y));
    //        n.x /= norm_length;
    //        n.y /= norm_length;
    //        return (M + (distance * n));
    //    }
    
    
    cv::Point2f Geometry::normalizePoint(const cv::Point &point, const cv::Size& size) {
        return cv::Point2f(float(point.x) / float(size.width), float(point.y) / float(size.height));
    }
    
    cv::Point Geometry::denormalizePoint(const cv::Point2f &point, const cv::Size& size) {
        return cv::Point(point.x * float(size.width), point.y * float(size.height));
    }
    
    cv::Point2f Geometry::rotatePoint90n(const cv::Point2f &point, int rotation, const cv::Size2f& viewSize) {
        cv::Point2f rotated;
        
        if (rotation < 0) rotation = 360 - rotation;
        rotation = (rotation % 360);
        
        switch (rotation) {
            case 0:
                rotated = point;
                break;
            case 90:
                rotated = cv::Point2f(viewSize.height - point.y, point.x);
                break;
            case 180:
                rotated = cv::Point2f(viewSize.width - point.x, viewSize.height - point.y);
                break;
            case 270:
                rotated = cv::Point2f(point.y, viewSize.width - point.x);
                break;
        }
        
        
        return rotated;
    }
    
    
    bool Geometry::getPlaneIntersection(const Eigen::Vector3f &planeNormal, const Eigen::Vector3f &planeCoordinate,
                                        Eigen::Vector3f &intersection, const Eigen::Vector3f &rayDirection, const Eigen::Vector3f &rayOrigin) {
        
        float denom = planeNormal.dot(rayDirection);
        
        if (fabs(denom) <= FLT_EPSILON) {
            return false; // No intersection, the line is parallel to the plane
        }
        
        Eigen::ParametrizedLine<float,3> pline = Eigen::ParametrizedLine<float,3>::Through(rayOrigin, rayDirection);
        
        Eigen::Hyperplane<float,3> focalPlane = Eigen::Hyperplane<float,3>(planeNormal, planeCoordinate);
        
        intersection = pline.intersectionPoint(focalPlane);
        
        return true;
    }
    
    bool isLineEqual(const cv::Vec4f& _l1, const cv::Vec4f& _l2)
    {
        cv::Vec4f l1(_l1), l2(_l2);
        
        float length1 = sqrtf((l1[2] - l1[0])*(l1[2] - l1[0]) + (l1[3] - l1[1])*(l1[3] - l1[1]));
        float length2 = sqrtf((l2[2] - l2[0])*(l2[2] - l2[0]) + (l2[3] - l2[1])*(l2[3] - l2[1]));
        
        float product = (l1[2] - l1[0])*(l2[2] - l2[0]) + (l1[3] - l1[1])*(l2[3] - l2[1]);
        
        float colinear = fabs(product / (length1 * length2));
        if (colinear < cos(CV_PI / 30))
            return false;
        
        float mx1 = (l1[0] + l1[2]) * 0.5f;
        float mx2 = (l2[0] + l2[2]) * 0.5f;
        
        float my1 = (l1[1] + l1[3]) * 0.5f;
        float my2 = (l2[1] + l2[3]) * 0.5f;
        float dist = sqrtf((mx1 - mx2)*(mx1 - mx2) + (my1 - my2)*(my1 - my2));
        
        //penalize long lines
        //float score = tanDiff / sqrtf(dist);
        
        float maxLength = std::max(length1, length2);
        if (dist > maxLength * 0.4 && dist < maxLength * 2.0) {
            //find extensions
            if (colinear > cos(CV_PI / 60)) {
                float partLength = 0.25 * maxLength;
                
                float xDiffAA = l1[0] - l2[0];
                float yDiffAA = l1[1] - l2[1];
                float lenAA = sqrtf(xDiffAA * xDiffAA + yDiffAA * yDiffAA);
                
                if (lenAA < partLength) return true;
                
                float xDiffBB = l1[2] - l2[2];
                float yDiffBB = l1[3] - l2[3];
                float lenBB = sqrtf(xDiffBB * xDiffBB + yDiffBB * yDiffBB);
                
                if (lenBB < partLength) return true;
                
                float xDiffAB = l1[0] - l2[2];
                float yDiffAB = l1[1] - l2[3];
                float lenAB = sqrtf(xDiffAB * xDiffAB + yDiffAB * yDiffAB);
                
                if (lenAB < partLength) return true;
            }
            return false;
        }
        
        return dist / maxLength < 0.1;
    }
    
    void Geometry::groupLines(const std::vector<cv::Vec4f> &detectedLines, std::vector<cv::Vec4f> &groupedLines, int minLineLength) {
        std::vector<cv::Vec4f>filtered;
        filtered.reserve(2 * detectedLines.size());
        float minLengthSQ = minLineLength * minLineLength;
        for (const auto &line : detectedLines) {
            if (!minLineLength || Geometry::euclideanDistanceSq(line) > minLengthSQ) {
                filtered.push_back(line);
            }
        }
        
        std::vector<int> labels;
        int numberOfLines = cv::partition(filtered, labels, isLineEqual);
        
        std::vector<std::vector<cv::Point2f>>points(numberOfLines);
        
        for (int i=0; i<filtered.size(); i++) {
            int label = labels[i];
            const auto &line = filtered[i];
            points[label].push_back(cv::Point2f(line[0], line[1]));
            points[label].push_back(cv::Point2f(line[2], line[3]));
        }
        
        std::vector<cv::Vec4f> newLines;
        newLines.reserve(points.size() * 2);
        
        for (int i=0; i<numberOfLines; i++) {
            const std::vector<cv::Point2f> &linePoints = points[i];
            if (linePoints.size() > 2) {
                cv::RotatedRect box = cv::minAreaRect(linePoints);
                float linearity = fmin(box.size.width, box.size.height) / fmax(box.size.width, box.size.height);
                //if not a rectangle of only 10 percent width, ignore this grouping
                if (linearity > 0.1) {
                    //add them all back, all parallel
                    for (int j=0; j<linePoints.size(); j+=2) {
                        newLines.push_back(cv::Vec4f(linePoints[j].x, linePoints[j].y, linePoints[j+1].x, linePoints[j+1].y));
                    }
                } else {
                    cv::Point2f points[4];
                    box.points(points); //The order is bottomLeft, topLeft, topRight, bottomRight.
                    cv::Point2f pointA, pointB;
                    if (box.size.width < box.size.height) {
                        pointA = (points[0] + points[3]) / 2.0f;
                        pointB = (points[1] + points[2]) / 2.0f;
                    } else {
                        pointA = (points[0] + points[1]) / 2.0f;
                        pointB = (points[2] + points[3]) / 2.0f;
                    }
                    newLines.push_back(cv::Vec4f(pointA.x, pointA.y, pointB.x, pointB.y));
                }
            } else {
                newLines.push_back(cv::Vec4f(linePoints[0].x, linePoints[0].y, linePoints[1].x, linePoints[1].y));
            }
        }
        
        groupedLines = newLines;
    }
}
