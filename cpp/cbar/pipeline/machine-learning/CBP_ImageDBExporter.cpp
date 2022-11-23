//
//  CBP_ImageDBExporter.cpp
//  Cambrian
//
//  Created by Joel Teply on 1/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#include "CBP_ImageDBExporter.hpp"
#include "CBP_FloodFillIterator.hpp"

#include "CBP_MLUtility.hpp"
#include <imaging/ImageProcessing.h>
#include <utility/Directory.h>
#include <utility/Diagnostics.h>

#include "CBP_LBDB.h"

namespace cbpipe {
    
    struct CBP_ImageDBExporter::Impl
    {
        Impl(CBP_ImageDBExporter *parent)
        {
            _parent = parent;
            
            exportTypesToString[EXPORT_TYPE_LABELS] = "labels";
            exportTypesToString[EXPORT_TYPE_EDGES] = "edges";
            exportTypesToString[EXPORT_TYPE_SIAMESE] = "siamese";
            exportTypesToString[EXPORT_TYPE_TRIAMESE] = "triamese";
            exportTypesToString[EXPORT_TYPE_FULL] = "full";
            exportTypesToString[EXPORT_TYPE_EDGE_ANGLES] = "edge-angles";
            
            //replicate the other way
            for(auto const& keyVal : exportTypesToString) {
                stringsToExportTypes[keyVal.second.c_str()] = keyVal.first;
            }
        }
        ~Impl() {
            
        }
        
        CBP_ImageDBExporter *_parent;
        
        std::map<EXPORT_TYPE, std::string>exportTypesToString;
        std::map<const char *, EXPORT_TYPE>stringsToExportTypes;
        
        
        void _generate_labels_training_data(LMDB &trainDB,
                                        LMDB &testDB,
                                        const std::string &debugPath,
                                        const std::vector<std::string> &allImagePaths,
                                        const std::vector<std::string> &trainLabels,
                                        const cv::Size &gridUnitSize,
                                        bool isRGB,
                                        int testCountIn100,
                                        int &trainCount,
                                        int &testCount,
                                        int &maxLabel) {
            
            /*
             CAFFE format:
             
             /path/to/folder/image1.jpg LABEL
             /path/to/folder/image2.jpg 3
             
             */
            
            maxLabel = int(trainLabels.size());
            
            for (auto &imagePath : allImagePaths) {
                
                auto fileName = Directory::get_filename(imagePath);
                
                auto parentDir = Directory::parent_dir(imagePath);
                bool canUseEdges = (parentDir.find("_edges") != std::string::npos);
                
                size_t pos = fileName.find_first_of("_");
                auto classDir = fileName.substr(0, pos);
                
                std::cerr << "Exporting image " << fileName << " class " << classDir << ": " << canUseEdges << std::endl;
                
                auto frame = CBP_MLUtility::generateFrame(imagePath, false);
                
                cv::Point2f paintPoint = CBP_MLUtility::getTrainingPaintPoint(frame->getTrainingMask());
                std::vector<cv::Point2f> points = {paintPoint};
                
                auto grid = CBP_MLUtility::createGrid(frame, gridUnitSize, cv::Size(640,640), points);
                CBP_MLUtility::makePositiveNegativeMask(grid);
                
                //(datapath)/(classdir)/(imagefile).(jpg|png|etc)
                
                if (!debugPath.empty()) {
                    auto debugImagePath = string_sprintf("%s/%s", debugPath.c_str(), fileName.c_str());
                    std::cerr << "Saving debug image " << debugImagePath << std::endl;
                    
                    std::vector<cv::Mat> matches;
                    cv::Mat debugImage = CBP_MLUtility::getDebugImage(grid, matches);
                    
                    cv::imwrite(debugImagePath, debugImage);
                }

                int label = int(std::find(trainLabels.begin(), trainLabels.end(), classDir) - trainLabels.begin());
                
                std::cerr << "Label: " << label << ": " << classDir << std::endl;
                
                for (int y=0; y<grid->positivesMask.rows; y++) {
                    for (int x=0; x<grid->positivesMask.cols; x++) {
                        
                        cv::Point gridIndex = cv::Point(x, y);
                        
                        uchar isPositiveExample = grid->positivesMask.at<uchar>(gridIndex);
                        uchar isNegativeExample = grid->negativesMask.at<uchar>(gridIndex);
                        
                        if (isPositiveExample || (isNegativeExample && canUseEdges)) {
                            cv::Rect roi = cv::Rect(x * grid->gridUnitSize.width,
                                                    y * grid->gridUnitSize.height,
                                                    grid->gridUnitSize.width, grid->gridUnitSize.height);
                            
                            cv::Mat subImage = frame->uprightRGBImage()(roi);
                            //populate for FloodFill
                            
                            int randomNumber = rand() % 100;
                            bool isTestImage = randomNumber < testCountIn100;
                            
                            if (isTestImage) {
                                testCount ++;
                            } else {
                                trainCount ++;
                            }
                            
#if HAVE_CAFFE
                            caffe::Datum datum;
                            testDB.CVMatToDatum(subImage, &datum);
                            
                            datum.set_label(label);

                            std::string out;
                            datum.SerializeToString(&out);

                            
                            if (isTestImage) {
                                std::string key_str = caffe::format_int(testCount-1, 8);
                                testDB.appendData(key_str, out, true);
                            } else {
                                std::string key_str = caffe::format_int(trainCount-1, 8);
                                trainDB.appendData(key_str, out, true);
                            }
#endif
                        }
                    }
                }
            }
        }
        
