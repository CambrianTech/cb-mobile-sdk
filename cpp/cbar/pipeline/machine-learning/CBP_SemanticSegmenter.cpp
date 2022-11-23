//
//  CBP_SemanticSegmenter.cpp
//  Cambrian
//
//  Created by Joel Teply on 4/2/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#include "CBP_SemanticSegmenter.hpp"

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
        
    struct CBP_SemanticSegmenter::Impl
    {
        Impl(CBP_SemanticSegmenter *parent) : m_parent(parent) {
            
        }
        
        ~Impl() {}
        
        CBP_SemanticSegmenter *m_parent;
        
        cv::Mat _get_debug_output(const cv::Mat &srcImage, const cv::Mat &result) {
            
            std::vector<std::string> paletteNames;
            std::vector<cv::Scalar> fullPalette;
            
            //todo: populate labelCounts
            std::vector<int64_t> labelCounts;
            
            if (fullPalette.empty()) {
                paletteNames.push_back("Item 1"); fullPalette.push_back(cv::Scalar(255,0,0));
                paletteNames.push_back("Item 2"); fullPalette.push_back(cv::Scalar(0,255,0));
                paletteNames.push_back("Item 3"); fullPalette.push_back(cv::Scalar(0,0,255));
                paletteNames.push_back("Item 4"); fullPalette.push_back(cv::Scalar(255,255,0));
                paletteNames.push_back("Item 5"); fullPalette.push_back(cv::Scalar(255,0,255));
                paletteNames.push_back("Item 6"); fullPalette.push_back(cv::Scalar(0,255,255)); //floor is index = 0
                paletteNames.push_back("Item 7"); fullPalette.push_back(cv::Scalar(255,128,255));
                paletteNames.push_back("Item 8"); fullPalette.push_back(cv::Scalar(255,255,128));
                paletteNames.push_back("Item 9"); fullPalette.push_back(cv::Scalar(128,255,0));
                paletteNames.push_back("Item 10"); fullPalette.push_back(cv::Scalar(255,128,0));
                paletteNames.push_back("Item 11"); fullPalette.push_back(cv::Scalar(255,0,128));
            }
            
            cv::Mat debugImage = cv::Mat::zeros(result.rows, result.cols, CV_8UC3);
            
            int j = 0;
            for (int i = 0; i < result.total(); ++i) {
                uchar value = result.data[i];
                cv::Scalar color = fullPalette[value];
                debugImage.data[j++] = color[0];
                debugImage.data[j++] = color[1];
                debugImage.data[j++] = color[2];
            }
            
            cv::resize(debugImage, debugImage, srcImage.size());
            cv::addWeighted(debugImage, 0.7, srcImage, 0.3, 0.0, debugImage);
            
            //add map key
            cv::Mat mapKey = cv::Mat(debugImage.rows, 400, CV_8UC3);
            mapKey.setTo(cv::Scalar::all(255));
            
            int index = 0;
            int padding = 5;
            int elementHeight = 30 + padding;
            int elementWidth = 50;
            
            for (int i=0; i<labelCounts.size(); i++) {
                if (labelCounts[i] > 3) {
                    auto startPoint = cv::Point(padding, index * elementHeight + padding);
                    auto endPoint = cv::Point(elementWidth - padding, startPoint.y + elementHeight - padding);
                    cv::rectangle(mapKey, startPoint, endPoint, fullPalette[i], CV_FILLED);
                    std::string label = string_sprintf("%d) %s", i, paletteNames[i].c_str());
                    cv::putText(mapKey, label, endPoint + cv::Point(10, - padding), cv::FONT_HERSHEY_DUPLEX, 0.7, cv::Scalar::all(0), 1, cv::LINE_AA);
                    index ++;
                }
            }
            
            cv::hconcat(debugImage, mapKey, debugImage);
            
            return debugImage;
        }
    };
    
    CBP_SemanticSegmenter::CBP_SemanticSegmenter() {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }
    
    CBP_SemanticSegmenter::~CBP_SemanticSegmenter() {
        
    }
    
    const CBP_DeepModelInfo CBP_SemanticSegmenter::getModelInfo() const {
        CBP_DeepModelInfo modelInfo;
        
        modelInfo.name = "Semantic";
        modelInfo.type = CBAR_PredictionType_Semantic;
        
        modelInfo.input.name = "input";
        modelInfo.input.resolution = cv::Size(256,256);
        modelInfo.input.depth = CV_8UC3;
        
        modelInfo.output.name = "output";
        modelInfo.output.resolution = cv::Size(256,256);
        modelInfo.output.depth = CV_8UC1;
        
        return modelInfo;
    }
    
    cv::Mat CBP_SemanticSegmenter::getDebugOutput(const cv::Mat &srcImage, const cv::Mat &result) {
        return m_pImpl->_get_debug_output(srcImage, result);
    }
};
