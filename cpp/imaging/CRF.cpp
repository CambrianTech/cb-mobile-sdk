//
//  CRF.cpp
//  Cambrian
//
//  Created by Joel Teply on 2/17/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#include "CRF.hpp"

#include <DGM.h>
#include <FEX.h>

using namespace DirectGraphicalModels;
using namespace DirectGraphicalModels::fex;

namespace imaging {
    
    struct CRF::Impl
    {
        Impl(CRF *parent, CRF_Model model, unsigned int numStates, unsigned int numFeatures)
            : m_parent(parent), m_model(model), m_numStates(numStates), m_numFeatures(numFeatures)
        {
            m_nodeTrainer = CTrainNode::create(m_model, numStates, numFeatures);
        }
        ~Impl() {
            
        }
        
        // merges some classes in one
        cv::Mat shrinkStateImage(const cv::Mat &gt, byte nStates)
        {
            // assertions
            if (gt.type() != CV_8UC1) return cv::Mat();
            
            cv::Mat res;
            gt.copyTo(res);
            
            for (byte &val : static_cast<Mat_<byte>>(res))
                if (val < 3)        val = 0;
                else if (val < 4)    val = 1;
                else                val = 2;
            //    val = (val + 1) % nStates;
            
            return res;
        }
        
        //https://github.com/Project-10/DGM/blob/master/demos/Demo%20Train.cpp
        void train(const cv::Mat &img, const cv::Mat &_gt) {
            
            // reduce the number of classes in gt to nStates
            cv::Mat gt = shrinkStateImage(_gt, m_numStates);
            
            // the value of partition function
            float Z =  1.0f;
            if (m_model == CRF_Model_Bayes) Z = 2e34f;
            else if (m_model == CRF_Model_MS_RandomForest || m_model == CRF_Model_CV_ANN) Z = 0.0f;
            
            //    ---------- Features Extraction ----------
            vec_mat_t featureVector;
            CCommonFeatureExtractor fExtractor(img);
            featureVector.push_back(fExtractor.getNDVI(0).autoContrast().get());
            featureVector.push_back(fExtractor.getSaturation().invert().get());
            
            for (int y = 0; y < gt.rows; y++)
                for (int x = 0; x < gt.cols; x++)
                    if (gt.at<byte>(y, x) == 1) {
                        float val = (float) featureVector[0].at<byte>(y, x);
                        val = val - 10;
                        featureVector[0].at<byte>(y, x) = (byte) MAX(0.0f, val + 0.5f);
                    }
                    else if (gt.at<byte>(y, x) == 2) {
                        float val = (float) featureVector[1].at<byte>(y, x);
                        val = val + 0;
                        featureVector[1].at<byte>(y, x) = (byte) MAX(0.0f, val + 0.5f);
                    }
            
            //    ---------- Training ----------
            m_nodeTrainer->addFeatureVecs(featureVector, gt);
            m_nodeTrainer->train();
        }
        
        CRF *m_parent;
        CRF_Model m_model;
        std::shared_ptr<CTrainNode> m_nodeTrainer;
        unsigned int m_numStates, m_numFeatures;
    };
    
    CRF::CRF(CRF_Model model, unsigned int numStates, unsigned int numFeatures)
    {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, model, numStates, numFeatures));
    }
    
    CRF::~CRF()
    {
        
    }
    
    void CRF::train(const cv::Mat &img, const cv::Mat &gt) {
        m_pImpl->train(img, gt);
    }
}
