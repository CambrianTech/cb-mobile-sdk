//
//  CBP_SemanticAnalyzer.hpp
//  Cambrian
//
//  Created by Joel Teply on 4/2/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#ifndef CBP_SemanticAnalyzer_hpp
#define CBP_SemanticAnalyzer_hpp

#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <memory>

#include <cbar/pipeline/analysis/CBP_AreaAnalyzer.hpp>
#include <cbar/pipeline/CBP_Types.hpp>
#include <cbar/pipeline/analysis/CBP_AnalysisTypes.hpp>

namespace cbpipe {
    
    class DLL_LOCAL CBP_SemanticAnalyzer : public CBP_AreaAnalyzer {
    public:
        CBP_SemanticAnalyzer();
        ~CBP_SemanticAnalyzer();
        
        enum result_index {
            result_index_bw = 0,
            result_index_semantic_ground,
            result_index_semantic_walls,
            result_index_semantic_other,
            result_index_length,
        };
        
    protected:
        virtual void initialize();
        virtual area_run_params getParams() const;
        virtual bool analyze(cbar::CBAR_VideoFramePtr frame);
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif /* CBP_SemanticAnalyzer_hpp */
