//
//  CBP_AnalysisTypes.hpp
//  Cambrian
//
//  Created by Joel Teply on 7/12/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#ifndef CBP_AnalysisTypes_h
#define CBP_AnalysisTypes_h

namespace cbpipe {
    
    struct DLL_LOCAL deep_result {
        cbar::CBAR_VideoFramePtr frame;
        std::vector<cv::Mat> outputs;
    };
    
    struct plane {
        cbar::CBAR_VideoFramePtr frame;
        
        cv::Mat transform;
        cv::Rect transformBounds;
        Eigen::Vector3f cameraPosition;
        
        Eigen::Matrix4f worldTransform;
        
        Eigen::Vector3f center;
        Eigen::Vector3f normal;
        
        cv::Rect roi3D;
        
        //contour_record(int64_t _frameIndex) : frameIndex(_frameIndex) {}
        //contour_record(contour_record const&) = default;
    };
    
    struct plane_contour : plane {
        cbar::CBAR_VideoFramePtr frame;
        
        cv::Mat prob2D;
        cv::Mat probTransformed;
        cv::Mat imageTransformed;
        cv::Mat boundsTransformed;
        
        std::vector<cv::Point2f> extentsVideo;
        std::vector<cv::Point2f> extentsTransformed;
    };
}

#endif /* CBP_AnalysisTypes_h */

