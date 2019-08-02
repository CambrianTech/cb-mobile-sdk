//
//  FloodFill.hpp
//  Cambrian
//
//  Created by Joel Teply on 3/31/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#ifndef FloodFill_hpp
#define FloodFill_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <cambrian.h>

namespace imaging {
    
    template<typename ComparePatches>
    bool floodFillIterator(const cv::Mat &image, cv::Mat &onMask, const cv::Size &unitSize,
                           const cv::Point &index, std::vector<cv::Point> &newSamples, ComparePatches comparator) {
        
        bool expanded = false;
        
        int startY = index.y-1;
        int endY = index.y+1;
        
        int startX = index.x-1;
        int endX = index.x+1;
        
        cv::Rect indexRoi(index.x * unitSize.width, index.y * unitSize.height, unitSize.width, unitSize.height);
        
        for (int i=startY; i<=endY; i++) {
            
            int y = i * unitSize.height;
            
            if (i < 0 || y + unitSize.height >= image.rows) continue;
            
            //go into all neighbors
            for (int j=startX; j<=endX; j++) {
                
                int x = j * unitSize.width;
                
                if (j < 0 || x + unitSize.width >= image.cols || (index.y == i && index.x == j)) continue;
                
                cv::Point siblingIndex = cv::Point(j,i);
                
                uchar value = onMask.at<uchar>(siblingIndex);
                if (value) continue;
                
                cv::Rect siblingRoi(x, y, unitSize.width, unitSize.height);
                if (comparator(indexRoi, siblingRoi)) {
                    onMask.at<uchar>(siblingIndex) = 255;
                    newSamples.push_back(siblingIndex);
                    expanded = true;
                }
            }
        }
        
        return expanded;
    }
    
    template<typename ComparePatches>
    int floodFill(const cv::Mat &image, const cv::Size &unitSize, cv::Mat &onMask, const std::vector<cv::Point>&seeds, ComparePatches comparator) {
        
        if (onMask.empty()) {
            onMask = cv::Mat::zeros(image.rows / unitSize.height, image.cols / unitSize.width, CV_8UC1);
            //flood fill some
            for (const auto &seed : seeds) {
                cv::Point point = cv::Point(seed.x / unitSize.width, seed.y / unitSize.height);
                onMask.at<uchar>(point) = 255;
            }
        }
        
        std::vector<cv::Point> indices;
        cv::findNonZero(onMask, indices);
        
        int count = 0;
        while (indices.size()) {
            auto sample = indices.back();
            count++;
            indices.pop_back();
            
            std::vector<cv::Point> newSamples;
            if (floodFillIterator(image, onMask, unitSize, sample, newSamples, comparator)) {
                indices.insert(indices.end(), newSamples.begin(), newSamples.end());
            }
        }
        
        return count;
    }
    
    
    
}

#endif /* FloodFill_hpp */