        void _generate_edges_training_data(LMDB &trainDB,
                                       LMDB &testDB,
                                       const std::string &debugPath,
                                       const std::vector<std::string> &allImagePaths,
                                       const cv::Size &gridUnitSize,
                                       bool isRGB,
                                       int testCountIn100,
                                       int &trainCount,
                                       int &testCount,
                                       int &maxLabel) {
            
            std::map<std::string, int> indexes;
            
            for (auto &imagePath : allImagePaths) {
                
                auto fileName = Directory::get_filename(imagePath);
                
                auto parentDir = Directory::parent_dir(imagePath);
                bool canUseEdges = (parentDir.find("_edges") != std::string::npos);
                
                size_t pos = fileName.find_first_of("_");
                auto classDir = fileName.substr(0, pos);
                
                std::cerr << "Exporting image " << fileName << " class " << classDir << ": " << canUseEdges << std::endl;
                
                auto frame = CBP_MLUtility::generateFrame(imagePath, false);
                
                cv::Point2f paintPoint = CBP_MLUtility::getTrainingPaintPoint(frame->getTrainingMask());
                std::vector<cv::Point2f> points = {paintPoint};
                
                auto grid = CBP_MLUtility::createGrid(frame, gridUnitSize, cv::Size(640,640), points);
                CBP_MLUtility::makePositiveNegativeMask(grid);
                
                //(datapath)/(classdir)/(imagefile).(jpg|png|etc)
                
                if (!debugPath.empty()) {
                    auto debugImagePath = string_sprintf("%s/%s", debugPath.c_str(), fileName.c_str());
                    std::cerr << "Saving debug image " << debugImagePath << std::endl;
                    
                    std::vector<cv::Mat> matches;
                    cv::Mat debugImage = CBP_MLUtility::getDebugImage(grid, matches);
                    
                    cv::imwrite(debugImagePath, debugImage);
                }
                
                for (int y=0; y<grid->positivesMask.rows; y++) {
                    for (int x=0; x<grid->positivesMask.cols; x++) {
                        
                        cv::Rect roi = cv::Rect(x * grid->gridUnitSize.width,
                                                y * grid->gridUnitSize.height,
                                                grid->gridUnitSize.width, grid->gridUnitSize.height);
                        
                        
                        cv::Mat subImage;
                        if (isRGB) {
                            subImage = frame->getRGBImage()(roi);
                        } else {
                            subImage = frame->getBWImage()(roi);
                        }
                        
                        //populate for FloodFill
                        
                        int randomNumber = rand() % 100;
                        bool isTestImage = randomNumber < testCountIn100;
                        
                        if (isTestImage) {
                            testCount ++;
                        } else {
                            trainCount ++;
                        }
#if HAVE_CAFFE
                        cv::Point gridIndex = cv::Point(x, y);
                        bool isPositiveExample = grid->positivesMask.at<uchar>(gridIndex);
                        bool isNegativeExample = grid->negativesMask.at<uchar>(gridIndex);
                        
                        caffe::Datum datum;
                        testDB.CVMatToDatum(subImage, &datum);
                        if (isNegativeExample) {
                            datum.set_label(2);
                        } else if (isPositiveExample) {
                            datum.set_label(1);
                        } else {
                            datum.set_label(0);
                        }

                        
                        std::string out;
                        datum.SerializeToString(&out);
                        
                        if (isTestImage) {
                            std::string key_str = caffe::format_int(testCount-1, 8);
                            testDB.appendData(key_str, out, true);
                        } else {
                            std::string key_str = caffe::format_int(trainCount-1, 8);
                            trainDB.appendData(key_str, out, true);
                        }
#endif
                    }
                }
            }
        }
        
