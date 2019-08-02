//
//  Drawing.cpp
//  Cambrian
//
//  Created by Joel Teply on 6/15/12.
//
//

#include "Drawing.h"
#include "BezierCurve.h"

namespace imaging {
    
    static void drawEndcap(cv::Mat &img, const cv::Point &pointA, const cv::Point &pointB,
                           const cv::Scalar &color, int thickness, bool rounded);
    
    uchar Drawing::getSingleChannelPixelAt(const cv::Point &point, const cv::Mat &img)
    {
        return getPixel1(point.x, ROW_PTR(img, point.y));
    }
    
    cv::Vec3b Drawing::getTriChannelPixelAt(const cv::Point &point, const cv::Mat &img)
    {
        return getPixel3(point.x, ROW_PTR(img, point.y));
    }
    
    cv::Vec4b Drawing::getQuadChannelPixelAt(const cv::Point &point, const cv::Mat &img)
    {
        return getPixel4(point.x, ROW_PTR(img, point.y));
    }
    
    void Drawing::setSingleChannelPixelAt(const cv::Point &point, cv::Mat &img, uchar color)
    {
        uchar* rowPtr = ROW_PTR(img, point.y);
        
        rowPtr[point.x] = color;
    }
    
    void Drawing::setTriChannelPixelAt(const cv::Point &point, cv::Mat &img, const cv::Scalar &color)
    {
        uchar* rowPtr = ROW_PTR(img, point.y);
        int x3 = point.x * 3;
        
        rowPtr[x3] = color.val[0];
        rowPtr[x3 + 1] = color.val[1];
        rowPtr[x3 + 2] = color.val[2];
    }
    
    void Drawing::setQuadChannelPixelAt(const cv::Point &point, cv::Mat &img, const cv::Scalar &color)
    {
        uchar* rowPtr = ROW_PTR(img, point.y);
        int x4 = point.x * 4;
        
        rowPtr[x4] = color.val[0];
        rowPtr[x4 + 1] = color.val[1];
        rowPtr[x4 + 2] = color.val[2];
        rowPtr[x4 + 3] = color.val[3];
    }
    
    void Drawing::setPixelAt(const cv::Point &point, cv::Mat &img, const cv::Scalar &color)
    {
        int channels = img.channels();
        
        if (channels == 4)
            setQuadChannelPixelAt(point, img, color);
        else if (channels == 3)
            setTriChannelPixelAt(point, img, color);
        else
            setSingleChannelPixelAt(point, img, color.val[0]);
    }
    
    cv::Scalar Drawing::getPixelAt(const cv::Point &point, const cv::Mat &img)
    {
        int channels = img.channels();
        
        if (channels == 4)
            return cv::Scalar(getQuadChannelPixelAt(point, img));
        else if (channels == 3)
            return cv::Scalar(getTriChannelPixelAt(point, img));
        else
            return cv::Scalar(getSingleChannelPixelAt(point, img));
    }
    
    void Drawing::setOutputArray(cv::OutputArray outputArray, const cv::Scalar &scalar, int channels)
    {
        if (!outputArray.fixedSize() )
            outputArray.create(channels, 1, CV_64F, -1, true);
        
        cv::Mat dst = outputArray.getMat();
        
        double* dptr = dst.ptr<double>();
        for (int k = 0; k < channels; k++ )
            dptr[k] = scalar.val[k];
    }
    
    std::vector<cv::Point2f> Drawing::createBezierCurve(const std::vector<cv::Point2f> &inPoints, float minDistance)
    {
        //compute
        double length = cv::arcLength(inPoints, false);
        
        if (minDistance < 1) {
            minDistance = 1;
        }
        int totalPoints = int(length / minDistance);
        
        if (totalPoints < 5) {
            totalPoints = 5;
        }
        
        return createBezierCurve(inPoints, totalPoints);
    }
    
    
    std::vector<cv::Point2f> Drawing::createBezierCurve(const std::vector<cv::Point2f> &inPoints, int totalPoints)
    {
        std::vector<cv::Point2f>outPoints;
        imaging::BezierCurve::Bezier2D(inPoints, totalPoints, outPoints);
        
        bool isValid = false;
        
        if (outPoints.size() == totalPoints) {
            cv::Point2f testPoint = outPoints[0];
            //validate eliminate nan
            isValid = (testPoint.x == testPoint.x);
        }
        
        if (isValid) {
            return outPoints;
        } else {
            return inPoints;
        }
    }

    void Drawing::drawCurve(cv::Mat &img, const std::vector<DrawPoint> &points, const cv::Scalar &color, bool rounded)
    {
        size_t pointsSize = points.size();
        
        if (pointsSize < 2) return;
        
        if (points[0].radius < 6) rounded = false;
        
        //Draw line start
        drawEndcap(img, points[0].point, points[1].point, color, points[0].radius, rounded);
        
        
        //draw line(s)
        for (int i=0; i<pointsSize-1; i++) {
            DrawPoint pointA = points[i];
            DrawPoint pointB = points[i+1];
            int thickness = pointA.radius + pointB.radius;
            cv::line(img, pointA.point, pointB.point, color, thickness);
        }
        
        drawEndcap(img, points[pointsSize - 1].point, points[pointsSize - 2].point, color, points[pointsSize - 1].radius, rounded);
    }
    
    static void drawEndcap(cv::Mat &img, const cv::Point &pointA, const cv::Point &pointB,
                           const cv::Scalar &color, int thickness, bool rounded)
    {
        float radius = thickness/2;
        if (rounded) {
            cv::circle(img, pointA, radius, color, CV_FILLED);
        } else {
            float boxRadius = radius * sqrt(2);
            float radians45 = M_PI / 4;
            float deltaX = pointB.x - pointA.x;
            float deltaY = pointB.y - pointA.y;
            float angle = atanf(deltaY / deltaX);
            
            cv::Point endcapPoints[4];
            endcapPoints[0] = cv::Point(pointA.x + boxRadius * cosf(angle - radians45),
                                        pointA.y + boxRadius * sinf(angle - radians45));
            
            endcapPoints[1] = cv::Point(pointA.x + boxRadius * cosf(angle + radians45),
                                        pointA.y + boxRadius * sinf(angle + radians45));
            
            endcapPoints[2] = cv::Point(pointA.x + boxRadius * cosf(angle + 3 * radians45),
                                        pointA.y + boxRadius * sinf(angle + 3 * radians45));
            
            endcapPoints[3] = cv::Point(pointA.x + boxRadius * cosf(angle - 3 * radians45),
                                        pointA.y + boxRadius * sinf(angle - 3 * radians45));

            cv::fillConvexPoly(img, endcapPoints, 4, color);
        }
    }
}
