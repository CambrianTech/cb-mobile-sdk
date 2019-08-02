//
//  CBP_Surface.hpp
//  Cambrian
//
//  Created by Joel Teply on 2/19/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#ifndef CBP_LargeImage_hpp
#define CBP_LargeImage_hpp

#include <stdio.h>

#include <stdio.h>
#include <cbar/pipeline/CBP_Types.hpp>
#include <opencv2/opencv.hpp>
#include <memory>

namespace cbpipe {
    
    class DLL_LOCAL CBP_LargeImage {
        
    public:
        CBP_LargeImage();
        CBP_LargeImage(const cv::Mat& other, const cv::Rect& roi);
        CBP_LargeImage(const CBP_LargeImage& other);
        ~CBP_LargeImage();
        
        CBP_LargeImage& operator = (const CBP_LargeImage &t);
        
        cv::Mat operator()( const cv::Rect& roi );

        cv::Rect region() const;
        
        cv::Mat get() const;
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif /* CBP_LargeImage_hpp */
