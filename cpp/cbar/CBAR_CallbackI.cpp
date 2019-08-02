//
//  CBAR_CallbackI.cpp
//  Cambrian
//
//  Created by Joel Teply on 1/30/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#include <stdio.h>
#include "CBAR_CallbackI.hpp"
#include <cbar/pipeline/rendering/rendering.h>

using namespace cbpipe;

namespace cbar {
    void CBAR_CallbackI::saveScreenshot(const std::string &path, bool isJPEG, int qualityLevel) {

    }
    
    std::vector<cbpipe::HitTestResult> CBAR_CallbackI::hitTestAtPoint(const cv::Point2f &point, cbpipe::HitTestResultType resultType) {
        std::vector<cbpipe::HitTestResult> results;
        
        return results;
    }
}