        void _generate_siamese_training_data(LMDB &trainDB,
                                         LMDB &testDB,
                                         const std::string &debugPath,
                                         const std::vector<std::string> &allImagePaths,
                                         const cv::Size &gridUnitSize,
                                         bool isRGB,
                                         int positiveCount,
                                         int testCountIn100,
                                         int &trainCount,
                                         int &testCount,
                                         int &maxLabel) {
            
            for (auto &imagePath : allImagePaths) {
                
                auto fileName = Directory::get_filename(imagePath);
                
                auto parentDir = Directory::parent_dir(imagePath);
                bool canUseEdges = (parentDir.find("_edges") != std::string::npos);
                
                size_t pos = fileName.find_first_of("_");
                auto classDir = fileName.substr(0, pos);
                
                std::cerr << "Exporting image " << fileName << " class " << classDir << ": " << canUseEdges << std::endl;
                
                for (int f=0; f<3; f++) {
                    
                    auto frame = CBP_MLUtility::generateFrame(imagePath, f);
                    
                    cv::Point2f paintPoint = CBP_MLUtility::getTrainingPaintPoint(frame->getTrainingMask());
                    std::vector<cv::Point2f> points = {paintPoint};
                    
                    auto grid = CBP_MLUtility::createGrid(frame, gridUnitSize, cv::Size(640,640), points);
                    CBP_MLUtility::makePositiveNegativeMask(grid);
                    
                    std::vector<texture_sample*>sampleStack = grid->seeds;
                    
                    std::vector<cv::Mat> positiveImageSets, negativeImageSets;
                    
                    cv::Mat positivesDebug = cv::Mat::zeros(grid->gridDimensions.height, grid->gridDimensions.width, CV_8UC1);
                    cv::Mat negativesDebug = cv::Mat::zeros(grid->gridDimensions.height, grid->gridDimensions.width, CV_8UC1);
                    
                    CBP_FloodFillIterator iterator;
                    
                    while (sampleStack.size()) {
                        texture_sample sample = *sampleStack.back();
                        
                        sampleStack.pop_back();
                        
                        std::vector<texture_sample*>newSamples;
                        if (iterator.iterativeAnalysis(grid, sample, newSamples,
                                                       [this, grid, isRGB, &positiveImageSets, &negativeImageSets, &positivesDebug, positiveCount, &negativesDebug]
                                                       (const texture_sample & a, texture_sample & b)
                                                       {
                                                           std::vector<cv::Mat>planes;
                                                           bool isPositiveExample = grid->positivesMask.at<uchar>(b.gridIndex);
                                                           
                                                           std::vector<cv::Rect>positiveROIs;
                                                           
                                                           const texture_sample *sample = &a;
                                                           for (int i=0; i<positiveCount; i++) {
                                                               
                                                               cv::Rect roi = cv::Rect(sample->gridIndex.x * grid->gridUnitSize.width,
                                                                                       sample->gridIndex.y * grid->gridUnitSize.height,
                                                                                       grid->gridUnitSize.width, grid->gridUnitSize.height);
                                                               positiveROIs.push_back(roi);
                                                               
                                                               if (sample->sourceIndex.x >= 0) {
                                                                   sample = &grid->rows[sample->sourceIndex.y].samples[sample->sourceIndex.x];
                                                               }
                                                           }
                                                           
                                                           
                                                           
                                                           cv::Rect roiB = cv::Rect(b.gridIndex.x * grid->gridUnitSize.width,
                                                                                    b.gridIndex.y * grid->gridUnitSize.height,
                                                                                    grid->gridUnitSize.width, grid->gridUnitSize.height);
                                                           
                                                           for (const auto &roi : positiveROIs) {
                                                               std::vector<cv::Mat>planesA;
                                                               cv::split(grid->srcImage(roi), planesA);
                                                               planes.insert(planes.end(), planesA.begin(), planesA.end());
                                                           }
                                                           
                                                           //negative/positive example lane
                                                           if (isRGB) {
                                                               std::vector<cv::Mat>planesB;
                                                               cv::split(grid->srcImage(roiB), planesB);
                                                               planes.insert(planes.end(), planesB.begin(), planesB.end());
                                                           } else {
                                                               planes.push_back(grid->srcImage(roiB));
                                                           }
                                                           
                                                           b.isOn = isPositiveExample;
                                                           
                                                           cv::Mat imageN;
                                                           cv::merge(planes, imageN);
                                                           
                                                           if (isPositiveExample) {
                                                               positivesDebug.at<uchar>(b.gridIndex) = 255;
                                                               positiveImageSets.push_back(imageN);
                                                           } else {
                                                               negativesDebug.at<uchar>(b.gridIndex) = 255;
                                                               negativeImageSets.push_back(imageN);
                                                           }
                                                           
                                                           return b.isOn;
                                                       })) {
                                                           
                                                           sampleStack.insert(sampleStack.begin(), newSamples.begin(), newSamples.end());
                                                       }
                    }
                    
                    
                    
                    //debug
                    if (!debugPath.empty()) {
                        auto debugImagePath = string_sprintf("%s/%d_%s", debugPath.c_str(), f, fileName.c_str());
                        std::cerr << "Saving debug image " << debugImagePath << std::endl;
                        
                        std::vector<cv::Mat>matches = {positivesDebug, negativesDebug};
                        cv::Mat debugImage = CBP_MLUtility::getDebugImage(grid, matches);
                        
                        cv::imwrite(debugImagePath, debugImage);
                    }

                    std::cerr << " Created " << positiveImageSets.size() << " positive examples and " << negativeImageSets.size() << " negative" << std::endl;
                    
#if HAVE_CAFFE
                    int randomNumber = rand() % 100;
                    bool isTestImage = randomNumber < testCountIn100;
                    for (const auto &imagePair : positiveImageSets) {
                        
                        if (isTestImage) {
                            testCount ++;
                        } else {
                            trainCount ++;
                        }
                        

                        caffe::Datum datum;
                        testDB.CVMatToDatum(imagePair, &datum);
                        
                        //is positive example
                        datum.set_label(1);
                        
                        std::string out;
                        datum.SerializeToString(&out);
                        
                        if (isTestImage) {
                            std::string key_str = caffe::format_int(testCount-1, 8);
                            testDB.appendData(key_str, out, true);
                        } else {
                            std::string key_str = caffe::format_int(trainCount-1, 8);
                            trainDB.appendData(key_str, out, true);
                        }
                    }
                    
                    for (const auto &imagePair : negativeImageSets) {
                        
                        if (isTestImage) {
                            testCount ++;
                        } else {
                            trainCount ++;
                        }

                        caffe::Datum datum;
                        testDB.CVMatToDatum(imagePair, &datum);
                        
                        //is negative example
                        datum.set_label(0);
                        
                        std::string out;
                        datum.SerializeToString(&out);
                        
                        if (isTestImage) {
                            std::string key_str = caffe::format_int(testCount-1, 8);
                            testDB.appendData(key_str, out, true);
                        } else {
                            std::string key_str = caffe::format_int(trainCount-1, 8);
                            trainDB.appendData(key_str, out, true);
                        }
                        
                    }
#endif
                }
            }
        }
        
