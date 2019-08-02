//
//  CBP_RenderUtility.cpp
//  Cambrian
//
//  Created by Joel Teply on 3/20/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#include "CBP_RenderUtility.hpp"

#include <utility/CommonUtility.h>
#include <cbar/pipeline/util/util.h>
#include <utility/Directory.h>
#include <utility/Diagnostics.h>

namespace cbpipe {
    
    cv::Scalar CBP_RenderUtility::getLightingMultiplier(LightingType lighting) {
        
        cv::Scalar multiplier = cv::Scalar::all(1.0);
        
        switch (lighting) {
            case LightingTypeIncandescent:
                multiplier = cv::Scalar(1.0, 214/255.0, 170/255.0, 1.0);
                break;
            case LightingTypeFluorescent:
                multiplier = cv::Scalar(244/255.0, 255/255.0, 250/255.0, 1.0);
                break;
            case LightingTypeLEDWarm:
                multiplier = cv::Scalar(255/255.0, 234/255.0, 200/255.0, 1.0);
                break;
            case LightingTypeLEDWhite:
                multiplier = cv::Scalar(255/255.0, 240/255.0, 255/255.0, 1.0);
                break;
            case LightingTypeSunrise:
                multiplier = cv::Scalar(255/255.0, 200/255.0, 120/255.0, 1.0);
                break;
            case LightingTypeDaylightMorning:
                multiplier = cv::Scalar(255/255.0, 214/255.0, 170/255.0, 1.0);
                break;
            case LightingTypeDaylight:
                multiplier = cv::Scalar(245/255.0, 245/255.0, 245/255.0, 1.0);
                break;
            case LightingTypeDaylightEvening:
                multiplier = cv::Scalar(205/255.0, 205/255.0, 215/255.0, 1.0);
                break;
            case LightingTypeDaylightOvercast:
                multiplier = cv::Scalar(240/255.0, 245/255.0, 255/255.0, 1.0);
                break;
            default:
                
                break;
        }
        
        return multiplier;
    }
    
}
