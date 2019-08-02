//
//  LineSegment.hpp
//  Cambrian
//
//  Created by Joel Teply on 9/21/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#ifndef LineSegment_hpp
#define LineSegment_hpp

#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <cambrian.h>

namespace imaging {

    class DLL_LOCAL LineSegment {
        
    public:

        cv::Point2f point0;
        cv::Point2f point1;
        cv::Point2f midpoint;
        
        int64_t point0Terminated = 0;
        int64_t point1Terminated = 0;
        
        double angle = 0;
        double length = 0;
        int64_t index = 0;
        int64_t userIndex = 0;
        bool invalid = false;
        int confirmations = 0;
        int age = 0;
        int64_t udid = 0;
        
        LineSegment() {};
        
        LineSegment(cv::Vec4f pts) : LineSegment(cv::Point2f(pts[0], pts[1]), cv::Point2f(pts[2], pts[3])) {}
        
        LineSegment(cv::Point2f pA, cv::Point2f pB, int64_t i=0);

        LineSegment(const LineSegment &lineA, const LineSegment &lineB, int64_t i=0);

        cv::Point2f& operator[] (const int index) { return index ? point1 : point0;}
        const cv::Point2f& operator[] (const int index) const { return index ? point1 : point0;}
        
        void recalculate();
        
    };
}

#endif /* LineSegment_hpp */