        void _generate_full_training_data(LMDB &trainDB,
                                      LMDB &testDB,
                                      const std::string &debugPath,
                                      const std::vector<std::string> &allImagePaths,
                                      const cv::Size &gridUnitSize,
                                      bool isRGB,
                                      int testCountIn100,
                                      int &trainCount,
                                      int &testCount,
                                      int &maxLabel) {
            
            //db.testRW(lmdbPath, 1000000);
            
            std::map<std::string, int> indexes;
            
            for (auto &imagePath : allImagePaths) {
                
                auto fileName = Directory::get_filename(imagePath);
                
                auto parentDir = Directory::parent_dir(imagePath);
                bool canUseEdges = (parentDir.find("_edges") != std::string::npos);
                
                size_t pos = fileName.find_first_of("_");
                auto classDir = fileName.substr(0, pos);
                
                std::cerr << "Exporting image " << fileName << " class " << classDir << ": " << canUseEdges << std::endl;
                
                auto frame = CBP_MLUtility::generateFrame(imagePath, false);
                cv::Point2f paintPoint = CBP_MLUtility::getTrainingPaintPoint(frame->getTrainingMask());
                std::vector<cv::Point2f> points = {paintPoint};
                
                auto grid = CBP_MLUtility::createGrid(frame, gridUnitSize, cv::Size(640,640), points);
                CBP_MLUtility::makePositiveNegativeMask(grid);
                
                //(datapath)/(classdir)/(imagefile).(jpg|png|etc)
                
                if (!debugPath.empty()) {
                    auto debugImagePath = string_sprintf("%s/%s", debugPath.c_str(), fileName.c_str());
                    std::cerr << "Saving debug image " << debugImagePath << std::endl;
                    
                    std::vector<cv::Mat> matches;
                    cv::Mat debugImage = CBP_MLUtility::getDebugImage(grid, matches);
                    
                    cv::imwrite(debugImagePath, debugImage);
                }

                cv::Mat subImage;
                cv::resize(isRGB ? frame->uprightRGBImage() : frame->uprightBWImage(), subImage, gridUnitSize);
                
                std::vector<cv::Mat>planes;
                cv::split(subImage, planes);
                
                cv::Mat mask;
                cv::resize(frame->getTrainingMask(), mask, gridUnitSize);
                planes.push_back(mask);
                
                cv::merge(planes, subImage);//last is mask
                
#if HAVE_CAFFE
                int randomNumber = rand() % 100;
                bool isTestImage = randomNumber < testCountIn100;
                caffe::Datum datum;
                testDB.CVMatToDatum(subImage, &datum);
                
                std::string out;
                datum.SerializeToString(&out);
                
                if (isTestImage) {
                    testCount ++;
                    std::string key_str = caffe::format_int(testCount-1, 8);
                    testDB.appendData(key_str, out, true);
                } else {
                    trainCount ++;
                    std::string key_str = caffe::format_int(trainCount-1, 8);
                    trainDB.appendData(key_str, out, true);
                }
#endif
            }
        }
        
