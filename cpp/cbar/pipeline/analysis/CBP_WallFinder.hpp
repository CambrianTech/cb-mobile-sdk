//
//  CBP_WallFinder.hpp
//  Cambrian
//
//  Created by Joel Teply on 4/17/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#ifndef CBP_WallFinder_hpp
#define CBP_WallFinder_hpp

#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <memory>

#include <cbar/pipeline/analysis/CBP_AreaAnalyzer.hpp>
#include <cbar/pipeline/CBP_Types.hpp>
#include <cbar/pipeline/analysis/CBP_AnalysisTypes.hpp>

namespace cbpipe {
    
    class DLL_LOCAL CBP_WallFinder : public CBP_AreaAnalyzer {
    public:
        CBP_WallFinder();
        ~CBP_WallFinder();
        
        struct semantic_data {
            int64_t frameIndex;
            cv::Mat rgb;
            cv::Mat semantic;
            cv::Mat normals;
        };
        
        struct candidate_wall {
            cv::Vec4f lineA;
            cv::Vec4f lineB;
        };

        void semanticDataUpdated(const semantic_data &semantic);
        
        void candidateWallsFound(const std::vector<candidate_wall> &walls, int64_t frameIndex);

    protected:
        virtual bool analyze(cbar::CBAR_VideoFramePtr frame);
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}


#endif /* CBP_WallFinder_hpp */
