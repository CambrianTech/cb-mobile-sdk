//
//  CBP_ElevationEstimator.hpp
//  Cambrian
//
//  Created by Joel Teply on 1/15/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#ifndef CBP_ElevationEstimator_hpp
#define CBP_ElevationEstimator_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <cbar/CBAR_VideoFrame.hpp>
#include "CBP_DeepModel.hpp"

namespace cbpipe {
    
    class DLL_LOCAL CBP_ElevationEstimator : public CBP_DeepModel  {
        
    public:
        CBP_ElevationEstimator();
        ~CBP_ElevationEstimator();
        
        virtual const CBP_DeepModelInfo getModelInfo() const;
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
        
        static CBMutex m_singleInstanceMutex; //one at a time
        static std::shared_ptr<CBP_ElevationEstimator> m_singleInstance;
    };
};

#endif /* CBP_ElevationEstimator_hpp */