        cv::Point _get_x_edge(const cv::Mat &maskSubImage, bool existingValue, int yPos) {
            
            cv::Point point;
            int count = 0;
            for (int xPos=1; xPos<maskSubImage.cols; xPos++) {
                bool pixelOn = maskSubImage.at<uchar>(yPos,xPos);
                if (pixelOn != existingValue) {
                    point = cv::Point(xPos, yPos);
                    existingValue = !existingValue;
                    count ++;
                }
            }
            
            if (count == 1) {
                return point;
            }
            
            return cv::Point(-1,-1);
        }
        
        cv::Point _get_y_edge(const cv::Mat &maskSubImage, bool existingValue, int xPos) {
            cv::Point point;
            int count = 0;
            for (int yPos=1; yPos<maskSubImage.rows; yPos++) {
                bool pixelOn = maskSubImage.at<uchar>(yPos,xPos);
                if (pixelOn != existingValue) {
                    point = cv::Point(xPos, yPos);
                    existingValue = !existingValue;
                    count ++;
                }
            }
            
            if (count == 1) {
                return point;
            }
            
            return cv::Point(-1,-1);
        }
        
        float _angle_of_points(const cv::Point2f &pointA, const cv::Point2f &pointB) {
            double deltaX = pointA.x - pointB.x;
            double deltaY = pointA.y - pointB.y;
            double angleInRadians = atan2f(deltaY, deltaX);
            if (angleInRadians < 0) angleInRadians = angleInRadians + M_PI + M_PI;
            return angleInRadians;
        }
        
