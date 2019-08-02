//
//  CBP_FloorFloodFill.cpp
//  Cambrian
//
//  Created by Joel Teply on 4/7/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#include "CBP_FloorFloodFill.hpp"

#include <opencv2/ml/ml.hpp>
#include <opencv2/dnn/dnn.hpp>

#include <fstream>

#include <utility/CommonUtility.h>
#include <utility/Directory.h>
#include <utility/Diagnostics.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Imaging.h>
#include <imaging/LineProcessing.h>

#define BACKCHECK_SUCCESS 3
#define BACKCHECK_CONF 0.7

#define REFINE_NEGATIVES 1
#define NEIGHBOR_COUNT 10
#define EDGES_DATA_SIZE 6
#define REFINE_DATA_SIZE (3 + 2 * NEIGHBOR_COUNT + 6)
#define INCLUDE_RGB_DATA 1

#include "CBP_MLUtility.hpp"

using namespace imaging;

namespace cbpipe {
    
    struct CBP_FloorFloodFill::Impl
    {
        Impl(CBP_FloorFloodFill *floodFill)
          : m_floodFill(floodFill)
        {
            
            auto modelTxt = cbar::getCBAssetPath("floors.nnet.prototxt");
            auto modelBin = cbar::getCBAssetPath("floors.nnet.caffemodel");
            
            m_siameseModel = cv::dnn::readNetFromCaffe(modelTxt, modelBin);
            
            if (m_siameseModel.empty()) {
                CBError("Caffe model %s could not be found", modelTxt.c_str());
                return;
            } else {
                CBLog("Caffe model %s loaded", modelTxt.c_str());
            }
            
            auto refineTxt = cbar::getCBAssetPath("floors.refine.xml");
            
            cv::ml::StatModel::load<cv::ml::ANN_MLP>(refineTxt);
            
            m_annModel = cv::ml::StatModel::load<cv::ml::ANN_MLP>(refineTxt);
            
            if (m_annModel.empty()) {
                CBError("ANN model %s could not be found", refineTxt.c_str());
                return;
            } else {
                CBLog("ANN model %s loaded", refineTxt.c_str());
            }
        }
        ~Impl() {}
        
        cv::dnn::Net m_siameseModel;
        CBP_FloorFloodFill *m_floodFill;
        cv::Ptr<cv::ml::ANN_MLP> m_annModel;
        
        cv::Mat m_floodImage;
        bool m_isReady = false;

        void prepare(cv::Ptr<texture_grid> grid, bool isVideo) {
            m_isReady = false;
            
            if (m_siameseModel.empty()) {
                return;
            }
            
            std::vector<int> outputs = m_siameseModel.getUnconnectedOutLayers();
            if (!outputs.size()) {
                return;
            }
            
            auto layer = m_siameseModel.getLayer(outputs[0]);
            
            if (layer->getDefaultName().empty()) {
                return;
            }
            
            cv::blur(grid->srcImage, m_floodImage, cv::Size(5,5));
            
            m_isReady = true;
        }
        
        inline void ensureData(texture_sample &element, const cv::Mat &rgbImage) {
            if (element.blob.empty()) {
                element.blob = cv::dnn::blobFromImage(rgbImage(element.roi));
            }
        }
        
        double getMinimumConfidence(const texture_sample &element, int numElements, cv::Ptr<texture_grid> grid) {
            std::vector<double>confidences;
            
            cv::Point sourceIndex = element.sourceIndex;
            
            for (int i=0; i<numElements; i++) {
                if (sourceIndex.x < 0) break;
                
                confidences.push_back(grid->rows[sourceIndex.y].samples[sourceIndex.x].confidence);
                sourceIndex = grid->rows[sourceIndex.y].samples[sourceIndex.x].sourceIndex;//get next
            }
            
            if (confidences.size() > 3) {
                cv::Scalar mean, stddev;
                cv::meanStdDev(confidences, mean, stddev);
                
                return mean[0] - 2.0 * stddev[0];
            }
            
            return 0.7;
        }
        
        void runSiameseCNN(cv::Ptr<texture_grid> grid, texture_sample &elementA, texture_sample &elementB) {
            
            int classId = 0;
            double classProb = 0.0f;
            
            if (!elementA.blob.empty() && !elementB.blob.empty()) {
                m_siameseModel.setInput(elementA.blob, "image_a");
                m_siameseModel.setInput(elementB.blob, "image_b");
                
                cv::Mat prob = m_siameseModel.forward("prob");
                
                CBP_MLUtility::getMaxClass(prob, &classId, &classProb);
            }
            
            elementB.confidence = classProb;
            elementB.label = classId ? 1 : 0;
        }
        
        cv::Point getSourceIndex(const texture_sample &element, int numElementsBack, cv::Ptr<texture_grid> grid) {
            cv::Point sourceIndex = element.sourceIndex;
            
            for (int i=0; i<numElementsBack && sourceIndex.x >= 0; i++) {
                cv::Point testSourceIndex = grid->rows[sourceIndex.y].samples[sourceIndex.x].sourceIndex;//get next
                if (testSourceIndex.x < 0) {
                    break;
                }
                sourceIndex = testSourceIndex;
            }
            
            return sourceIndex;
        }
        
