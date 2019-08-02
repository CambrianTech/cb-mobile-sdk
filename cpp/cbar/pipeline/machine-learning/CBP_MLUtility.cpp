//
//  CBP_MLUtility.cpp
//  Cambrian
//
//  Created by Joel Teply on 1/5/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#include "CBP_MLUtility.hpp"

#include <utility/Directory.h>
#include <utility/Diagnostics.h>
#include <utility/CommonUtility.h>

#define ROW_PTR(img, y) ((uchar*)((img).data + (img).step * y))

namespace cbpipe {
    
    bool addGaussianNoise(const cv::Mat mSrc, cv::Mat &mDst,double Mean=0.0, double StdDev=10.0)
    {
        cv::Mat mSrc_16SC;
        
        cv::Mat mGaussian_noise = cv::Mat(mSrc.size(), mSrc.channels() == 3 ? CV_16SC3 : CV_16SC1);
        randn(mGaussian_noise,cv::Scalar::all(Mean), cv::Scalar::all(StdDev));
        
        mSrc.convertTo(mSrc_16SC, mSrc.channels() == 3 ? CV_16SC3 : CV_16SC1);
        addWeighted(mSrc_16SC, 1.0, mGaussian_noise, 1.0, 0.0, mSrc_16SC);
        mSrc_16SC.convertTo(mDst,mSrc.type());
        
        return true;
    }
    
    
    void augmentImage(const cv::Mat &image, cv::Mat &new_image) {
        if (new_image.empty()) {
            new_image = image.clone();
        }
        
        double noise = double(rand() % 15);
        
        addGaussianNoise(image, new_image, 0.0, noise);
        
        double alpha = double(85 + (rand() % 30)) / 100.0;
        double beta = (rand() % 30) - 15;
        
        int channels = MIN(3, image.channels());
        for( int y = 0; y < image.rows; y++ ) {
            for( int x = 0; x < image.cols; x++ ) {
                for( int c = 0; c < channels; c++ ) {
                    new_image.at<cv::Vec3b>(y,x)[c] =
                    cv::saturate_cast<uchar>( alpha*( image.at<cv::Vec3b>(y,x)[c] ) + beta );
                }
            }
        }
    }
    
    bool importMaskedPNG(cv::Mat &largeImage, cv::Mat &alphaMask) {
        int alphaCount = 0;
        //int totalPixels = largeImage.cols * largeImage.rows;
        alphaMask = cv::Mat(largeImage.rows, largeImage.cols, CV_8UC1);
        bool hasAlpha = false;
        
        for(int y=0; y<largeImage.rows; y++) {
            uchar *imageRowPtr = ROW_PTR(largeImage, y);
            uchar *alphaRowPtr = ROW_PTR(alphaMask, y);
            for(int x=0, x3=0, x4=0; x<largeImage.cols; x++, x3+=3, x4+=4) {
                int alphaValue = imageRowPtr[x4 + 3];
                if (alphaValue == 255) {
                    //one
                    alphaRowPtr[x] = 255;
                }
                else {
                    //zero
                    alphaCount ++;
                    alphaRowPtr[x] = 0;
                    
                    imageRowPtr[x4+3] = 255;
                }
            }
        }
        
        hasAlpha = alphaCount > 2 * (largeImage.cols + largeImage.rows);
        return hasAlpha;
    }
    
    cbar::CBAR_VideoFramePtr CBP_MLUtility::generateFrame(const std::string &imagePath, bool _augmentImage) {
        
        cv::Mat rgbaImage = cv::imread(imagePath, cv::IMREAD_UNCHANGED);
        
        if (rgbaImage.channels() != 4) {
            //std::cerr << std::endl << ERROR_BEGIN << imagePath << " is only " << rgbaImage.channels()
            //<< " channels, aborting! " << ERROR_END << std::endl;
            exit(0);
        }
        
        cv::Mat mask;
        importMaskedPNG(rgbaImage, mask);
        
        if (_augmentImage) {
            cv::Mat rgbImage;
            cv::cvtColor(rgbaImage, rgbImage, CV_RGBA2RGB);
            augmentImage(rgbImage, rgbImage);
            cv::cvtColor(rgbImage, rgbaImage, CV_RGB2RGBA);
        }
        
        cbar::CBAR_VideoFramePtr frame = new cbar::CBAR_VideoFrame(rgbaImage, 0, false, false);
        frame->setTrainingMask(mask);
        
        return frame;
    }
    
