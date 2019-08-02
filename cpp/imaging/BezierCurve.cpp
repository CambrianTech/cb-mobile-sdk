//
//  BezierCurve.mm
//  ObjectFinder
//
//  Created by Joel Teply on 6/10/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>

#include "BezierCurve.h"

namespace imaging {
    class BezierCurveImp
    {
    public:
        
        std::vector<double> FactorialLookup;
        
        // create lookup table for fast factorial calculation
        void CreateFactorialTable()
        {
            // fill untill n=32. The rest is too high to represent
            std::vector<double>a;
            a.push_back(1.0);
            a.push_back(1.0);
            a.push_back(2.0);
            a.push_back(6.0);
            a.push_back(24.0);
            a.push_back(120.0);
            a.push_back(720.0);
            a.push_back(5040.0);
            a.push_back(40320.0);
            a.push_back(362880.0);
            a.push_back(3628800.0);
            a.push_back(39916800.0);
            a.push_back(479001600.0);
            a.push_back(6227020800.0);
            a.push_back(87178291200.0);
            a.push_back(1307674368000.0);
            a.push_back(20922789888000.0);
            a.push_back(355687428096000.0);
            a.push_back(6402373705728000.0);
            a.push_back(121645100408832000.0);
            a.push_back(2432902008176640000.0);
            a.push_back(51090942171709440000.0);
            a.push_back(1124000727777607680000.0);
            a.push_back(25852016738884976640000.0);
            a.push_back(620448401733239439360000.0);
            a.push_back(15511210043330985984000000.0);
            a.push_back(403291461126605635584000000.0);
            a.push_back(10888869450418352160768000000.0);
            a.push_back(304888344611713860501504000000.0);
            a.push_back(8841761993739701954543616000000.0);
            a.push_back(265252859812191058636308480000000.0);
            a.push_back(8222838654177922817725562880000000.0);
            a.push_back(263130836933693530167218012160000000.0);
            this->FactorialLookup = a;
        }
        
        // just check if n is appropriate, then return the result
        double factorial(int n)
        {
            //if (n < 0) { throw new Exception("n is less than 0"); }
            //if (n > 32) { throw new Exception("n is greater than 32"); }
            return this->FactorialLookup[n]; /* returns the value n! as a SUMORealing point number */
        }
        
        double Ni(int n, int i)
        {
            double ni;
            double a1 = factorial(n);
            double a2 = factorial(i);
            double a3 = factorial(n - i);
            
            ni =  a1 / (a2 * a3);
            return ni;
        }
        
        // Calculate Bernstein basis
        double Bernstein(int n, int i, double t)
        {
            double basis;
            double ti;      /* t^i */
            double tni;     /* (1 - t)^i */
            
            /* Prevent problems with pow */
            
            if (t == 0.0 && i == 0)
                ti = 1.0;
            else
                ti = pow(t, i);
            
            if (n == i && t == 1.0)
                tni = 1.0;
            else
                tni = pow((1 - t), (n - i));
            
            //Bernstein basis
            basis = Ni(n, i) * ti * tni;
            return basis;
        }
        
        void Bezier2D(std::vector<cv::Point2f> inPoints, int numPoints, std::vector<cv::Point2f> &outPoints)
        {
            int npts = (int) inPoints.size();
            int icount, jcount;
            double step, t;
            
            // Calculate points on curve
            
            icount = 0;
            t = 0;
            step = (double)1.0 / (numPoints - 1);
            
            //create numPoints total points
            for (int i1 = 0; i1 != numPoints; i1++)
            {
                if ((1.0 - t) < 5e-6)
                    t = 1.0;
                
                jcount = 0;
                
                cv::Point2f newPoint = cv::Point2f(0, 0);
                
                //go through all of the input points
                for (int i = 0; i != npts; i++)
                {
                    cv::Point2f inPoint = inPoints[jcount];
                    double basis = Bernstein(npts - 1, i, t);
                    newPoint.x += basis * inPoint.x;
                    newPoint.y += basis * inPoint.y;
                    jcount = jcount + 1;
                }
                
                outPoints.push_back(newPoint);
                
                icount++;
                t += step;
            }
        }
    };
    
#pragma mark - Public Methods
    
    static BezierCurve *curve = 0;
    
    BezierCurve::BezierCurve()  :  _impl( new BezierCurveImp() )
    {
        _impl->CreateFactorialTable();
    }
    
    BezierCurve::~BezierCurve()
    {
        delete  _impl;
    }
    
    void BezierCurve::Bezier2D(std::vector<cv::Point2f> inPoints, int numPoints, std::vector<cv::Point2f> &outPoints)
    {
        if (!curve) {
            curve = new BezierCurve();
        }
        
        return curve->_impl->Bezier2D(inPoints, numPoints, outPoints);
    }
    
}