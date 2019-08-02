//
//  CBP_ElevationEstimator.cpp
//  Cambrian
//
//  Created by Joel Teply on 4/2/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#include "CBP_ElevationEstimator.hpp"

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
    
    struct CBP_ElevationEstimator::Impl
    {
        Impl(CBP_ElevationEstimator *parent) : m_parent(parent) {
            
        }
        
        ~Impl() {}
        
        CBP_ElevationEstimator *m_parent;
    };
    
    CBP_ElevationEstimator::CBP_ElevationEstimator() {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }
    
    CBP_ElevationEstimator::~CBP_ElevationEstimator() {
        
    }
    
    const CBP_DeepModelInfo CBP_ElevationEstimator::getModelInfo() const {
        CBP_DeepModelInfo modelInfo;
        
        modelInfo.name = "Elevation";
        modelInfo.type = CBAR_PredictionType_Elevation;
        
        modelInfo.input.name = "input";
        modelInfo.input.resolution = cv::Size(512,512);
        modelInfo.input.depth = CV_8UC3;
        modelInfo.input.multiplier = 1.0f/255.0f;

        modelInfo.output.name = "output";
        modelInfo.output.resolution = cv::Size(512,512);
        modelInfo.output.depth = CV_8UC3;
        modelInfo.output.multiplier = 255.0f;
        modelInfo.output.flipBGR = false;
        
        return modelInfo;
    }
};
