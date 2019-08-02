//
//  LineSegment.cpp
//  Cambrian
//
//  Created by Joel Teply on 9/21/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#include "LineSegment.hpp"

#include <imaging/Geometry.h>

static int64_t udidMax = 0;

namespace imaging {
    
    LineSegment::LineSegment(cv::Point2f pA, cv::Point2f pB, int64_t i)
    {
        invalid = false;
        index = i;
        
        point0 = pA;
        point1 = pB;
        
        recalculate();
    }
    
    LineSegment::LineSegment(const LineSegment &lineA, const LineSegment &lineB, int64_t i)
    {
        invalid = false;
        index = i;
        
        point0 = Geometry::euclideanDistanceSq(lineA.midpoint, lineB[0])
        > Geometry::euclideanDistanceSq(lineA.midpoint, lineB[1]) ? lineB[0] : lineB[1];
        
        point1 = Geometry::euclideanDistanceSq(lineB.midpoint, lineA[0])
        > Geometry::euclideanDistanceSq(lineB.midpoint, lineA[1]) ? lineA[0] : lineA[1];
        
        recalculate();
    }
    
    void LineSegment::recalculate() {
        length = Geometry::euclideanDistance(point0, point1);
        angle = Geometry::angleOfPoints(point0, point1);
        midpoint = Geometry::midpoint(point0, point1);
        if (!udid) {
            udidMax ++;
            udid = udidMax;
        }
    }
    
}