        float _get_mask_angle(const cv::Mat &maskSubImage) {
            
            bool hasTopLeft = maskSubImage.at<uchar>(0, 0);
            bool hasTopRight = maskSubImage.at<uchar>(0, maskSubImage.cols - 1);
            bool hasBottomLeft = maskSubImage.at<uchar>(maskSubImage.rows-1, 0);
            bool hasBottomRight = maskSubImage.at<uchar>(maskSubImage.rows-1, maskSubImage.cols - 1);
            
            std::vector<cv::Point> points;
            
            if (hasTopLeft != hasTopRight) {
                auto edgePoint = _get_x_edge(maskSubImage, hasTopLeft, 0);
                if (edgePoint.x >= 0) {
                    points.push_back(edgePoint);
                }
            }
            if (hasBottomLeft != hasBottomRight) {
                auto edgePoint = _get_x_edge(maskSubImage, hasBottomLeft, maskSubImage.rows-1);
                if (edgePoint.x >= 0) {
                    points.push_back(edgePoint);
                }
            }
            
            if (points.size() < 2 && hasTopLeft != hasBottomLeft) {
                auto edgePoint = _get_y_edge(maskSubImage, hasTopLeft, 0);
                if (edgePoint.x >= 0) {
                    points.push_back(edgePoint);
                }
            }
            
            if (points.size() < 2 && hasTopRight != hasBottomRight) {
                auto edgePoint = _get_y_edge(maskSubImage, hasTopRight, maskSubImage.cols-1);
                if (edgePoint.x >= 0) {
                    points.push_back(edgePoint);
                }
            }
            
            if (points.size() == 2) {
                //std::cerr << points[0] << points[1] << std::endl;
                return _angle_of_points(points[0], points[1]);
            }
            
            return -1;
        }
        
        int _angle_to_label(float radians, int numAngles) {
            
            //calculate
            float angle = toDegrees(radians);
            
            float denom = 180.0 / numAngles;
            
            return int(round(angle / denom)) % numAngles;
        }
        
        void _generate_edge_angles_Training_data(LMDB &trainDB,
                                            LMDB &testDB,
                                            const std::string &debugPath,
                                            const std::vector<std::string> &allImagePaths,
                                            const cv::Size &gridUnitSize,
                                            bool isRGB,
                                            int testCountIn100,
                                            int &trainCount,
                                            int &testCount,
                                            int &maxLabel) {
            
            for (auto &imagePath : allImagePaths) {
                
                auto fileName = Directory::get_filename(imagePath);
                
                auto parentDir = Directory::parent_dir(imagePath);
                bool canUseEdges = (parentDir.find("_edges") != std::string::npos);
                
                size_t pos = fileName.find_first_of("_");
                auto classDir = fileName.substr(0, pos);
                
                std::cerr << "Exporting image " << fileName << " class " << classDir << ": " << canUseEdges << std::endl;
                
                auto frame = CBP_MLUtility::generateFrame(imagePath, false);
                cv::Point2f paintPoint = CBP_MLUtility::getTrainingPaintPoint(frame->getTrainingMask());
                std::vector<cv::Point2f> points = {paintPoint};
                
                auto grid = CBP_MLUtility::createGrid(frame, gridUnitSize, cv::Size(640,640), points);
                CBP_MLUtility::makePositiveNegativeMask(grid);
                
                //(datapath)/(classdir)/(imagefile).(jpg|png|etc)
                
                if (!debugPath.empty()) {
                    auto debugImagePath = string_sprintf("%s/%s", debugPath.c_str(), fileName.c_str());
                    std::cerr << "Saving debug image " << debugImagePath << std::endl;
                    
                    std::vector<cv::Mat> matches;
                    cv::Mat debugImage = CBP_MLUtility::getDebugImage(grid, matches);
                    
                    cv::imwrite(debugImagePath, debugImage);
                }
                
                for (int y=0; y<grid->positivesMask.rows; y++) {
                    for (int x=0; x<grid->positivesMask.cols; x++) {
                        
                        cv::Point gridIndex = cv::Point(x, y);
                        
                        bool isPositiveExample = grid->positivesMask.at<uchar>(gridIndex);
                        bool isNegativeExample = grid->negativesMask.at<uchar>(gridIndex);
                        
                        if (!isPositiveExample && !isNegativeExample) continue;
                        
                        cv::Rect roi = cv::Rect(x * grid->gridUnitSize.width,
                                                y * grid->gridUnitSize.height,
                                                grid->gridUnitSize.width, grid->gridUnitSize.height);
                        
                        cv::Mat subImage;
                        if (isRGB) {
                            subImage = frame->getRGBImage()(roi);
                        } else {
                            subImage = frame->getBWImage()(roi);
                        }
                        
                        //populate for FloodFill

#if HAVE_CAFFE
                        int randomNumber = rand() % 100;
                        bool isTestImage = randomNumber < testCountIn100;
                        caffe::Datum datum;
                        testDB.CVMatToDatum(subImage, &datum);
                        
                        if (isNegativeExample) {
                            //get angle
                            cv::Mat maskSubImage = frame->trainingMask(roi);
                            
                            if (cv::countNonZero(maskSubImage) > (4 * roi.area() / 5)) continue;//too full
                            
                            float radians = getMaskAngle(maskSubImage);
                            
                            if (radians == -1) continue;
                            
                            int label = 1 + angleToLabel(radians, 4);
                            
                            datum.set_label(label);
                            
                            //Diagnostics::SaveDiagnosticMask(maskSubImage, "label_%d_%f.png", label, toDegrees(radians));
                            
                        } else if (isPositiveExample) {
                            datum.set_label(0);
                        }
                        
                        if (isTestImage) {
                            testCount ++;
                        } else {
                            trainCount ++;
                        }
                        
                        std::string out;
                        datum.SerializeToString(&out);
                        
                        if (isTestImage) {
                            std::string key_str = caffe::format_int(testCount-1, 8);
                            testDB.appendData(key_str, out, true);
                        } else {
                            std::string key_str = caffe::format_int(trainCount-1, 8);
                            trainDB.appendData(key_str, out, true);
                        }
                        
#endif
                    }
                }
            }
        }
        
    };
    
    
    CBP_ImageDBExporter::CBP_ImageDBExporter() {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }
    
