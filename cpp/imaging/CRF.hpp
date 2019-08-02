//
//  CRF.hpp
//  Cambrian
//
//  Created by Joel Teply on 2/17/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#ifndef CRF_hpp
#define CRF_hpp

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <cambrian.h>
#include <cbcommon/CB_Types.h>

namespace imaging {
    
    class DLL_PUBLIC CRF
    {
        enum CRF_Model {
            CRF_Model_Bayes = 0,
            CRF_Model_Gaussian,
            CRF_Model_CV_Gaussian,
            CRF_Model_NearestNeighbor,
            CRF_Model_CV_NearestNeighbor,
            CRF_Model_CV_RandomForest,
            CRF_Model_MS_RandomForest,
            CRF_Model_CV_ANN,
            CRF_Model_CV_SVM,
        };
        
    public:
        CRF(CRF_Model model, unsigned int numStates = 6, unsigned int numFeatures = 3);
        ~CRF();
        
        void train(const cv::Mat &img, const cv::Mat &gt);
        
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
};

#endif /* CRF_hpp */
