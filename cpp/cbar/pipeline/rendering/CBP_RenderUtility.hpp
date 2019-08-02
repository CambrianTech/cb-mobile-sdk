//
//  CBP_RenderUtility.hpp
//  Cambrian
//
//  Created by Joel Teply on 3/20/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBP_RenderUtility_hpp
#define CBP_RenderUtility_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <cambrian.h>
#include <cbcommon/CB_Types.h>

//#include <assimp/Importer.hpp>      // C++ importer interface
//#include <assimp/scene.h>           // Output data structure
//#include <assimp/postprocess.h>     // Post processing flags

namespace cbpipe {
    
    class DLL_LOCAL CBP_RenderUtility {
    public:
        static cv::Scalar getLightingMultiplier(LightingType lighting);
    };
}

#endif /* CBP_RenderUtility_hpp */
