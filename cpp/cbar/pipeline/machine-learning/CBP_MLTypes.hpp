//
//  CBP_MLTypes.hpp
//  Cambrian
//
//  Created by Joel Teply on 1/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBP_MLTypes_hpp
#define CBP_MLTypes_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <cbar/CBAR_VideoFrame.hpp>

namespace cbpipe {

    struct DLL_LOCAL texture_sample {
        cv::Point gridIndex;

        cv::Point center;
        cv::Size size;
        std::vector<float>data;
        bool isOn = false;
        bool isSeed = false;
        int label=-1;
        double confidence = 1;
        bool isEdge = false;
        cv::Point sourceIndex = cv::Point(-1,-1);

        cv::Rect roi;
        
        cv::Mat blob;
        char *userData=0;
    };

    struct DLL_LOCAL texture_row {
        int index;
        std::vector<texture_sample>samples;
        void clearData() {
            for (auto &sample : samples) {
                sample.data.clear();
            }
        }
        void reset() {
            for (auto &sample : samples) {
                sample.isOn = false;
                sample.isSeed = false;
                sample.isEdge = false;
                sample.label = -1;
                sample.confidence = 1;
                sample.data.clear();
            }
        }
    };

    struct DLL_LOCAL texture_grid {

        const cv::Size gridDimensions;
        
        cv::Mat srcImage;
        const cv::Size gridUnitSize;

        std::vector<texture_row>rows;
        std::vector<texture_sample*>seeds;
        std::vector<texture_sample*>selectedSamples;

        cv::Mat positivesMask;
        cv::Mat negativesMask;

        cv::Mat trainingMask;

        texture_grid(const cv::Mat &_srcImage, cv::Size _gridUnitSize)
            :srcImage(_srcImage), gridUnitSize(_gridUnitSize),
            gridDimensions(_srcImage.cols / _gridUnitSize.width, _srcImage.rows / _gridUnitSize.height) {

        }

        void clearData() {
            for (auto &row : rows) {
                row.clearData();
            }
        }

        void reset(const cv::Mat &_srcImage) {
            srcImage = _srcImage;
            seeds.clear();
            selectedSamples.clear();
            for (auto &row : rows) {
                row.reset();
            }
        }
    };
}

#endif /* CBP_MLTypes_hpp */
