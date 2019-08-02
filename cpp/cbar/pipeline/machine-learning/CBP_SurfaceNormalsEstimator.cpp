//
//  CBP_SurfaceNormalsEstimator.cpp
//  Cambrian
//
//  Created by Joel Teply on 4/2/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#include "CBP_SurfaceNormalsEstimator.hpp"

#include <cbar/pipeline/pipeline.h>
#include <imaging/ImageProcessing.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Accelerated.h>
#include <utility/Directory.h>
#include <imaging/Imaging.h>
#include <thread>

#include <cbar/pipeline/CBP_Types.hpp>
#include <cbar/pipeline/motion/motion.h>
#include <cbar/pipeline/machine-learning/machine-learning.h>

using namespace imaging;

namespace cbpipe {
    
    struct CBP_SurfaceNormalsEstimator::Impl
    {
        Impl(CBP_SurfaceNormalsEstimator *parent) : m_parent(parent) {
            
        }
        
        ~Impl() {}
        
        CBP_SurfaceNormalsEstimator *m_parent;
    };
    
    CBP_SurfaceNormalsEstimator::CBP_SurfaceNormalsEstimator() {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }
    
    CBP_SurfaceNormalsEstimator::~CBP_SurfaceNormalsEstimator() {
        
    }
    
    const CBP_DeepModelInfo CBP_SurfaceNormalsEstimator::getModelInfo() const {
        CBP_DeepModelInfo modelInfo;
        
        modelInfo.name = "Normals";
        modelInfo.type = CBAR_PredictionType_Normals;
        
        modelInfo.input.name = "input";
        modelInfo.input.resolution = cv::Size(256,256);
        modelInfo.input.depth = CV_8UC3;
        modelInfo.input.multiplier = 1.0f/255.0f;

        modelInfo.output.name = "output";
        modelInfo.output.resolution = cv::Size(256,256);
        modelInfo.output.depth = CV_8UC3;
        modelInfo.output.multiplier = 255.0f;
        modelInfo.output.flipBGR = false;
        
        return modelInfo;
    }
};
