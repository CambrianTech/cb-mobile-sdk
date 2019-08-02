//
//  CBP_SemanticSegmenter.hpp
//  Cambrian
//
//  Created by Joel Teply on 4/2/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#ifndef CBP_SemanticSegmenter_hpp
#define CBP_SemanticSegmenter_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <cbar/CBAR_VideoFrame.hpp>

#include "CBP_DeepModel.hpp"

namespace cbpipe {
    
    class DLL_LOCAL CBP_SemanticSegmenter : public CBP_DeepModel {
        
    public:
        CBP_SemanticSegmenter();
        ~CBP_SemanticSegmenter();
        
        virtual cv::Mat getDebugOutput(const cv::Mat &srcImage, const cv::Mat &result);
        virtual const CBP_DeepModelInfo getModelInfo() const;
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
        
        static CBMutex m_singleInstanceMutex; //one at a time
        static std::shared_ptr<CBP_SemanticSegmenter> m_singleInstance;
    };
};

#endif /* CBP_SemanticSegmenter_hpp */