    void CBP_MLUtility::makePositiveNegativeMask(cv::Ptr<texture_grid> grid) {
        return makePositiveNegativeMask(grid, grid->positivesMask, grid->negativesMask);
    }
    
    void CBP_MLUtility::makePositiveNegativeMask(const cv::Ptr<texture_grid> grid,
                                                 cv::Mat &positivesMask,
                                                 cv::Mat &negativesMask) {
        
        positivesMask = cv::Mat::zeros(grid->gridDimensions.height, grid->gridDimensions.width, CV_8UC1);
        negativesMask = positivesMask.clone();
        //cv::Mat ambiguousMask = positivesMask.clone();
        
        for (int y=0; y<grid->positivesMask.rows; y++) {
            for (int x=0; x<grid->positivesMask.cols; x++) {
                cv::Point pointIndex = cv::Point(x, y);
                
                auto roi = cv::Rect(pointIndex.x * grid->gridUnitSize.width,
                                    pointIndex.y * grid->gridUnitSize.height,
                                    grid->gridUnitSize.width, grid->gridUnitSize.height);
                
                auto value = cv::mean(grid->trainingMask(roi));
                
                if (value[0] == 255) {
                    positivesMask.at<uchar>(pointIndex) = 255;
                } else if (value[0] > 50) {
                    negativesMask.at<uchar>(pointIndex) = 255;
                } else if (value[0] > 0) {
                    //ambiguousMask.at<uchar>(pointIndex) = 255;
                }
            }
        }
        
        //    cv::resize(mask, negativesMask, gridSize, 0,0, cv::INTER_NEAREST);
        //            cv::pyrUp(negativesMask, negativesMask);
        //            //cv::dilate(negativesMask, negativesMask, cv::getStructuringElement(CV_SHAPE_RECT,cv::Size(2,2)));
        //            cv::pyrDown(negativesMask, negativesMask);
    }
    
    cv::Point CBP_MLUtility::indexForPoint(cv::Ptr<texture_grid> grid, const cv::Point &point) {
        cv::Point result;
        
        result.x = MIN(grid->gridDimensions.width-1, MAX(0, (point.x + grid->gridUnitSize.width / 2) / grid->gridUnitSize.width));
        result.y = MIN(grid->gridDimensions.height-1, MAX(0, (point.y + grid->gridUnitSize.height / 2) / grid->gridUnitSize.height));
        
        return result;
    }
    
    cv::Point CBP_MLUtility::pointForIndex(cv::Ptr<texture_grid> grid, const cv::Point &index) {
        return cv::Point(index.x * grid->gridUnitSize.width, index.y * grid->gridUnitSize.height);
    }
    
    cv::Point CBP_MLUtility::getTrainingPaintPoint(const cv::Mat &mask) {
        
        cv::Moments m = cv::moments(mask, false);
        cv::Point indexPoint(m.m10/m.m00, m.m01/m.m00);
        
        //auto posCount = cv::countNonZero(positivesMask);
        //auto negCount = cv::countNonZero(negativesMask);
        
        if (indexPoint.x < 0 || indexPoint.y < 0
            || indexPoint.x >= mask.cols || indexPoint.y >= mask.rows) {
            return cv::Point(mask.cols/2, mask.rows/2);
        }
        
        if (!mask.at<uchar>(indexPoint)) {
            int startX = mask.cols / 5;
            int endX = 4 * mask.cols / 5;
            int startY = mask.rows / 2;
            int endY = 3 * mask.rows / 4;
            for (int y=startY; y<endY; y++) {
                for (int x=startX; x<endX; x++) {
                    if (mask.at<uchar>(y,x)) {
                        return cv::Point(x,y);
                    }
                }
            }
            
            cv::Point bestPoint;
            int totalCount = 0;
            for (int y=0; y<mask.rows; y++) {
                for (int x=startX; x<mask.cols; x++) {
                    auto point = cv::Point(x, y);
                    if (mask.at<uchar>(point)) {
                        bestPoint = point;
                        totalCount ++;
                        
                        if (totalCount > 10) {
                            return bestPoint;
                        }
                    }
                }
            }
        }
        
        return indexPoint;
    }
    
