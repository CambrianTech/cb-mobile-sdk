//
//  CBP_SurfaceNormalsEstimator.hpp
//  Cambrian
//
//  Created by Joel Teply on 4/2/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#ifndef CBP_SurfaceNormalsEstimator_hpp
#define CBP_SurfaceNormalsEstimator_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <cbar/CBAR_VideoFrame.hpp>
#include "CBP_DeepModel.hpp"

namespace cbpipe {
    
    class DLL_LOCAL CBP_SurfaceNormalsEstimator : public CBP_DeepModel  {
        
    public:
        CBP_SurfaceNormalsEstimator();
        ~CBP_SurfaceNormalsEstimator();
        
        virtual const CBP_DeepModelInfo getModelInfo() const;
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
        
        static CBMutex m_singleInstanceMutex; //one at a time
        static std::shared_ptr<CBP_SurfaceNormalsEstimator> m_singleInstance;
    };
};

#endif /* CBP_SurfaceNormalsEstimator_hpp */