        bool isMatch(cv::Ptr<texture_grid> grid, texture_sample &elementA, texture_sample &elementB) {
            
            if (!m_isReady) return false;
            
            ensureData(elementA, m_floodImage);
            ensureData(elementB, m_floodImage);
            
            if (elementA.blob.empty() || elementB.blob.empty()) return false;
            
            runSiameseCNN(grid, elementA, elementB);
            
            if (elementB.isSeed) return true;
            
            bool success = elementB.label == 1 && elementB.confidence > getMinimumConfidence(elementA, 5, grid);
            
#if BACKCHECK_SUCCESS
            if (success && elementB.confidence < BACKCHECK_CONF) {
                cv::Point sourceIndex = getSourceIndex(elementA, BACKCHECK_SUCCESS, grid);
                if (sourceIndex.x >= 0) {
                    runSiameseCNN(grid, grid->rows[sourceIndex.y].samples[sourceIndex.x], elementB);
                    success = elementB.label == 1;
                    //                        if (!success) {
                    //                            printf("\nSTOPPED\n\n");
                    //                        }
                }
            }
#endif

            return success;
        }
        
        inline void populateRefinementSample(cv::Ptr<texture_grid> grid, texture_sample &sample) {
            
            sample.data.resize(REFINE_DATA_SIZE);
            
            sample.data[0] = sample.label;
            sample.data[1] = sample.confidence;
            sample.data[2] = sample.isEdge;
            
            int start = 3;
            
#if NEIGHBOR_COUNT
            const texture_sample *neigh = &sample;
            for (int i=0; i<NEIGHBOR_COUNT; i++) {
                cv::Point sourceIndex = getSourceIndex(*neigh, start/2, grid);
                if (sourceIndex.x >= 0) {
                    neigh = &grid->rows[sourceIndex.y].samples[sourceIndex.x];
                    sample.data[start] = neigh->label;
                    sample.data[start+1] = neigh->confidence;
                }
                start+=2;
            }
#endif
            
#if INCLUDE_RGB_DATA
            
            cv::Rect roi = cv::Rect(grid->gridUnitSize.width * sample.gridIndex.x,
                                    grid->gridUnitSize.height * sample.gridIndex.y,
                                    grid->gridUnitSize.width, grid->gridUnitSize.height);
            
            cv::Scalar mean, stddev;
            cv::meanStdDev(m_floodImage(roi), mean, stddev);
            
            for (int i=0; i<3; i++) {
                sample.data[i+start] = mean[i] / 255.0f;
                sample.data[i+start+3] = stddev[i] / 255.0f;
            }
#endif
        }
        
        inline void populateRefinementNNElement(cv::Ptr<texture_grid> grid,
                                                cv::Mat &data, int i, texture_sample &sample) {
            
            if (sample.data.empty()) populateRefinementSample(grid, sample);
            
            for (int j=0; j<sample.data.size(); j++) {
                data.at<float>(i,j) = sample.data[j];
            }
        }
        
        void runRefinementClassifier(cv::Ptr<texture_grid> grid, cv::Mat &result) {
            //run refinement classifier
            for (int y=0; y<result.rows; y++) {
                for (int x=0; x<result.cols; x++) {
                    
                    cv::Point gridIndex = cv::Point(x, y);
                    
                    bool isPositiveExample = result.at<uchar>(gridIndex);
                    
                    //ON
                    auto &sample = grid->rows[y].samples[x];
                    //_floodFill->analyzeSample(grid, sample);
                    //std::cerr << sample.origin << " " << sample.data[0] << std::endl;
                    
                    //populate for FloodFill
                    if ((isPositiveExample || REFINE_NEGATIVES) && sample.confidence < 0.95) {
                        
                        auto matchesData = cv::Mat_<float>(1, REFINE_DATA_SIZE);
                        
                        populateRefinementNNElement(grid, matchesData, 0, sample);
                        
                        int prediction;
                        
                        //ANN
                        cv::Mat output;
                        m_annModel->predict(matchesData, output);
                        float value = output.at<float>(0);
                        prediction = value > FLT_EPSILON ? 1.0 : -1.0;
                        
                        sample.label = prediction > 0 ? 1 : 0;
                        result.at<uchar>(y,x) = prediction > 0 ? 1 : 0;
                    }
                }
            }
            
        }
    };
    
    CBP_FloorFloodFill::CBP_FloorFloodFill(const cv::Size &gridSize)
    : CBP_FloodFillAlgorithm(PaintPointType_Surface, gridSize, cv::Size(640, 640)) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }
    
    CBP_FloorFloodFill::~CBP_FloorFloodFill() {
        
    }
    
    void CBP_FloorFloodFill::getReplicatedSeedValue(cv::Ptr<texture_grid> grid,
                                                     std::vector<texture_sample*>&seeds, texture_sample &replicatedSample) {
        
    }
    
    void CBP_FloorFloodFill::prepare(cv::Ptr<texture_grid> grid, bool isVideo) {
        
        m_pImpl->prepare(grid, isVideo);
    }
    
    cv::Mat CBP_FloorFloodFill::runGrid(cv::Ptr<texture_grid> grid, const std::vector<cv::Point2f> &paintPoints, bool isVideo) {

        cv::Mat result = CBP_FloodFillAlgorithm::runGrid(grid, paintPoints, false);
        
        if (!result.empty()) m_pImpl->runRefinementClassifier(grid, result);
        
        return result;
    }
    
    bool CBP_FloorFloodFill::isMatch(cv::Ptr<texture_grid> grid, texture_sample &elementA, texture_sample &elementB) {
        return m_pImpl->isMatch(grid, elementA, elementB);
    }
};
