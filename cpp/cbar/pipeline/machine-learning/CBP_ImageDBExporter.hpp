//
//  CBP_ImageDBExporter.hpp
//  Cambrian
//
//  Created by Joel Teply on 1/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBP_ImageDBExporter_hpp
#define CBP_ImageDBExporter_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <cbar/CBAR_VideoFrame.hpp>

namespace cbpipe {
    
    class DLL_LOCAL CBP_ImageDBExporter {
    public:
        CBP_ImageDBExporter();
        ~CBP_ImageDBExporter();
        
        enum EXPORT_TYPE {
            EXPORT_TYPE_LABELS,
            EXPORT_TYPE_EDGES,
            EXPORT_TYPE_SIAMESE,
            EXPORT_TYPE_TRIAMESE,
            EXPORT_TYPE_FULL,
            EXPORT_TYPE_EDGE_ANGLES,
        };
        
        std::string exportTypeToString(EXPORT_TYPE type);
        EXPORT_TYPE stringToExportType(std::string typeString);
        
        void exportTrainingImages(const std::string &trainingPath, const std::string &outputPath, const std::string &debugPath,
                                  cv::Size gridUnitSize=cv::Size(32,32), bool isRGB=true, EXPORT_TYPE type=EXPORT_TYPE_LABELS, std::string oredFilter="");
        
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
};


#endif /* CBP_ImageDBExporter_hpp */
