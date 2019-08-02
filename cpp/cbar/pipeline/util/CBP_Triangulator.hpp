//
//  CBP_Triangulator.hpp
//  Cambrian
//
//  Created by Joel Teply on 2/8/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#ifndef CBP_Triangulator_hpp
#define CBP_Triangulator_hpp

#include <Eigen/Geometry>
#include <Eigen/QR>
#include <Eigen/LU>
#include <opencv2/opencv.hpp>

namespace cbpipe {
    
    class CBP_Triangulator
    {
        struct Impl;
        std::unique_ptr<Impl> p_;
    public:
        CBP_Triangulator();
        ~CBP_Triangulator();
        
        void setBoundaryContour(const std::vector<cv::Point>& contour);
        void addHole(const std::vector<cv::Point>& contour);
        
        double maxArea() const;
        void setMaxArea(double area);
        
        double minRadians() const;
        void setMinRadians(double radians);
        
        bool debuggingEnabled() const;
        void setDebuggingEnabled(bool enabled);
        
        cv::Rect2f triangulate(std::vector<Eigen::Vector3f> &vertices, std::vector<int> &indices,
                               const Eigen::Vector3f &offset, float scale, size_t indexOffset=0);
    };
    
};

#endif /* CBP_Triangulator_hpp */