    CBP_ImageDBExporter::~CBP_ImageDBExporter() {
        
    }
    
    std::string CBP_ImageDBExporter::exportTypeToString(CBP_ImageDBExporter::EXPORT_TYPE type) {
        return m_pImpl->exportTypesToString[type];
    }
    
    CBP_ImageDBExporter::EXPORT_TYPE CBP_ImageDBExporter::stringToExportType(std::string typeString) {
        return m_pImpl->stringsToExportTypes[typeString.c_str()];
    }
    
    void CBP_ImageDBExporter::exportTrainingImages(const std::string &trainingPath, const std::string &exportPath, const std::string &debugPath,
                                                   cv::Size gridUnitSize, bool isRGB, EXPORT_TYPE type, std::string oredFilter) {
        
        auto typeString = exportTypeToString(type);
        
        auto logPath = string_sprintf("%s/logging", exportPath.c_str());
        if (Directory::exists(logPath.c_str())) {
            Directory::remove_directory(logPath.c_str(), true);
        }
        Diagnostics::Initialize(logPath);
        
        
        std::cerr << "############################################" << std::endl;
        std::cerr << "### Exporting Images with squares " << gridUnitSize.width << " by " << gridUnitSize.height;
        std::cerr << " in " << (isRGB ? "FULL COLOR" : "GRAYSCALE") << std::endl;
        
        std::cerr << "### Type: " << typeString << std::endl;
        if (oredFilter.length()) {
            std::cerr << "### Filtered by " << oredFilter << std::endl;
        }
        std::cerr << "############################################" << std::endl;
        
        auto rootPath = string_sprintf("%s/%s", exportPath.c_str(), typeString.c_str());
        auto trainExportPath = string_sprintf("%s/%s_%s", rootPath.c_str(), typeString.c_str(), "train");
        auto testExportPath = string_sprintf("%s/%s_%s", rootPath.c_str(), typeString.c_str(), "test");
        
        if (!debugPath.empty()) {
            if (Directory::exists(debugPath.c_str())) {
                Directory::remove_directory(debugPath.c_str(), true);
            }
            Directory::mkpath(debugPath.c_str());
        }
        
        auto allImagePaths = CBP_MLUtility::getTrainingImages(trainingPath, oredFilter);
        
        std::random_shuffle (allImagePaths.begin(), allImagePaths.end());
                
        std::vector<std::string> trainLabels;
        
        //get labels
        for (const auto &imagePath : allImagePaths) {
            
            auto fileName = Directory::get_filename(imagePath);
            
            size_t pos = fileName.find_first_of("_");
            auto classDir = fileName.substr(0, pos);
            
            if (std::find(trainLabels.begin(), trainLabels.end(), classDir) == trainLabels.end()) {
                trainLabels.push_back(classDir);
            }
        }
        
        std::cerr << "Exporting " << allImagePaths.size() << " training images with " << trainLabels.size() << " labels" << std::endl;
        
        std::cerr << "Labels found:";
        for (auto &label : trainLabels) {
            std::cerr << " " << label;
        }
        std::cerr << std::endl << std::endl << std::endl;
        
        std::sort(trainLabels.begin(), trainLabels.end());
        
        srand ((unsigned)time(NULL));
        
        int testCountIn100 = 20;
        
        LMDB trainDB, testDB;
        
        auto lmdbTrainPath = string_sprintf("%s_lmdb", trainExportPath.c_str());
        
        std::cerr << "Opening LMDB at path " << lmdbTrainPath << std::endl;
        
        
        auto lmdbTestPath = string_sprintf("%s_lmdb", testExportPath.c_str());
        
        std::cerr << "Opening LMDB at path " << lmdbTestPath << std::endl;
        
        if (Directory::exists(lmdbTrainPath.c_str())) {
            Directory::remove_directory(lmdbTrainPath.c_str(), true);
        }
        
        if (Directory::exists(lmdbTestPath.c_str())) {
            Directory::remove_directory(lmdbTestPath.c_str(), true);
        }
        
        trainDB.open(lmdbTrainPath);
        trainDB.beginTransaction();
        
        testDB.open(lmdbTestPath);
        testDB.beginTransaction();
        
        
        std::cerr << "Creating " << typeString << " training and testing images." << std::endl;
        
        int trainCount = 0;
        int testCount = 0;
        int numOutputs = 0;
        
        switch (type) {
            case EXPORT_TYPE_LABELS:
                m_pImpl->_generate_labels_training_data(trainDB, testDB, debugPath, allImagePaths, trainLabels, gridUnitSize,
                                                    isRGB, testCountIn100, trainCount, testCount, numOutputs);
                break;
                
            case EXPORT_TYPE_EDGES:
                m_pImpl->_generate_edges_training_data(trainDB, testDB, debugPath, allImagePaths, gridUnitSize,
                                                   isRGB, testCountIn100, trainCount, testCount, numOutputs);
                break;
                
            case EXPORT_TYPE_SIAMESE:
                m_pImpl->_generate_siamese_training_data(trainDB, testDB, debugPath, allImagePaths, gridUnitSize,
                                                     isRGB, 1, testCountIn100, trainCount, testCount, numOutputs);
                break;
            case EXPORT_TYPE_TRIAMESE:
                m_pImpl->_generate_siamese_training_data(trainDB, testDB, debugPath, allImagePaths, gridUnitSize,
                                                     isRGB, 2, testCountIn100, trainCount, testCount, numOutputs);
                break;
                
            case EXPORT_TYPE_FULL:
                m_pImpl->_generate_full_training_data(trainDB, testDB, debugPath, allImagePaths, gridUnitSize,
                                                  isRGB, testCountIn100, trainCount, testCount, numOutputs);
                break;
                
            case EXPORT_TYPE_EDGE_ANGLES:
                m_pImpl->_generate_edge_angles_Training_data(trainDB, testDB, debugPath, allImagePaths, gridUnitSize,
                                                        isRGB, testCountIn100, trainCount, testCount, numOutputs);
                break;
            default:
                break;
        }
        
        
        trainDB.commitTransaction();
        trainDB.close();
        
        testDB.commitTransaction();
        testDB.close();
        
        std::cerr << "Created " << trainCount << " training images and " << testCount;
        std::cerr << " testing images with "  << numOutputs << " labels." << std::endl;
        
    }
};
