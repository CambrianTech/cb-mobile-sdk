//
//  CBP_DeepModel.hpp
//  Cambrian
//
//  Created by Joel Teply on 9/5/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#ifndef CBP_DeepModel_hpp
#define CBP_DeepModel_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <thread>
#include <cbar/CBAR_VideoFrame.hpp>
#include "CBP_FloodFillAlgorithm.hpp"

#define MODEL_ICNET 1001
#define MODEL_PIX2PIX 1002

namespace cbpipe {
    
    class DLL_LOCAL CBP_DeepModel {
        
    public:
        CBP_DeepModel();
        
        void loadDeepNetwork();
        virtual void inference(const cv::Mat &srcImage, const std::map<std::string, cv::Mat> &images, cv::Mat &result);
        virtual cv::Mat getDebugOutput(const cv::Mat &srcImage, const cv::Mat &result) const { return result; };
        
        float getBenchmarkedDuration() const;
        virtual const CBP_DeepModelInfo getModelInfo() const = 0;
    protected:
         ~CBP_DeepModel();
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
};

#endif /* CBP_DeepModel_hpp */
