//
//
//  Created by Joel Teply on 6/14/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef __Geometry__
#define __Geometry__

#include <opencv2/opencv.hpp>
#include <cambrian.h>
#include <Eigen/Geometry>

#define toDegrees(radians) ((180.0f * radians) / M_PI)
#define toRadians(degrees) ((M_PI * degrees) / 180.0f)

namespace imaging {
    class DLL_LOCAL Geometry
    {
    public:
        static float normalizeAngle(float angle);
        static float angleDifference(float angleA, float angleB);
        static cv::Rect rectAtPoint(const cv::Point &point, int radius, const cv::Mat &cvMat);
        
        static float angleAtVertex(const cv::Point2f &vertex, const cv::Point2f &pointA, const cv::Point2f &pointB);
        static float angleOfPoints(const cv::Point2f &pointA, const cv::Point2f &pointB);
        static float angleOfLine(const cv::Vec4f &line);
        
        static float slope(const cv::Point2f &pointA, const cv::Point2f &pointB);
        static void getPerpendicularPointsForLine(const cv::Point& p1, const cv::Point& p2, int atDistance,
                                                  cv::Point& out1, cv::Point& out2);
        static void getParallelLines(const cv::Point& p1, const cv::Point& p2, int atDistance,
                                     cv::Point& outLine11, cv::Point& outLine12,
                                     cv::Point& outLine21, cv::Point& outLine22);
        
        static float euclideanDistance(const cv::Point2f &pointA, const cv::Point2f &pointB);
        static float euclideanDistanceSq(const cv::Point2f &pointA, const cv::Point2f &pointB);
        static float manhattanDistance(const cv::Point2f &pointA, const cv::Point2f &pointB);
        static float manhattanDistanceSq(const cv::Point2f &pointA, const cv::Point2f &pointB);
        
        static float euclideanDistance(const cv::Vec4f &segment);
        static float euclideanDistanceSq(const cv::Vec4f &segment);
        static float euclideanDistance(const Eigen::Vector3f &pointA, const Eigen::Vector3f &pointB);
        static float manhattanDistance(const Eigen::Vector3f &pointA, const Eigen::Vector3f &pointB);
        
        static float vectorMagnitude(const cv::Point2f &point);
        
        static cv::Point2f midpoint(const cv::Point2f &pointA, const cv::Point2f &pointB);
        static cv::Point2f pointOnLine(const cv::Point2f &startPoint, const cv::Point2f &endPoint, float percentageToEndpoint);
        static std::vector<cv::Point2f> generatePointsOnLine(const cv::Point2f &startPoint, const cv::Point2f &endPoint, int numPoints);
        
        static bool isWithinSize(const cv::Point2f &point, const cv::Size &size);
        static bool isWithinRect(const cv::Point2f &point, const cv::Rect &rect);
        
        static void cartesianToPolar(const cv::Point2f &point, float *radius, float *radians);
        static cv::Point2f polarToCartesian(float radius, float degrees);
        
        static float radiansDifference(double angleA, double angleB);
        static float degreesDifference(double degreesA, double degreesB);
        
        static double distanceBetweenSegments(const cv::Point2f& A1, const cv::Point2f& A2,
                                              const cv::Point2f& B1, const cv::Point2f& B2);
        
        static bool segmentsIntersect(const cv::Point2f& p0, const cv::Point2f& p1,
                                      const cv::Point2f& p2, const cv::Point2f& p3,
                                      cv::Point2f& intersection);
        
        static void getPointsOnSegment(const cv::Point &point1, const cv::Point &point2, std::vector<cv::Point> &points);
        
        static cv::Vec4f scaleLineSegment(const cv::Vec4f &line, float amount);
        static void scaleLineSegment(cv::Point2f &pointA, cv::Point2f &pointB, float amount);
        
        static cv::Point2f rotatePoint(const cv::Point2f &point, const cv::Point2f &center, float theta);
        
        
        static cv::Point2f normalizePoint(const cv::Point &point, const cv::Size& size);
        
        static cv::Point denormalizePoint(const cv::Point2f &point, const cv::Size& size);
        
        static cv::Point2f rotatePoint90n(const cv::Point2f &point, int rotation, const cv::Size2f& viewSize=cv::Size2f(1,1));
        
        static bool getPlaneIntersection(const Eigen::Vector3f &planeNormal, const Eigen::Vector3f &planeCenter,
                                         Eigen::Vector3f &intersection, const Eigen::Vector3f &rayDirection,
                                         const Eigen::Vector3f &rayOrigin);
        
        static void groupLines(const std::vector<cv::Vec4f> &detectedLines, std::vector<cv::Vec4f> &groupedLines, int minLineLength=0);
        
        static float lineAngleDifference(const cv::Vec4f &lineA, const cv::Vec4f &lineB);
        
        static float linePerpendicularity(const cv::Vec4f &lineA, const cv::Vec4f &lineB);
        static float lineParallelism(const cv::Vec4f &lineA, const cv::Vec4f &lineB);
    private:
        static float _angleDifference(float angleA, float angleB);
    };
};
#endif
