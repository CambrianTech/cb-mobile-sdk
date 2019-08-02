//
//  CBP_FloodFillIterator.hpp
//  Cambrian
//
//  Created by Joel Teply on 1/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBP_FloodFillIterator_hpp
#define CBP_FloodFillIterator_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <memory>
#include <cbar/CBAR_VideoFrame.hpp>
#include "CBP_MLTypes.hpp"

namespace cbpipe {
    
    class DLL_LOCAL CBP_FloodFillIterator {
        
    public:
        CBP_FloodFillIterator() {};
        ~CBP_FloodFillIterator() {};
        
        template<typename CompareElements>
        bool iterativeAnalysis(cv::Ptr<texture_grid> grid,
                               texture_sample &element, std::vector<texture_sample*>&newSamples,
                               CompareElements comparator) {
            
            bool added = false;

            int startY = element.gridIndex.y-1;
            int endY = element.gridIndex.y+1;
            
            int startX = element.gridIndex.x-1;
            int endX = element.gridIndex.x+1;
            
            for (int i=startY; i<=endY; i++) {
                
                if (i < 0 || i >= grid->rows.size()) continue;
                
                auto &row = grid->rows[i];
                
                for (int j=startX; j<=endX; j++) {
                    
                    if (j < 0 || j >= row.samples.size()) continue;
                    
                    texture_sample &sibling = row.samples[j];
                    
                    if (element.gridIndex == sibling.gridIndex || sibling.isOn) continue;
                    
                    if (comparator(element, sibling)) {
                        sibling.sourceIndex = element.gridIndex;
                        sibling.isOn = true;
                        added = true;
                        newSamples.push_back(&sibling);
                    }
                }
            }

            return added;
        }
    };
}

#endif /* CBP_FloodFillIterator_hpp */