    void CBP_MLUtility::debugResults(cv::Ptr<texture_grid> grid, cv::Mat &debugOutput, const cv::Mat &results, const cv::Scalar &color, bool filled) {
        
        for (int y=0; y<results.rows; y++) {
            for (int x=0; x<results.cols; x++) {
                cv::Point pointIndex = cv::Point(x, y);
                
                int label = results.at<uchar>(pointIndex);
                if (!label) continue;
                
                int size = filled ? grid->gridUnitSize.width/4 : grid->gridUnitSize.width-3;
                auto origin = cv::Point(pointIndex.x * grid->gridUnitSize.width + (grid->gridUnitSize.width - size) / 2,
                                        pointIndex.y * grid->gridUnitSize.height + (grid->gridUnitSize.height - size) / 2);
                
                if (label == 255) {
                    cv::rectangle(debugOutput, origin, origin + cv::Point(size, size),
                                  color, filled ? cv::FILLED : 1);
                } else {
                    std::string labelText = string_sprintf("%d", label);
                    cv::putText(debugOutput, labelText, origin, cv::FONT_HERSHEY_PLAIN, 1.2, color, 1, cv::LINE_AA);
                }
            }
        }
    }
    
    cv::Mat CBP_MLUtility::getDebugImage(cv::Ptr<texture_grid> grid, std::vector<cv::Mat> &matches) {
        
        cv::Mat debugOutput, resultsImage;
        debugOutput = grid->srcImage.clone();
        resultsImage = cv::Mat::zeros(grid->srcImage.rows, grid->srcImage.cols, CV_8UC3);
        grid->srcImage.copyTo(resultsImage);
        
        //printf("Found %lu matches\n", matches.size());
        //            {
        //                std::vector<std::vector<cv::Point> > contours;
        //                std::vector<cv::Vec4i> hierarchy;
        //                cv::findContours(grid->frame->getUtilizedMask(), contours, hierarchy, CV_RETR_LIST, CV_LINK_RUNS, cv::Point() );
        //                cv::drawContours(debugOutput, contours, 0,  cv::Scalar::all(255), 3);
        //            }
        
        auto positiveColor = cv::Scalar(0,255,0);
        auto negativeColor = cv::Scalar(0,0,255);
        
        debugResults(grid, debugOutput, grid->positivesMask, positiveColor, false);
        debugResults(grid, debugOutput, grid->negativesMask, negativeColor, false);
        
        int index = 0;
        cv::RNG& rng=cv::theRNG();
        for (const auto& match : matches) {
            cv::Scalar randomColor = index == 0 ? cv::Scalar(200,255,20) : cv::Scalar(rng.uniform(0,255), rng.uniform(0, 255), rng.uniform(0, 255));
            debugResults(grid, resultsImage, match, randomColor, true);
            
            index++;
        }
        
        for (auto &seed : grid->seeds) {
            cv::circle(	resultsImage, seed->center, 20, cv::Scalar::all(255), 3);
        }
        
        //output text
        //string_sprintf("Training Image")
        cv::putText(debugOutput, "Training", cv::Point(50, debugOutput.rows - 50), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255,255,255), 2, cv::LINE_AA);
        cv::putText(resultsImage, "Prediction", cv::Point(50, debugOutput.rows - 50), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255,255,255), 2, cv::LINE_AA);
        
        cv::hconcat(debugOutput, resultsImage, debugOutput);
        
        return debugOutput;
    }
    
    cv::Ptr<texture_grid> CBP_MLUtility::createGrid(cbar::CBAR_VideoFramePtr frame,
                                                    const cv::Size &gridUnitSize, const cv::Size & maxImageSize,
                                                    const std::vector<cv::Point2f> &points) {
        
        cv::Mat rgbImage = frame->uprightRGBImage();
        double scale = 1.0;
                
        scale = fmin(maxImageSize.width / double(rgbImage.cols), maxImageSize.height / double(rgbImage.rows));
        
        if (scale < 1.0) {
            cv::resize(rgbImage, rgbImage, cv::Size(scale * double(rgbImage.cols), scale * double(rgbImage.rows)));
        } else {
            scale = 1.0;
        }

        cv::Ptr<texture_grid> newGrid = new texture_grid(rgbImage, gridUnitSize);
        
        if (!frame->getTrainingMask().empty()) {
            cv::resize(frame->getTrainingMask(), newGrid->trainingMask, rgbImage.size());
        }
        
        newGrid->rows.resize(newGrid->gridDimensions.height);
        
        cv::Size halfSize = newGrid->gridUnitSize / 2;
        
        int cols = rgbImage.cols / gridUnitSize.width;
        int rows = rgbImage.rows / gridUnitSize.height;
        
        for (int i=0; i<rows; i++) {
            texture_row &row = newGrid->rows[i];
            row.samples.resize(newGrid->gridDimensions.width);
            for (int j=0; j<cols; j++) {
                texture_sample &sample = newGrid->rows[i].samples[j];
                sample.gridIndex = cv::Point(j, i);
                
                sample.roi = cv::Rect(j * newGrid->gridUnitSize.width,
                                      i * newGrid->gridUnitSize.height,
                                      newGrid->gridUnitSize.width,
                                      newGrid->gridUnitSize.height);
                
                sample.roi = sample.roi & cv::Rect(0,0, rgbImage.cols, rgbImage.rows);
                
                sample.center = cv::Point(sample.roi.x + halfSize.width, sample.roi.y + halfSize.height);
                
                sample.size = newGrid->gridUnitSize;
            }
        }
        
        if (points.size()) {
            std::vector<cv::Point2f> rotatedPaintPoints;
            
            for (const auto &point : points) {
                rotatedPaintPoints.push_back(imaging::Geometry::rotatePoint90n(point, frame->getFrameRotation(), frame->frameSize()));
            }
            
            std::vector<cv::Point2f> paintPoints;
            for (const auto &rotatedPoint : rotatedPaintPoints) {
                auto truePoint = cv::Point2f(rotatedPoint.x * scale, rotatedPoint.y * scale);
                paintPoints.push_back(truePoint);
                //printf("point=%fx%f\n", truePoint.x, truePoint.y);
            }
            setGridSeeds(newGrid, paintPoints);
        }
        
        
        
        return newGrid;
    }
    
    void CBP_MLUtility::setGridSeeds(cv::Ptr<texture_grid> grid, const std::vector<cv::Point2f> &paintPoints) {
        //Set seed points
        for (int i=0; i<paintPoints.size(); i++) {
            cv::Point index = indexForPoint(grid, paintPoints[i]);
            auto &seed = grid->rows[index.y].samples[index.x];
            seed.isSeed = true;
            seed.isOn = true;
            
            //printf("Seed is at %d,%d\n", index.y, index.x);
            grid->seeds.push_back(&seed);
        }
    }
    
    std::vector<std::string> CBP_MLUtility::getTrainingImages(const std::string &trainingDirectory, std::string oredFilter) {
        
        std::vector<std::string> imagePaths;
        std::vector<std::string> trainingDirectories = Directory::list(trainingDirectory.c_str(), true, false);
        
        if (!trainingDirectories.empty()) {
            for (auto &directory : trainingDirectories) {
                auto fullPath = string_sprintf("%s/%s", trainingDirectory.c_str(), directory.c_str());
                auto subImages = getTrainingImages(fullPath, oredFilter);
                if (!subImages.empty()) {
                    imagePaths.insert(imagePaths.end(), subImages.begin(), subImages.end());
                }
            }
        }
        
        std::vector<std::string> filters;
        
        std::stringstream ss(oredFilter.c_str()); // Turn the string into a stream.
        std::string tok;
        
        while(getline(ss, tok, ',')) {
            filters.push_back(tok);
        }
        
        std::vector<std::string> trainingFiles = Directory::list(trainingDirectory.c_str(), false, true);
        
        if (trainingFiles.empty()) {
            std::cerr << "Input directory '" << trainingDirectory << "' has no files" << std::endl;
            return imagePaths;
        }
        
        for (auto &filename : trainingFiles) {
            
            auto fileExtension = Directory::get_file_extension(filename);
            std::transform(fileExtension.begin(), fileExtension.end(), fileExtension.begin(), ::tolower);
            
            if (fileExtension != "png") continue;
            
            //filter
            bool skip = filters.size();
            for (const auto &filter : filters) {
                auto match = filename.substr(0, filter.size());
                if (filename.substr(0, filter.size()) == filter) {
                    skip = false;
                    break;
                }
            }
            
            if (!skip) {
                char fullPath[1000];
                sprintf(fullPath, "%s/%s", trainingDirectory.c_str(), filename.c_str());
                
                //printf("importing image: %s (%dx%d)\n", filename.c_str(), image.cols, image.rows);
                imagePaths.push_back(fullPath);
            }
        }
        
        std::random_shuffle (imagePaths.begin(), imagePaths.end());
        
        return imagePaths;
    }
    
    void CBP_MLUtility::getMaxClass(const cv::Mat &probBlob, int *classId, double *classProb) {
        cv::Mat probMat = probBlob.reshape(1, 1); //reshape the blob to 1x1000 matrix
        cv::Point classNumber;
        cv::minMaxLoc(probMat, NULL, classProb, NULL, &classNumber);
        *classId = classNumber.x;
    }
};
