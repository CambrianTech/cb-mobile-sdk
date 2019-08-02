//
//  BezierCurve.h
//  ObjectFinder
//
//  Created by Joel Teply on 6/10/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef ObjectFinder_BezierCurve_h
#define ObjectFinder_BezierCurve_h

//#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <cambrian.h>

namespace imaging {
    class BezierCurveImp;                    // forward declaration of Pimpl
    
    class DLL_LOCAL BezierCurve
    {
    public:
        int test;
        
        BezierCurve ();
        ~BezierCurve();
        
        BezierCurve( const BezierCurve &rhs );   // undefined for simplicity
        BezierCurve& operator=( BezierCurve );
        
        static void Bezier2D(std::vector<cv::Point2f> inPoints, int numPoints, std::vector<cv::Point2f> &outPoints);
        
    private:
        BezierCurveImp *_impl;              // the implementation
    };
    
};

#endif
