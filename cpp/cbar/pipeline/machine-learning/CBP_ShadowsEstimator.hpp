//
//  CBP_ShadowsEstimator.cpp
//  Cambrian
//
//  Created by Joel Teply on 1/15/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#ifndef CBP_ShadowsEstimator_hpp
#define CBP_ShadowsEstimator_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <cbar/CBAR_VideoFrame.hpp>
#include "CBP_DeepModel.hpp"

namespace cbpipe {
    
    class DLL_LOCAL CBP_ShadowsEstimator : public CBP_DeepModel  {
        
    public:
        CBP_ShadowsEstimator();
        ~CBP_ShadowsEstimator();
        
        virtual const CBP_DeepModelInfo getModelInfo() const;
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
        
        static CBMutex m_singleInstanceMutex; //one at a time
        static std::shared_ptr<CBP_ShadowsEstimator> m_singleInstance;
    };
};

#endif /* CBP_ShadowsEstimator_hpp */
