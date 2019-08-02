//
//  ImageProcessing.cpp
//  Cambrian
//
//  Created by Joel Teply on 6/15/12.
//
//

#include "ImageProcessing.h"
#include "Imaging.h"
#include "Coloring.h"
#include "Drawing.h"
#include "Geometry.h"
#include <cbcommon/CB_Config.h>
#include <imaging/Accelerated.h>
#include <imaging/ImageDefines.h>

#include <utility/Threads.h>

#if __APPLE__
#import <Accelerate/Accelerate.h>
#endif

#define LOG_FILL 0

using namespace std;

namespace imaging {
    
    void ImageProcessing::getContourAndArea(const cv::Mat &mask, std::vector<cv::Point> &contour, double &contourArea) {
        
        std::vector<std::vector<cv::Point> > contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(mask, contours, hierarchy, CV_RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        if (contours.size()) {
            std::vector<cv::Point> polygon;
            cv::approxPolyDP(cv::Mat(contours[0]), polygon, 9, true);
            
            contourArea = cv::contourArea(polygon);
            
            if (contourArea > 0) {
                contour = polygon;
            } else {
                contour = contours[0];
                contourArea = cv::contourArea(contours[0]);
            }
        }
    }
    
    //#define ROW_PTR(img, y) ((uchar*)((img).data + (img).step * y))
    cv::Mat ImageProcessing::normalMap(const cv::Mat& bwTexture, double pStrength)
    {
        // assume square texture, not necessarily true in real code
        int scale = 1.0;
        int delta = 127;
        
        cv::Mat sobelZ, sobelX, sobelY;
        cv::Sobel(bwTexture, sobelX, CV_8U, 1, 0, 13, scale, delta, cv::BORDER_DEFAULT);
        cv::Sobel(bwTexture, sobelY, CV_8U, 0, 1, 13, scale, delta, cv::BORDER_DEFAULT);
        sobelZ = cv::Mat(bwTexture.rows, bwTexture.cols, CV_8UC1);
        
        for(int y=0; y<bwTexture.rows; y++) {
            const uchar *sobelXPtr = ROW_PTR(sobelX, y);
            const uchar *sobelYPtr = ROW_PTR(sobelY, y);
            uchar *sobelZPtr = ROW_PTR(sobelZ, y);
            
            for(int x=0; x<bwTexture.cols; x++) {
                double Gx = double(sobelXPtr[x]) / 255.0;
                double Gy = double(sobelYPtr[x]) / 255.0;
                
                double Gz =  pStrength * sqrt(0.5 + Gx * Gx + Gy * Gy);
                
                uchar value = uchar(Gz * 127.0);
                
                sobelZPtr[x] = value;
            }
        }
        
        std::vector<cv::Mat>planes;
        
        planes.push_back(sobelX);
        planes.push_back(sobelY);
        planes.push_back(sobelZ);
        
        cv::Mat normalMap;
        cv::merge(planes, normalMap);
        
        cv::Mat originalNormalMap = normalMap.clone();
        
        cv::Mat normalMapBlurred;
        
        for (int i=0; i<3; i++) {
            cv::GaussianBlur(normalMap, normalMapBlurred, cv::Size(13, 13), 5, 5);
            addWeighted(normalMap, 0.5, normalMapBlurred, 0.5, 0, normalMap);
        }
        addWeighted(originalNormalMap, 0.5, normalMapBlurred, 0.5, 0, normalMap);
        
        return normalMap;
    }
    
    void ImageProcessing::drawSobel(const cv::Mat &imgBW, cv::Mat &sobel) {
        int scale = 1;
        int delta = 0;
        
        cv::Mat sobelAdded, sobelX, sobelY;
        cv::Sobel(imgBW, sobelX, CV_8U, 1, 0, 3, scale, delta, cv::BORDER_DEFAULT);
        cv::Sobel(imgBW, sobelY, CV_8U, 0, 1, 3, scale, delta, cv::BORDER_DEFAULT);
        addWeighted(sobelX, 0.5, sobelY, 0.5, 0, sobelAdded);
        
        cv::bilateralFilter(sobelAdded, sobel, 11, 30, 80);
    }
    
    void ImageProcessing::freiChen(const cv::Mat &imgBW, cv::Mat &dest, double intensity) {
        static const float kernelData[9][9] = {
            {0.3535533906, 0.5, 0.3535533906, 0.0, 0.0, 0.0, -0.3535533906, -0.5, -0.3535533906},
            {0.3535533906, 0.0, -0.3535533906, 0.5, 0.0, -0.5, 0.3535533906, 0.0, -0.3535533906},
            {0.0, -0.3535533906, 0.5, 0.3535533906, 0.0, -0.3535533906, -0.5, 0.3535533906, 0.0},
            {0.5, -0.3535533906, 0.0, -0.3535533906, 0.0, 0.3535533906, 0.0, 0.3535533906, -0.5},
            {0.0, 0.5, 0.0, -0.5, 0.0, -0.5, 0.0, 0.5, 0.0},
            {-0.5, 0.0, 0.5, 0.0, 0.0, 0.0, 0.5, 0.0, -0.5},
            {0.1666666667, -0.3333333334, 0.1666666667, -0.3333333334, 0.6666666668, -0.3333333334, 0.1666666667, -0.3333333334, 0.1666666667},
            {-0.3333333334, 0.1666666667, -0.3333333334, 0.1666666667, 0.6666666668, 0.1666666667, -0.3333333334, 0.1666666667, -0.3333333334},
            {0.3333333333, 0.3333333333, 0.3333333333, 0.3333333333, 0.3333333333, 0.3333333333, 0.3333333333, 0.3333333333, 0.3333333333}
        };
        
        
        cv::Mat bw32;
        imgBW.convertTo(bw32, CV_32FC1, 1.0f/255.0f);
        
        cv::Mat M = cv::Mat(imgBW.rows, imgBW.cols, bw32.depth());
        cv::Mat S = cv::Mat(imgBW.rows, imgBW.cols, bw32.depth());
        
        cv::Mat temp = cv::Mat(imgBW.rows, imgBW.cols, bw32.depth());
        
        cv::Mat kern(3, 3, bw32.depth());
        float* tp = kern.ptr<float>(0);
        for (int i=0; i<9; i++) {
            for (int j = 0; j < 9; j++) {
                tp[j] = kernelData[i][j];
            }
            temp = conv2(bw32, kern);
            cv::pow(temp, 2, temp);
            
            if (i<4) {
                M += temp;
            } else {
                if (i==4) S = M + temp;
                else S += temp;
            }
            
        }
        
        cv::sqrt(M/S, dest);
        
        dest.convertTo(dest, CV_8UC1, intensity * 255.0f);
    }
    
    void ImageProcessing::retinex(const cv::Mat &img, cv::Mat &imgOut, double amount) {
        //a large, weak gaussian blur of the image, subtracted from the image
        //cv::Mat gauss = cv::Mat(img.rows, img.cols, CV_32F);
        
        cv::Mat img32;
        img.convertTo(img32, CV_32F);
        cv::Mat gauss;
        cv::GaussianBlur(img32, gauss,  cv::Size(31, 31), 1, 1, cv::BORDER_DEFAULT);
        
        double gain = 1.0f/(1.0f - amount);
        img32 = gain * (img32 - (amount * gauss));
        
        img32.convertTo(imgOut, CV_8U);
    }
    
    //Next after lines:
    //http://cmm.ensmp.fr/~beucher/wtshed.html
    //connect lines http://blog.ayoungprogrammer.com/2013/04/tutorial-detecting-multiple-rectangles.html
    
    cv::Mat ImageProcessing::getEdgesFromBWImage(cv::Mat imgBW, CBSample sampleInfo)
    {
        if (sampleInfo.hsvDeviation.val[2] > 6) {
            cv::blur(imgBW, imgBW, cv::Size(2,2));
        }
        // detect edges
        
        cv::Mat imgReturn;
        cv::Canny(imgBW, imgReturn, 20, 250);
        
        //make sure canny hasn't gotten out of control
        cv::Scalar seedAvgHsv, seedStdHsv;
        
        Imaging::meanStdDevAtPoint(imgReturn, seedAvgHsv, seedStdHsv,
                                   cv::Point(sampleInfo.origin.x, sampleInfo.origin.y));
        
        //destroy canny pixels
        if (seedAvgHsv.val[0] > 3) {
            imgReturn = cv::Scalar::all(0);
        }
        cv::equalizeHist(imgBW, imgBW);
        
        drawContours(imgBW, imgReturn, COLOR_WHITE);
        
        cv::rectangle(imgReturn,
                      cv::Point(sampleInfo.origin.x - SEED_RADIUS, sampleInfo.origin.y - SEED_RADIUS),
                      cv::Point(sampleInfo.origin.x + SEED_RADIUS, sampleInfo.origin.y + SEED_RADIUS),
                      cv::Scalar::all(0), CV_FILLED);
        
        return imgReturn;
    }
    
    vector<vector<cv::Point> > ImageProcessing::findContours(cv::Mat greyscale, int threshold, int maxThreshold)
    {
        
        //cv::Canny(greyscale, greyscale, 120, 250);
        if (threshold || maxThreshold) {
            cv::threshold(greyscale, greyscale, threshold, maxThreshold, CV_THRESH_BINARY);
        }
        
        vector<vector<cv::Point> > contours;
        vector<cv::Vec4i> hierarchy;
        
        cv::findContours( greyscale, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
        
        
        return contours;
    }
    
    vector<vector<cv::Point> > ImageProcessing::getPolygonsForContours(vector<vector<cv::Point> > contours, double epsilon)
    {
        const size_t contoursSize = contours.size();
        vector<vector<cv::Point> > polygons( contoursSize );
        vector<vector<cv::Point> > filteredPolygons;
        
        //fast c style pointer access:
        for(int i = 0; i < contoursSize; i++) {
            //CBLog("approxPolyDP epsilon %f", epsilon);
            cv::approxPolyDP( cv::Mat(contours[i]), polygons[i], epsilon, true );
            filteredPolygons.push_back(polygons[i]);
        }
        
        return filteredPolygons;
    }
    
    vector<vector<cv::Point> > ImageProcessing::findPolygons(cv::Mat srcBW, double epsilon, int threshold, int maxThreshold)
    {
        vector<vector<cv::Point> > contours = findContours(srcBW, threshold, maxThreshold);
        
        if (contours.size()) {
            
            return getPolygonsForContours(contours, epsilon);
            
        }
        
        return vector<vector<cv::Point> >();
    }
    
    void ImageProcessing::drawContours(const cv::Mat &srcBW, cv::Mat &destImage, cv::Scalar color, int thickness, int threshold, int maxThreshold)
    {
        vector<vector<cv::Point> > contours = findContours(srcBW, threshold, maxThreshold);
        
        drawContours(contours , destImage, color, thickness);
    }
    
    void ImageProcessing::drawContours(const std::vector<std::vector<cv::Point> > &contours, cv::Mat &destImage, cv::Scalar color, int thickness)
    {
        if (contours.size()) {
            
            const size_t contoursSize = contours.size();
            
            bool randomizeColor = color[0] == -1;
            
            for(int i = 0; i < contoursSize; i++)
            {
                vector<cv::Point> points = contours[i];
                
                const size_t pointsSize = points.size();
                
                if (pointsSize > 1) {
                    cv::Point lastContourPoint;
                    lastContourPoint = points[0];
                    
                    for(int j = 1; j < pointsSize; j++)
                    {
                        cv::Point currentContourPoint = points[j];
                        
                        //cv::line(destImage, currentContourPoint, lastContourPoint, color);
                        
                        if (!(currentContourPoint.x == lastContourPoint.x
                              && (currentContourPoint.x < 5 || currentContourPoint.x > destImage.cols - 5))
                            && !(currentContourPoint.y == lastContourPoint.y
                                 && (currentContourPoint.y < 5 || currentContourPoint.y > destImage.rows - 5))) {
                                
                                if (randomizeColor) {
                                    //random
                                    color = Coloring::randomColor();
                                    //CBLog("color=%f", color[0]);
                                }
                                
                                cv::line(destImage, currentContourPoint, lastContourPoint, color, thickness);
                                //cv::circle(destImage, currentContourPoint, thickness+1, color);
                            }
                        lastContourPoint = currentContourPoint;
                    }
                }
                
            }
        }
    }
    
    void ImageProcessing::drawPolygons(const cv::Mat &srcBW, cv::Mat &destImage, cv::Scalar color, int thickness,
                                       double epsilon, int threshold, int maxThreshold)
    {
        vector<vector<cv::Point> > contours = findPolygons(srcBW, epsilon, threshold, maxThreshold);
        
        drawContours(contours , destImage, color);
    }
    
    int ImageProcessing::numContourPoints(const std::vector<std::vector<cv::Point> > &contours)
    {
        int totalLength = 0;
        if (contours.size()) {
            
            const size_t contoursSize = contours.size();
            
            for(int i = 0; i < contoursSize; i++)
            {
                vector<cv::Point> points = contours[i];
                totalLength += points.size();
            }
        }
        
        return totalLength;
    }
    
    double ImageProcessing::contourArea(const std::vector<std::vector<cv::Point> > &contours)
    {
        double totalArea = 0;
        if (contours.size()) {
            
            const size_t contoursSize = contours.size();
            
            for(int i = 0; i < contoursSize; i++)
            {
                vector<cv::Point> points = contours[i];
                totalArea += cv::contourArea(points);
            }
        }
        
        return totalArea;
    }
    
    typedef enum
    {
        FloodSizeTypeTooSmall,
        FloodSizeTypeVerySmall,
        FloodSizeTypeSmall,
        FloodSizeTypeMedium,
        FloodSizeTypeLarge,
        FloodSizeTypeTooBig,
    } FloodSizeType;
    
    int ImageProcessing::createMaskFromPaintPoint(const cv::Mat &src,
                                                  cv::Mat &mask,
                                                  cv::Point2f seedPoint,
                                                  int *utilizedThreshold,
                                                  int minThreshold,
                                                  int maxThreshold,
                                                  bool fast,
                                                  int maxIterations,
                                                  std::vector<LineSegment>* linesFound)
    {
        cv::Mat srcImage;
        if (src.channels() == 4) {
            cv::cvtColor(src, srcImage, CV_RGBA2RGB);
        } else {
            srcImage = src;
        }
        
        vector<cv::Point2f>seedPoints;
        seedPoints.push_back(seedPoint);
        return createMaskFromMultiplePaintPoints(srcImage, mask, seedPoints, utilizedThreshold, minThreshold, maxThreshold, fast, maxIterations, linesFound);
    }
    
#if LOG_FILL
#define MaskLog(fmt, ...) { printf(fmt, ##__VA_ARGS__); printf("\n"); }
#else
#define MaskLog(fmt, ...) { }
#endif
    
    int ImageProcessing::createMaskFromMultiplePaintPoints(const cv::Mat &srcImage,
                                                           cv::Mat &mask,
                                                           const vector<cv::Point2f>&seedPointsIn,
                                                           int *utilizedThreshold,
                                                           int minThreshold,
                                                           int maxThreshold,
                                                           bool fast,
                                                           int maxIterations,
                                                           std::vector<LineSegment>* linesFound)
    {
        //cv::Size maskSize = cv::Size(histogram.cols + 2, histogram.rows + 2);
        //cv::Mat imgMask = cv::Mat(maskSize, histogram.depth(), 1);
        vector<cv::Point2f>seedPoints;
        
        for (int i=0; i<seedPointsIn.size(); i++) {
            if (seedPointsIn[i].x >= 0 && seedPointsIn[i].x < srcImage.cols
                && seedPointsIn[i].y >= 0 && seedPointsIn[i].y < srcImage.rows) {
                seedPoints.push_back(seedPointsIn[i]);
            }
        }
        
        int totalFill = 0;
        
        cv::Mat imgMask = cv::Mat::zeros(srcImage.rows + 2, srcImage.cols + 2, CV_8UC1);
        
        int insensitivity = minThreshold;
        
        if (fast || minThreshold == maxThreshold) {
            int threshold = (3 * minThreshold + maxThreshold) / 4;//weighted towards min
            for (int i=0; i<seedPoints.size(); i++) {
                
                applyFloodFill(srcImage,
                               imgMask,
                               seedPoints[i],
                               COLOR_WHITE,
                               cv::Scalar::all(threshold),
                               true);
            }
        }
        else {
            //Search for it
            
            //start it at 2
            int totalPixels = imgMask.cols * imgMask.rows;
            
            vector<int> areaHistory;
            vector<int> sensitivityHistory;
            vector<int> pointsHistory;
            //double lastAvg = 0;
            double lastStdDev = 0;
            
            cv::Mat lastMask;
            int lastSensitivity = 0;
            
            FloodSizeType lastFloodSizeType = FloodSizeTypeVerySmall;
            
            for (int i=0; i<maxIterations; i++) {
                
                FloodSizeType floodSizeType;
                //int lastArea = 0;
                if (i > 0) {
                    for (int j=0; j<sensitivityHistory.size() - 1; j++) {
                        if (sensitivityHistory[j] == insensitivity) {
                            MaskLog("insensitivity %i already used, going back to %i", insensitivity, lastSensitivity);
                            if (i > 1) {
                                imgMask = lastMask;
                            }
                            insensitivity = lastSensitivity;
                            goto done;
                        }
                    }
                    lastSensitivity = insensitivity;
                    // lastArea = areaHistory[areaHistory.size() - 1];
                    lastMask = imgMask.clone();
                }
                
                imgMask = cv::Scalar::all(0);
                MaskLog("floodFill %i", insensitivity);
                totalFill = 0;
                
                for (int i=0; i<seedPoints.size(); i++) {
                    totalFill += applyFloodFill(srcImage,
                                                imgMask,
                                                seedPoints[i],
                                                COLOR_WHITE,
                                                cv::Scalar::all(insensitivity), true);
                }
                
                areaHistory.push_back(totalFill);
                sensitivityHistory.push_back(insensitivity);
                
                cv::Scalar avg, stddev; //, imgMask(cv::Rect(1,1,histogram.cols-2, histogram.rows-2))
                cv::meanStdDev(srcImage, avg, stddev, imgMask(cv::Rect(1,1,srcImage.cols, srcImage.rows)));
                double avgStdDev = (stddev[0] + stddev[1] + stddev[2]) / 3.0f;
                
#if LOG_FILL
                double avgMean = (avg[0] + avg[1] + avg[2]) / 3.0f;
                MaskLog("AVG:%f STD: %f", avgMean, avgStdDev);
#endif
                
                double deltaStddev = i > 0 ? avgStdDev / lastStdDev : 0;
                //double deltaMean = i > 0 ? avgMean / lastAvg : 0;
                
                floodSizeType = FloodSizeTypeVerySmall;
                
                bool topCovered = imgMask.at<uchar>(0, imgMask.cols / 2) > 0
                && imgMask.at<uchar>(0, 0) > 0 && imgMask.at<uchar>(0, imgMask.cols-1) > 0;
                //bool bottomCovered = mask.at<uchar>(mask.cols / 2, mask.rows -1) > 0;
                
                if (topCovered || totalFill > 0.7 * double(totalPixels)) {
                    floodSizeType = FloodSizeTypeTooBig;
                } else if (totalFill > 0.4 * double(totalPixels)) {
                    floodSizeType = FloodSizeTypeLarge;
                } else if (totalFill > 0.2 * double(totalPixels)) {
                    floodSizeType = FloodSizeTypeMedium;
                } else if (totalFill > 0.1 * double(totalPixels)) {
                    floodSizeType = FloodSizeTypeSmall;
                } else if (totalFill < 0.01 * double(totalPixels)) {
                    floodSizeType = FloodSizeTypeTooSmall;
                }
                
                cv::Mat test;
                cv::blur(imgMask, test, cv::Size(4,4));
                std::vector<std::vector<cv::Point> >polygons = ImageProcessing::findPolygons(test);
                
                std::vector<LineSegment>longLines;
                if (polygons.size()) {
                    LineProcessing::getLongLinesInContour(polygons[0], longLines, imgMask.size());
                }
                
                size_t totalLines = longLines.size();
                if (totalLines && linesFound) {
                    linesFound->insert(linesFound->end(), longLines.begin(), longLines.end());
                }
                
                int numPoints = ImageProcessing::numContourPoints(polygons);
                int lastNumPoints = pointsHistory.size() > 0 ? pointsHistory[pointsHistory.size() - 1] : 0;
                pointsHistory.push_back(numPoints);
                
                if ((totalLines < 15 && totalLines > 7) && floodSizeType > FloodSizeTypeSmall && floodSizeType < FloodSizeTypeTooBig) {
                    goto done;
                }
                else if (floodSizeType == FloodSizeTypeTooBig && insensitivity > minThreshold) {
                    insensitivity--;
                    MaskLog("Too much area %i, down to %i", totalFill, insensitivity);
                }
                else if ((floodSizeType == FloodSizeTypeTooSmall)
                         && (insensitivity < maxThreshold || numPoints < 2)) {
                    if (numPoints < 2) {
                        insensitivity += 2;
                    } else {
                        insensitivity ++;
                    }
                    insensitivity = MIN(insensitivity, maxThreshold);
                    
                    MaskLog("Too small an area %i, Up to %i", totalFill, insensitivity);
                }
                else if (deltaStddev > 0 && lastFloodSizeType > FloodSizeTypeTooSmall
                         && insensitivity > (minThreshold + 1)
                         && ((floodSizeType > FloodSizeTypeSmall && deltaStddev > 1.0)
                             || (floodSizeType > FloodSizeTypeTooSmall && deltaStddev > 1.8))) {
                             
                             insensitivity --;
                             
                             if (totalFill > 0.3 * double(totalPixels) && deltaStddev > 1.2) {
                                 MaskLog("Too much deviation growth %f, down to %i (%i > %i)",
                                         deltaStddev, insensitivity,
                                         totalFill, int(0.3 * double(totalPixels)));
                             } else {
                                 MaskLog("Too much deviation growth %f, down to %i (%i > %i)",
                                         deltaStddev, insensitivity,
                                         totalFill, int(0.1 * double(totalPixels)));
                             }
                             
                         }
                else if (deltaStddev > 0 && deltaStddev < 0.7 && insensitivity < maxThreshold) {
                    insensitivity ++;
                    MaskLog("Change in deviation small %f, Up to %i", deltaStddev, insensitivity);
                }
                else {
                    
                    bool tooManyPoints = numPoints > 500;
                    bool lastTooManyPoints = lastNumPoints > 0 && lastNumPoints > 500;
                    
                    if (i > 0 &&
                        (lastNumPoints < 1.5f * double(numPoints))
                        && !lastTooManyPoints
                        && lastFloodSizeType > FloodSizeTypeSmall) {
                        imgMask = lastMask;
                        insensitivity = lastSensitivity;
                        MaskLog("Too many points, but last were ok, %i", lastFloodSizeType);
                        goto done;
                    }
                    else if (!tooManyPoints && lastNumPoints > 0 && numPoints < lastNumPoints) {
                        goto done;
                    }
                    else if (insensitivity < maxThreshold) {
                        insensitivity ++;
                    }
                }
                
                lastFloodSizeType = floodSizeType;
                //lastAvg = avgMean;
                lastStdDev = avgStdDev;
            }
        }
        
    done:
        
        //        if (threshold) {
        //            *threshold = insensitivity;
        //        }
        MaskLog("Used %i from %d to %d", insensitivity, minThreshold, maxThreshold);
        
        //LOG_IMAGE(srcImage, "floodFillInput.png");
        
        cv::Rect roi = cv::Rect(1, 1, imgMask.cols - 2, imgMask.rows - 2);
        mask = imgMask(roi);
        //mask.setTo(cv::Scalar(255), mask);
        
        if (utilizedThreshold)  {
            *utilizedThreshold = insensitivity;
        }
        
        return totalFill;
    }
    
    void
    ImageProcessing::adjustContrast(const cv::Mat &image, cv::Mat &new_image, double alpha, double beta)
    {
        if (new_image.empty()) {
            new_image = cv::Mat(image.rows, image.cols, CV_8UC1);
        }
        /// Do the operation new_image(i,j) = alpha*image(i,j) + beta
        if (image.channels() == 1) {
            for( int y = 0; y < image.rows; y++ )
            {
                for( int x = 0; x < image.cols; x++ )
                {
                    new_image.at<uchar>(y,x) = cv::saturate_cast<uchar>( alpha*( image.at<uchar>(y,x)) + beta );
                }
            }
        }
        else if (image.channels() == 3) {
            for( int y = 0; y < image.rows; y++ )
            {
                for( int x = 0; x < image.cols; x++ )
                {
                    for( int c = 0; c < 3; c++ )
                    {
                        new_image.at<cv::Vec3b>(y,x)[c] =
                        cv::saturate_cast<uchar>( alpha*( image.at<cv::Vec3b>(y,x)[c]) + beta );
                    }
                }
            }
        }
        else if (image.channels() == 4) {
            for( int y = 0; y < image.rows; y++ )
            {
                for( int x = 0; x < image.cols; x++ )
                {
                    for( int c = 0; c < 3; c++ )
                    {
                        new_image.at<cv::Vec4b>(y,x)[c] =
                        cv::saturate_cast<uchar>( alpha*( image.at<cv::Vec4b>(y,x)[c]) + beta );
                    }
                }
            }
        }
        
    }
    
    void
    ImageProcessing::scaleSaturation(const cv::Mat &image, cv::Mat &new_image, double byAmount)
    {
        //inefficient as hell
        cv::Mat hsv;
        cv::Mat rgbImage;
        if (image.channels() == 3) {
            rgbImage = image;
        } else {
            cv::cvtColor(image, rgbImage, CV_RGBA2RGB);
        }
        cv::cvtColor(rgbImage, hsv, CV_RGB2HSV);
        
        /// Do the operation new_image(i,j) = alpha*image(i,j) + beta
        for( int y = 0; y < hsv.rows; y++ )
        {
            for( int x = 0; x < hsv.cols; x++ )
            {
                hsv.at<cv::Vec3b>(y,x)[1] = byAmount * hsv.at<cv::Vec3b>(y,x)[1];
            }
        }
        
        if (image.channels() == 3) {
            cv::cvtColor(image, new_image, CV_HSV2RGB);
        } else {
            cv::Mat temp;
            cv::cvtColor(hsv, temp, CV_HSV2RGB);
            std::vector<cv::Mat>planes;
            cv::split(temp, planes);
            cv::Mat alpha;
            cv::extractChannel(image, alpha, 3);
            planes.push_back(alpha);
            cv::merge(planes, new_image);
        }
    }
    
    void
    ImageProcessing::getPlane(const cv::Mat& img, cv::Mat& dest, int plane)
    {
        int from_to[] = { plane, 0 };
        
        dest.create(img.size(), CV_MAKETYPE(img.depth(), 1));
        cv::mixChannels(&img, 1, &dest, 1, from_to, 1);
    }
    
    bool
    ImageProcessing::hasPixelData(const cv::Mat& img, int inChannel)
    {
        //todo: far more efficicent scanning, exit on first find
        if (inChannel >= 0) {
            cv::Mat plane;
            getPlane(img, plane, inChannel);
            return cv::countNonZero(plane) > 0;
        }
        return cv::countNonZero(img) > 0;
    }
    
    cv::Mat
    ImageProcessing::combine(const cv::Mat& red, const cv::Mat& green, const cv::Mat& blue)
    {
        if (red.rows != green.rows || red.rows!=blue.rows) {
            return cv::Mat();
        }
        if (red.cols != green.cols || red.cols!=blue.cols) {
            return cv::Mat();
        }
        cv::Mat red2;
        cv::Mat green2;
        cv::Mat blue2;
        if (red.type() != CV_8UC1) {
            red.convertTo(red2, CV_8UC1);
        }
        else {
            red2 = red;
        }
        if (green.type() != CV_8UC1) {
            green.convertTo(green2, CV_8UC1);
        }
        else {
            green2 = green;
        }
        if (blue.type() != CV_8UC1) {
            blue.convertTo(blue2, CV_8UC1);
        }
        else {
            blue2 = blue;
        }
        cv::Mat res(red.rows, red.cols, CV_8UC3);
        for (int i = 0; i < red.rows; i++) {
            const uint8_t* r = red2.ptr(i);
            const uint8_t* g = green2.ptr(i);
            const uint8_t* b = blue2.ptr(i);
            uint8_t* tp = res.ptr(i);
            for (int j = 0; j < red.cols; j++) {
                *tp++ = r[j];
                *tp++ = g[j];
                *tp++ = b[j];
            }
        }
        return res;
    }
    
    cv::Scalar ImageProcessing::getPaintDiffFromSamples(const std::vector<CBSample> &samples)
    {
        const size_t sampleSize = samples.size();
        
        cv::Scalar totalDeviation = cv::Scalar::all(0);
        for(int i = 0; i < sampleSize; i++)
        {
            CBSample sample = samples[i];
            
            totalDeviation += cv::Scalar(sample.hsvDeviation);
        }
        cv::Scalar avgDeviation;
        cv::divide(totalDeviation, cv::Scalar::all(sampleSize), avgDeviation);
        
        cv::Scalar lowDiff, upDiff;
        
        //double insensitivity = 4 + avgDeviation.val[0] + avgDeviation.val[1] + avgDeviation.val[2];
        //    double insensitivity = 2 + sqrt(avgDeviation.val[0] + avgDeviation.val[1] + avgDeviation.val[2]);
        //	if (insensitivity < 3.0) insensitivity = 3.0;
        //	if (insensitivity > 12.0) insensitivity = 12.0;
        //    lowDiff = cv::Scalar::all(insensitivity);
        
        cv::Scalar variance;
        cv::multiply(avgDeviation, avgDeviation, variance);
        cv::multiply(variance, cv::Scalar(0.6f, 0.13f, 1.0f), lowDiff);
        
        for (int i=0; i<3; i++) {
            if (lowDiff.val[i] < 3) lowDiff.val[i] = 3;
            else if (lowDiff.val[i] > 13) lowDiff.val[i] = 13;
        }
        
        return lowDiff;
    }
    
    bool isPointWithinImages(const cv::Point &position, const cv::Mat &img, const cv::Mat &mask) {
        cv::Rect rect = cv::Rect(0,0, img.cols, img.rows) & cv::Rect(0,0, mask.cols, mask.rows);
        if (position.x >= 0 && position.y >= 0 && position.x < rect.width && position.y < rect.height) {
            return true;
        }
        return false;
    }
    
    int
    ImageProcessing::applyFloodFill(const cv::Mat &image,
                                    cv::Mat &mask,
                                    const cv::Point &position,
                                    cv::Scalar color,
                                    cv::Scalar diff,
                                    bool search)
    {
        if (mask.empty() || image.empty() || !isPointWithinImages(position, image, mask)) return 0;
        
        cv::Scalar value = mask.cols ? Drawing::getPixelAt(position, mask) : Drawing::getPixelAt(position, image);
        
        if (!value[0] && !value[1] && !value[2]) {
            
            int flags = cv::FLOODFILL_MASK_ONLY | 4 | 1 << 8;
            //cv has a bug, explaining the strange params for the flag here.
            int result = cv::floodFill(image, mask,
                                       position, color, 0,
                                       diff, diff, flags);
            
            if (search) {
                std::vector<cv::Point2f>validTestPoints;
                bool isOk = _isValidFloodFill(image, mask, position, &validTestPoints);
                
                if (!isOk) {
                    for (cv::Point testPoint : validTestPoints) {
                        mask.setTo(cv::Scalar::all(0));
                        result = cv::floodFill(image, mask,
                                               testPoint, color, 0,
                                               diff, diff, flags);
                        if (_isValidFloodFill(image, mask, testPoint)) {
                            return result;
                        }
                    }
                }
            }
            
#if 0
            cv::Scalar mean, stddev;
            Imaging::meanStdDevAtPoint(image, mean, stddev, position, 30);
            cv::Scalar offset = cv::Scalar(40, 40, 40) + cv::Scalar(10 * stddev[0], 10 * stddev[1], 10 * stddev[2]);
            cv::Mat ranged;
            cv::inRange(image, mean - offset, mean + offset, ranged);
            
            ImageProcessing::refineMask(image, ranged, cv::Mat(), ranged,
                                        50, 50, true);
            
            cv::Rect inner = cv::Rect(1,1,ranged.cols, ranged.rows);
            mask(inner) = mask(inner) & ranged;
#endif
            //Diagnostics::SaveDiagnosticImage(ranged, "ranged.png");
            
            return result;
        }
        
        return -1;
    }
    
    bool ImageProcessing::_isValidFloodFill(const cv::Mat &img, const cv::Mat &mask, const cv::Point &position, std::vector<cv::Point2f>*validTestPoints)
    {
        int range = 10;
        std::vector<cv::Point2f>testPoints;
        testPoints.push_back(position + cv::Point(0, -range));
        testPoints.push_back(position + cv::Point(range, 0));
        testPoints.push_back(position + cv::Point(0, range));
        testPoints.push_back(position + cv::Point(-range, 0));
        
        int invalidCount = 0;
        for (cv::Point testPoint : testPoints) {
            if (isPointWithinImages(testPoint, img, mask)) {
                if (validTestPoints) validTestPoints->push_back(testPoint);
                if (Drawing::getSingleChannelPixelAt(testPoint, mask) == 0) {
                    invalidCount ++;
                }
            }
        }
        
        return invalidCount < 2;
    }
    
    class WatershedSegmenter {
    private:
        cv::Mat markers;
    public:
        void setMarkers(const cv::Mat& markerImage){
            markerImage.convertTo(markers, CV_32S);
        }
        
        cv::Mat process(const cv::Mat &image){
            cv::watershed(image,markers);
            return markers;
        }
    };
    
    
    cv::Mat ImageProcessing::createShadowsAndHighlights(const cv::Mat &rgbImg, const cv::Size &destSize) {
        
        //https://mathematica.stackexchange.com/questions/7414/image-shadow-removal-in-mathematica
        cv::Mat working, rgbSmall;
        cv::resize(rgbImg, rgbSmall, destSize);
        //auto start = sys_usec_time();
        cv::pyrMeanShiftFiltering(rgbSmall, working, 20, 50, 1, cv::TermCriteria(cv::TermCriteria::MAX_ITER,3,1) );
        
        //printf("TOOK %f seconds", seconds_elapsed(start));

        cv::cvtColor(working, working, CV_RGB2GRAY);
        
        //cv::resize(textureless, textureless, rgbImg.size());
        //ImageProcessing::rotate_image_90n(test, test, 90);
        //Diagnostics::SaveDiagnosticImage(true, working, "textureless.png");
        
        cv::Mat highlights, shadows;
        
        ImageProcessing::adjustContrast(working, highlights, 1.5, 50);
        ImageProcessing::adjustContrast(working, shadows, 1.5, -100);
        
        cv::GaussianBlur(highlights, highlights, cv::Size(13, 13), 3.0);
        cv::GaussianBlur(shadows, shadows, cv::Size(5, 5), 3.0);
        
        addWeighted(shadows, 0.5, highlights, 0.5, 0.0, working);
        
        cv::equalizeHist(working, working);
        
        cv::GaussianBlur(highlights, highlights, cv::Size(5, 5), 3.0);
        
        //cv::GaussianBlur(highlights, highlights, cv::Size(7, 7), 5.0);
        
//        Diagnostics::SaveDiagnosticImage(true, shadows, "shadows.png");
//        Diagnostics::SaveDiagnosticImage(true, highlights, "highlights.png");
//        Diagnostics::SaveDiagnosticImage(true, working, "combined.png");
        
        //Diagnostics::SaveDiagnosticImage(false, highlights, "highlights.png");
        
        return working;
    }
    
    bool ImageProcessing::getPolygonalMask(const cv::Mat &maskSrc, cv::Mat &maskDest, std::vector<LineSegment>&longLines, double epsilon, int minLength) {
        
        std::vector<std::vector<cv::Point> > simplifiedPoints = ImageProcessing::findPolygons(maskSrc, epsilon);
        
        if (simplifiedPoints.size())  {
            return getPolygonalMask(maskSrc, maskDest, longLines, simplifiedPoints[0], epsilon, minLength);
        }
        
        return false;
    }
    
    bool ImageProcessing::getPolygonalMask(const cv::Mat &maskSrc, cv::Mat &maskDest,
                                           std::vector<LineSegment>&longLines, std::vector<cv::Point> &polygons, double epsilon, int minLength) {
        
        LineProcessing::getLongLinesInContour(polygons, longLines, maskSrc.size(), minLength);
        
        if (longLines.size()) {
            //draw polygon
            cv::Mat polygonMask = cv::Mat::zeros(maskSrc.rows, maskSrc.cols, CV_8UC1);
            std::vector<std::vector<cv::Point> >contours;
            contours.push_back(polygons);
            cv::drawContours(polygonMask, contours, 0,  cv::Scalar::all(255));
            polygons = contours[0];
            
            //fill result
            cv::Point floodPoint = cv::Point(-1,-1);
            for (int i=0; i<polygons.size() && floodPoint.x == -1; i++) {
                cv::Point startPoint = polygons[i];
                for (int i=0; i<4; i++) {
                    cv::Point testPoint;
                    testPoint.x = startPoint.x + (i % 2 ? -5 : 5);
                    testPoint.y = startPoint.y + ((i / 2) ? -5 : 5);
                    bool inside = cv::pointPolygonTest(polygons, testPoint, false) > 0;
                    if (inside) {
                        floodPoint = testPoint;
                        break;
                    }
                }
            }
            
            if (floodPoint.x >= 0) {
                //LOG_IMAGE(reduced, "reduced.png");
                int amount = cv::floodFill(polygonMask, floodPoint, cv::Scalar::all(255));
                
                if (amount > 10) {
                    //cv::erode(polygonMask, polygonMask, cv::getStructuringElement(CV_SHAPE_ELLIPSE,cv::Size(2,2)), cv::Point(-1,-1), 1, cv::BORDER_REPLICATE);
                } else {
                    return false;
                }
            }
            
            if (&maskDest != &maskSrc) {
                maskDest = maskSrc.clone();
            }
            
            //draw lines reel big
            cv::Mat linesMask = cv::Mat::zeros(maskSrc.rows, maskSrc.cols, CV_8UC1);
            for (int i=0; i<longLines.size(); i++) {
                cv::line(linesMask, longLines[i][0], longLines[i][1], cv::Scalar::all(255), 2.0 * epsilon, CV_AA);
                cv::line(maskDest, longLines[i][0], longLines[i][1], cv::Scalar::all(0), 2.0 * epsilon, CV_AA);
            }
            
            polygonMask &= linesMask;
            
            //LOG_IMAGE(polygonMask, "polygonMask.png");
            
            maskDest |= polygonMask;
            
            return true;
        }
        
        return false;
    }
    
    void ImageProcessing::shiftImage(const cv::Mat &srcImage, cv::Mat &_destImage, cv::Point offset) {
        
        if (srcImage.empty()) return;
        
        cv::Mat destImage;
        if (_destImage.data != srcImage.data) {
            destImage = _destImage;
        } else if (!offset.x && !offset.y) {
            return;//nothing to do, src/dest same object, no shift required either
        }
        destImage = cv::Mat::zeros(srcImage.rows, srcImage.cols, srcImage.type());
        
        cv::Rect srcROI(-1 * offset.x, -1 * offset.y, srcImage.cols, srcImage.rows);
        srcROI &= cv::Rect(0,0, srcImage.cols, srcImage.rows);
        cv::Rect destROI(offset.x, offset.y, srcImage.cols, srcImage.rows);
        destROI &= cv::Rect(0,0, srcImage.cols, srcImage.rows);
        
        if (srcROI.area() > 0 && destROI.area() > 0) {
            srcImage(srcROI).copyTo(destImage(destROI));
        }
        
        _destImage = destImage;
    }
    
    void ImageProcessing::_drawStretchRectanglesForPos(cv::Mat &mask, cv::Point startPos) {
        
        bool on = false;
        cv::Rect currentRectangle;
        
        //if statement may have broken this functionality but it prevented a memory issue
        if (startPos.x < 0 || startPos.y < 0 || startPos.x >= mask.cols || startPos.y >= mask.rows) return;
        
        if (startPos.y) { //horizontal, increment x
            if (startPos.y < mask.rows/2) {
                currentRectangle = cv::Rect(0, 0, 0, startPos.y);
            } else {
                currentRectangle = cv::Rect(0, startPos.y, 0, mask.rows - startPos.y);
            }
            
            for (startPos.x=0; startPos.x<mask.cols; startPos.x++) {
                uchar value = mask.at<uchar>(startPos);
                if (value == 255) {
                    if (on) {
                        currentRectangle.width++;
                    } else {
                        on = true;
                        currentRectangle.x = startPos.x;
                        currentRectangle.width = 0;//reset
                    }
                } else if (on) {
                    on = false;
                    //add rectangle
                    cv::rectangle(mask, currentRectangle, cv::Scalar(255), CV_FILLED);
                }
            }
            
        } else { //vertical, increment y
            if (startPos.x < mask.cols/2) {
                currentRectangle = cv::Rect(0, 0, startPos.x, 0);
            } else {
                currentRectangle = cv::Rect(startPos.x, 0, mask.cols - startPos.x, 0);
            }
            
            for (startPos.y=0; startPos.y<mask.rows; startPos.y++) {
                uchar value = mask.at<uchar>(startPos);
                if (value == 255) {
                    if (on) {
                        currentRectangle.height++;
                    } else {
                        on = true;
                        currentRectangle.y = startPos.y;
                        currentRectangle.height = 0;//reset
                    }
                } else if (on) {
                    on = false;
                    //add rectangle
                    cv::rectangle(mask, currentRectangle, cv::Scalar(255), CV_FILLED);
                }
            }
        }
        
        if (on) {
            cv::rectangle(mask, currentRectangle, cv::Scalar(255), CV_FILLED);
        }
    }
    
//    void ImageProcessing::refineMask(const cv::Mat &_reducedMask,
//                                     const cv::Mat &_rgbImage,
//                                     int dilate,
//                                     int erode,
//                                     cv::Mat &dest,
//                                     cv::Size maxSize)
    
    void ImageProcessing::_drawStretchRectangles(cv::Mat &mask, const cv::Point &offset) {
        
        //int dilateAmount = fmax(3, fmin(20, Geometry::vectorMagnitude(offset)));
        //int erodeAmount = fmax(3, fmin(10, Geometry::vectorMagnitude(offset)));
        
        if (offset.x > 0) {
            //moved right, left side needs rectangles
            _drawStretchRectanglesForPos(mask, cv::Point(offset.x + 1, 0));
            //cv::Rect roi = cv::Rect(0, 0, offset.x, mask.rows);
            //refineROI(srcRGB, mask, roi, dilateAmount, erodeAmount);
        } else if (offset.x < 0) {
            //moved left, right side needs rectangles
            _drawStretchRectanglesForPos(mask, cv::Point(mask.cols + offset.x - 1, 0));
            //cv::Rect roi =  cv::Rect(mask.cols + offset.x - 1, 0, -offset.x, mask.rows);
            //refineROI(srcRGB, mask, roi, dilateAmount, erodeAmount);
        }
        
        if (offset.y > 0) {
            //moved down, top needs rectangles
            _drawStretchRectanglesForPos(mask, cv::Point(0, offset.y + 1));
            //cv::Rect roi = cv::Rect(0, 0, mask.cols, offset.y);
            //refineROI(srcRGB, mask, roi, dilateAmount, erodeAmount);
        } else if (offset.y < 0) {
            //moved up, bottom needs rectangles
            _drawStretchRectanglesForPos(mask, cv::Point(0, mask.rows + offset.y - 1));
            //cv::Rect roi = cv::Rect(0, mask.rows + offset.y, mask.cols, -offset.y);
            //refineROI(srcRGB, mask, roi, dilateAmount, erodeAmount);
        }
    }
    
    void ImageProcessing::refineROI(const cv::Mat &srcRGB, cv::Mat &mask, const cv::Rect &roi, int dilate, int erode) {
        cv::Mat littleMask;
        refineMask(mask(roi), srcRGB(roi), dilate, erode, littleMask);
        littleMask.copyTo(mask(roi));
    }
    
    void ImageProcessing::shiftMask(const cv::Mat &srcMask, cv::Mat &mask, cv::Point offset, bool stretchToBorder) {
        imaging::ImageProcessing::shiftImage(srcMask, mask, offset);
        if (stretchToBorder) {
            _drawStretchRectangles(mask, offset);
        }
    }
    
    void ImageProcessing::refineMask(const cv::Mat &image,
                                     const cv::Mat &maskSrc,
                                     const cv::Mat &noMask,
                                     cv::Mat &maskDest,
                                     int erodeAmount,
                                     int dilateAmount,
                                     bool fast,
                                     int postDilateAmount,
                                     int postErodeAmount) {
        
        int scaleInv = 4;
        
        int dilate = MAX(1, dilateAmount / scaleInv);
        int erode = MAX(1, erodeAmount / scaleInv);
        
        cv::Mat reducedMask;
        cv::resize(maskSrc, reducedMask, cv::Size(image.rows / scaleInv, image.cols / scaleInv), 0, 0, cv::INTER_NEAREST);
        
        refineMask(reducedMask,
                   image,
                   dilate,
                   erode,
                   maskDest);
        
        
        //LOG_IMAGE(maskDest, "watershed_result3.png");
    }
    
    cv::Mat ImageProcessing::refineMask(const cv::Mat &_reducedMask,
                                     const cv::Mat &_rgbImage,
                                     int dilate,
                                     int erode,
                                     cv::Mat &dest,
                                     cv::Size maxSize) {
        
        double scale = 1.0;
        
        //CBLog("refineMask, image size: %dx%d", _reducedMask.cols, _reducedMask.rows);
        
        cv::Mat rgbImage;
        if (_rgbImage.cols > maxSize.width || _rgbImage.rows > maxSize.height) {
            scale = fmin(double(maxSize.width) / double(_rgbImage.cols), double(maxSize.height) / double(_rgbImage.rows));

            cv::resize(_rgbImage, rgbImage, cv::Size(scale * double(_rgbImage.cols), scale * double(_rgbImage.rows)), 0,0, cv::INTER_AREA);
            dilate = max(3, int(double(dilate) * scale));
            erode = max(3, int(double(erode) * scale));
        } else {
            rgbImage = _rgbImage;
        }
        
        cv::Mat reducedMask;
        if (_reducedMask.size() != rgbImage.size()) {
            cv::resize(_reducedMask, reducedMask, rgbImage.size(), 0,0, cv::INTER_AREA);
        } else {
            reducedMask = _reducedMask;
        }
        

        if (dilate == 0 && erode == 0) {
            cv::resize(reducedMask, dest, rgbImage.size(), 0,0, cv::INTER_AREA);
            return reducedMask;
        };

        cv::Mat markers = cv::Mat::zeros(reducedMask.rows, reducedMask.cols, CV_8UC1);
        markers = cv::Scalar(127);
        
        cv::Mat dilatedResult;
        if (dilate > 1) {
            Accelerated::roughDilate(reducedMask, dilatedResult, cv::Size(dilate,dilate));
        } else {
            dilatedResult = reducedMask;
        }
        markers = markers.setTo(cv::Scalar(0), dilatedResult);
        
        cv::Mat erodedResult;
        if (erode > 1) {
            Accelerated::roughErode(reducedMask, erodedResult, cv::Size(erode,erode));
        } else {
            erodedResult = reducedMask;
        }
        
        markers = markers.setTo(cv::Scalar(255), erodedResult);
        
        //Diagnostics::SaveDiagnosticImage(false, markers, "markers");
        imaging::ImageProcessing::watershed(rgbImage, markers, dest);
        
        cv::threshold(dest, dest, 254, 255, cv::THRESH_BINARY);
        
        if (scale != 1.0) {
            cv::resize(dest, dest, _rgbImage.size(), 0, 0, cv::INTER_NEAREST);
        }
        
        return markers;
    }
    
    //http://stackoverflow.com/questions/11294859/how-to-define-the-markers-for-watershed-in-opencv
    void ImageProcessing::watershed(const cv::Mat &_rgbImage, const cv::Mat &markers, cv::Mat &dest) {
        WatershedSegmenter segmenter;
        segmenter.setMarkers(markers);
        
        cv::Mat rgbImage;
        if (_rgbImage.channels() == 1) {
            cv::cvtColor(_rgbImage, rgbImage, CV_GRAY2RGB);
        } else {
            rgbImage = _rgbImage;
        }
        cv::Mat result = segmenter.process(rgbImage);
        result.convertTo(dest,CV_8U);
    }
    
    void ImageProcessing::createOverlayFromMask(const cv::Mat &imgMask, cv::Mat &destImg, cv::Scalar color, int channels)
    {
        //LOG_IMAGE(imgMask, "createOverlayFromMask.png");
        
        if (channels == 4) {
            if (destImg.empty()) {
                destImg.create(imgMask.rows, imgMask.cols, CV_8UC4);
            }
            
            cv::Scalar zeroColor = cv::Scalar::all(0);
            
            for (int y=0; y<imgMask.rows; y++) {
                uchar *maskRowPtr = ROW_PTR(imgMask, y);
                uchar *destRowPtr = ROW_PTR(destImg, y);
                for (int x=0; x<imgMask.cols; x++) {
                    if (getPixel1(x, maskRowPtr)) {
                        setPixel4(x, destRowPtr, color);
                    } else {
                        setPixel4(x, destRowPtr, zeroColor);
                    }
                }
            }
        } else if (channels == 3) {
            if (destImg.empty()) {
                destImg.create(imgMask.rows, imgMask.cols, CV_8UC3);
            }
            
            cv::Scalar zeroColor = cv::Scalar::all(0);
            
            for (int y=0; y<imgMask.rows; y++) {
                uchar *maskRowPtr = ROW_PTR(imgMask, y);
                uchar *destRowPtr = ROW_PTR(destImg, y);
                for (int x=0; x<imgMask.cols; x++) {
                    if (getPixel1(x, maskRowPtr)) {
                        setPixel3(x, destRowPtr, color);
                    } else {
                        setPixel3(x, destRowPtr, zeroColor);
                    }
                }
            }
        }
        
        
    }
    
    /*
     *@brief rotate image by factor of 90 degrees
     *
     *@param source : input image
     *@param dst : output image
     *@param angle : factor of 90, even it is not factor of 90, the angle
     * will be mapped to the range of [-360, 360].
     * {angle = 90n; n = {-4, -3, -2, -1, 0, 1, 2, 3, 4} }
     * if angle bigger than 360 or smaller than -360, the angle will
     * be map to -360 ~ 360.
     * mapping rule is : angle = ((angle / 90) % 4) * 90;
     *
     * ex : 89 will map to 0, 98 to 90, 179 to 90, 270 to 3, 360 to 0.
     *
     */
    void ImageProcessing::rotate_image_90n(const cv::Mat &src, cv::Mat &dst, int angle)
    {
        if (src.empty()) return;
        
        if (angle < 0) angle = 360 + angle;
        angle = ((angle / 90) % 4) * 90;

        int const quarterRotations = std::abs(angle / 90);
        
        if (angle) {
            cv::rotate(src, dst, quarterRotations-1);
        } else if (src.data != dst.data) {
            src.copyTo(dst);
        }
    }
    
    cv::Scalar ImageProcessing::getWhiteBalanceGainAdjustment(const cv::Mat &src, const cv::Mat &mask) {
        
        cv::Scalar gains = cv::Scalar::all(1.0f);
        cv::Scalar means = cv::mean(src, mask);
        
        static double maxImpact = 0.8f;
        static double invMaxImpact = 1.0f / maxImpact;
        
        if (means[1] > 0) {
            gains[0] = means[0] / means[1];
            gains[2] = means[2] / means[1];
            
            gains[0] = 1.0 / LIMIT(gains[0], maxImpact, invMaxImpact);
            gains[1] = 1.0 / LIMIT(gains[1], maxImpact, invMaxImpact);
            gains[2] = 1.0 / LIMIT(gains[2], maxImpact, invMaxImpact);
        }
        
        return gains;
    }
    
    void ImageProcessing::adjustWhiteBalance(const cv::Mat &src, cv::Mat &dest, const cv::Mat &mask, double gainThreshold) {
        
        cv::Scalar gains = getWhiteBalanceGainAdjustment(src, mask);
        
        applyGain(src, dest, gains, gainThreshold);
    }
    
    void ImageProcessing::applyGain(const cv::Mat &src, cv::Mat &dest, cv::Scalar gain, double gainThreshold) {
        
        
        std::vector<cv::Mat> planes;
        cv::split(src, planes);
        for (int i=0; i<planes.size() && i<3; i++) {
            if (abs(gain[i] - 1) > gainThreshold) {
                planes[i] = planes[i] * gain[i];
            }
        }
        cv::merge(planes, dest);
    }
    
    double
    ImageProcessing::getOverlayIntensity(const cv::Mat &bwImage, const cv::Mat &msk) {
        
        cv::Mat mask;
        int dimension = sqrt(cv::countNonZero(msk));
        
        if (dimension < 20) return DEFAULT_OVERLAY_INTENSITY;
        
        int erodeAmount = MIN(20, MAX(4, dimension / 20));
        
        Accelerated::roughErode(msk, mask, cv::Size(erodeAmount, erodeAmount));
        
        cv::Scalar overlayIntensityAvg, overlayIntensityDev;
        cv::meanStdDev(bwImage, overlayIntensityAvg, overlayIntensityDev, mask);
        
        double overlayIntensity = (overlayIntensityAvg[0] - 2.0 * overlayIntensityDev[0]) / 255.0f;
        
        return fmax(0.1, overlayIntensity);
    }
    
    double
    ImageProcessing::getBackgroundIntensity(const cv::Mat &bwImage, const cv::Mat &canny) {
        
        cv::Mat mask;
        int dilateAmount = bwImage.cols/20;
        Accelerated::roughDilate(canny, mask, cv::Size(dilateAmount, dilateAmount));
        
        cv::Mat intensityMask;
        cv::threshold(bwImage, intensityMask, 240, 255, cv::THRESH_BINARY_INV);
        
        mask &= intensityMask;
        
        cv::Scalar overlayIntensityAvg, overlayIntensityDev;
        
        double histIntensity = getBrightness(bwImage, mask);
        
        double bgIntensity;
        cv::Scalar bgIntensityAvg, bgIntensityDev;
        cv::meanStdDev(bwImage, bgIntensityAvg, bgIntensityDev, mask);
        bgIntensity = (MAX_SCALAR(bgIntensityAvg) + MAX_SCALAR(bgIntensityDev)) / 255.0f;
        bgIntensity = sqrt(bgIntensity);
        bgIntensity = LIMIT(bgIntensity, 0.2f, 0.8f);
        
        return bgIntensity * 0.75 + histIntensity * 0.25;
    }
    
    void ImageProcessing::calculateAlphaBetaGamma(double overlayIntensity,
                                                  double backgroundIntensity,
                                                  double *alpha,
                                                  double *beta,
                                                  double *gamma)
    {
        
        overlayIntensity = overlayIntensity * OVERLAY_MULTIPLIER;
        backgroundIntensity = backgroundIntensity * BG_MULTIPLIER;
        
        *alpha = backgroundIntensity / overlayIntensity;
        
        double gainMax = 1.5f;//todo: calculate this limit or better background intensity calc
        //if (overlayIntensity < 0.3) gainMax = 2.0;
        //else if (overlayIntensity < 0.5) gainMax = 1.45;
        
        
        double gainMin = 1.0f / gainMax;
        
        *alpha = LIMIT(*alpha, gainMin, gainMax);
        
        //maybe use the pre-max capped to get beta
        
        //        double difference = paintColorIntensity - overlayIntensity;
        //        *alpha += difference;
        
        //        if (overlayIntensity < 0.4) {
        //            *beta += 0.3f;
        //        }
        //        else if (overlayIntensity < 0.5) {
        //            *beta += 0.2f;
        //        }
        
        //todo: other adjustments for painting on dark colors
        //        if (difference > 0.4) {
        //            *beta = MAX(0.2, *beta);
        //        } else if (difference > 0.3) {
        //            *beta = MAX(0.1, *beta);
        //        }
        
        //*alpha *= paintColorIntensity / backgroundIntensity;
        
        *beta = powf(LIMIT(backgroundIntensity - (*alpha * overlayIntensity), 0, 1), 0.7);
        //*beta = MIN(*beta, 0.3f);
        
        if (gamma) {
            *gamma = LIMIT(powf(backgroundIntensity, 0.25), 0.75, 0.95);
        }
        //printf("ot %.3f, bg %.3f, alpha %.3f, beta %.3f, gamma %.3f \n", overlayIntensity, backgroundIntensity, *alpha, *beta, *gamma);
    }
    
    float ImageProcessing::median(const cv::Mat &hist)
    {
        float half = cv::sum(hist)[0] * 0.5f;
        float tally = 0.f;
        int bin = 0;
        for(int i = 0; i < hist.rows; i++)
        {
            bin = i;
            tally += hist.at<float>(i, 0);
            if(tally >= half)
                break;
        }
        return float(bin);
    }
    
    float ImageProcessing::getBrightness(const cv::Mat &bwImage, const cv::Mat &mask) {
        int rows = 64, bins = 64;
        
        cv::Mat small, smallMask;

        cv::resize(bwImage, small, cv::Size(bwImage.cols*rows/bwImage.rows, rows), 0,0, cv::INTER_NEAREST);
        if (!mask.empty()) {
            cv::resize(mask, smallMask, cv::Size(bwImage.cols*rows/bwImage.rows, rows), 0,0, cv::INTER_NEAREST);
        }

        cv::threshold(small, small, 240, 0, cv::THRESH_TRUNC);
        
        cv::Mat hist;
        
        const float min = 0, max = 255;
        float range[] = { min, max };
        const float * ranges[] = { range };
        
        cv::calcHist(&small, 1, 0, smallMask, hist, 1, &bins, ranges, true, false);
        
        float value = median(hist);
        
        int brightness255 = value / bins;
        
        float brightness = float(brightness255) / 255.0f;
        
        return brightness;
    }
    
    void ImageProcessing::overlayMaskOntoRGB(const cv::Mat &_mask, cv::Mat &destImage, cv::Scalar color) {
        //draw bw onto machine
        
        assert(_mask.channels() == 1);
        
        int channels = destImage.channels();
        double srcAlpha = double(color[3])/255.0f;
        if (!srcAlpha) srcAlpha = 1.0;
        
        uchar color_0 = color[0];
        uchar color_1 = color[1];
        uchar color_2 = color[2];
        
        cv::Mat mask = _mask;
        if (mask.cols != destImage.cols || mask.rows != destImage.rows) {
            cv::resize(_mask, mask, destImage.size());
        }
        
        for(int y=0; y<mask.rows; y++) {
            uchar *machineRowPtr = ROW_PTR(destImage, y);
            uchar *bwRowPtr = ROW_PTR(mask, y);
            for(int x=0, x3=0; x<mask.cols; x++, x3+=channels) {
                uchar bwValue = bwRowPtr[x];
                
                uchar &value_0 = machineRowPtr[x3];
                uchar &value_1 = machineRowPtr[x3+1];
                uchar &value_2 = machineRowPtr[x3+2];
                
                double alpha = srcAlpha * double(bwValue) / 255.0f;
                
                if (alpha > 0.99) {
                    value_0 = color_0;
                    value_1 = color_1;
                    value_2 = color_2;
                }
                else {
                    double invAlpha = 1.0 - alpha;
                    //mix
                    value_0 = uchar(alpha * double(color_0) + invAlpha * double(value_0));
                    value_1 = uchar(alpha * double(color_1) + invAlpha * double(value_1));
                    value_2 = uchar(alpha * double(color_2) + invAlpha * double(value_2));
                }
            }
        }
    }
    
    void ImageProcessing::makeTexturelessImage(const cv::Mat &rgb, const cv::Mat &hsv, const cv::Mat &bwImage, cv::Mat &dest) {
        
        //        cv::Mat bilateral;
        //        cv::bilateralFilter(src, bilateral, 13, 50, 50);
        
        cv::pyrMeanShiftFiltering(rgb, dest, 30, 30, 1);
        cv::cvtColor(dest, dest, CV_RGB2HSV);
        
        cv::Mat edges;
        drawSobel(bwImage, edges);
        cv::threshold(edges, edges, 5, 255, CV_THRESH_TOZERO);
        cv::GaussianBlur(edges, edges, cv::Size(3,3), 3);
        
        for(int y=0; y<rgb.rows; y++) {
            const uchar *hsvPtr = ROW_PTR(hsv, y);
            const uchar *edgesPtr = ROW_PTR(edges, y);
            uchar *destPtr = ROW_PTR(dest, y);
            
            for(int x=0, x3=0; x<rgb.cols; x++, x3+=3) {
                uchar fgAlpha = MIN(255, powf(edgesPtr[x], 1.5));
                uchar fgAlphaInv = 255 - fgAlpha;
                
                destPtr[x3+2] = (fgAlphaInv * destPtr[x3+2] + fgAlpha * hsvPtr[x3+2]) >> 8;
            }
        }
        
        cv::cvtColor(dest, dest, CV_HSV2RGB);
        //dest = dest * 0.6 + bilateral * 0.4;
    }
    
    int ImageProcessing::kmeansColor(const cv::Mat &src, cv::Mat &dest, int k, cv::Mat *_centers, cv::Mat *_labels) {
        
        if (dest.empty()) {
            dest = cv::Mat::zeros(src.rows, src.cols, src.type());
        }
        
        std::vector<cv::Mat> imgRGB;
        cv::split(src,imgRGB);
        
        int n = src.rows *src.cols;
        cv::Mat img3xN(n, src.channels(), CV_8U);
        for(int i=0; i<src.channels(); ++i) {
            imgRGB[i].reshape(1,n).copyTo(img3xN.col(i));
        }
        img3xN.convertTo(img3xN,CV_32F);
        
        int bestK;
        
        cv::Mat labels, centers;
        if (k) {
            bestK = k;
        }
        else {
            //find best K, look for dropoff
            bestK = 3;
            double bestDeviation = 0;
            for (int testK=3; testK<9; testK++) {
                
                cv::kmeans(img3xN,testK, labels,cv::TermCriteria(),3,cv::KMEANS_RANDOM_CENTERS, centers);
                
                double deviation;
                //find best k
                if (src.channels() == 3) {
                    std::vector<cv::Scalar> colors;
                    for (int i=0; i<centers.rows; i++) {
                        float r=centers.at<float>(i, 0);
                        float g=centers.at<float>(i, 1);
                        float b=centers.at<float>(i, 2);
                        
                        colors.push_back(cv::Scalar(r,g,b));
                    }
                    cv::Scalar mean, stddev;
                    cv::meanStdDev(colors, mean, stddev);
                    deviation = (stddev[0] + stddev[1] + stddev[2]) / 3.0f;
                } else {
                    cv::Scalar mean, stddev;
                    cv::meanStdDev(centers, mean, stddev);
                    deviation = stddev[0];
                }
                
                //look for dropoff
                if (deviation < bestDeviation) {
                    bestK = testK;
                    break;
                }
                bestDeviation = deviation;
            }
        }
        
        cv::kmeans(img3xN, bestK, labels, cv::TermCriteria(),5,cv::KMEANS_RANDOM_CENTERS, centers);
        
        labels = labels.reshape(0, src.rows);
        
        for (int y=0; y<dest.rows; y++) {
            
            if (src.channels() == 3) {
                for (int x=0; x<dest.cols; x++) {
                    int label = labels.at<int>(cv::Point(x,y));
                    float r=centers.at<float>(label, 0);
                    float g=centers.at<float>(label, 1);
                    float b=centers.at<float>(label, 2);
                    
                    dest.at<cv::Vec3b>(cv::Point(x,y)) = cv::Vec3b(r,g,b);
                }
            } else {
                for (int x=0; x<dest.cols; x++) {
                    int label = labels.at<int>(cv::Point(x,y));
                    float color = centers.at<float>(label, 0);
                    dest.at<uchar>(cv::Point(x,y)) = color;
                }
            }
        }
        
        if (_centers) {
            *_centers = centers;
        }
        
        if (_labels) {
            *_labels = labels;
        }
        
        return bestK;
    }
    
    void ImageProcessing::maskInRange(const cv::Mat &src, cv::Mat &dest, int delta, const cv::Mat& mask) {
        cv::Scalar mean, stddev;
        cv::meanStdDev(src, mean, stddev, mask);
        if (!delta) {
            delta = 15 + 2 * MAX(stddev[0], MAX(stddev[1], stddev[2]));
        }
        cv::Scalar lowerBound = cv::Scalar(mean[0] - delta, mean[1] - delta, mean[2] - delta);
        cv::Scalar upperBound = cv::Scalar(mean[0] + delta, mean[1] + delta, mean[2] + delta);
        cv::Mat inRange;
        cv::inRange(src, lowerBound, upperBound, dest);
    }
    
    void ImageProcessing::sticktoitiveness(cv::Mat &mask, const cv::Mat &toStickTo, int dilateAmount, cv::Size maxSize) {
        
        if (!dilateAmount) {
            dilateAmount = fmax(mask.cols, mask.rows) / 60;
        }
        
        cv::Mat _mask, _toStickTo;
        double scale = 1.0;
        if (mask.cols > maxSize.width || mask.rows > maxSize.height) {
            scale = fmin(double(maxSize.width) / double(mask.cols), double(maxSize.height) / double(mask.rows));
            
            cv::resize(mask, _mask, cv::Size(scale * double(mask.cols), scale * double(mask.rows)), 0, 0, cv::INTER_CUBIC);
            cv::resize(mask, _toStickTo, _mask.size(), 0, 0, cv::INTER_CUBIC);
            dilateAmount = max(3, int(double(dilateAmount) * scale));
        } else {
            _mask = mask;
            _toStickTo = mask;
        }
        
        cv::Mat bledNewMask;
        cv::threshold(_mask, bledNewMask, 254, 255, cv::THRESH_BINARY);
        Accelerated::dilate(bledNewMask, bledNewMask, dilateAmount);
        
        cv::Mat bledExistingMask;
        cv::threshold(_toStickTo, bledExistingMask, 254, 255, cv::THRESH_BINARY);
        Accelerated::dilate(bledExistingMask, bledExistingMask, dilateAmount);
        
        cv::bitwise_and(bledNewMask, bledExistingMask, bledNewMask);
        
        int totalPixels = cv::countNonZero(bledNewMask);
        
        if (totalPixels > 0) {
            //extends out on the ends, which is most often between walls left/right extending upward and downward, so this erode is temporary fix.
            //ideally you would want to just remove the extra cap, which will take more thought
            cv::erode(bledNewMask, bledNewMask, cv::getStructuringElement(CV_SHAPE_ELLIPSE,cv::Size(1, dilateAmount)),
                      cv::Point(-1,-1), 1, cv::BORDER_REPLICATE);
            
            int padding = dilateAmount;
            cv::Rect roi = cv::Rect(padding, padding, _mask.cols - 2 * padding, _mask.rows - 2 * padding);
            
            cv::dilate(bledNewMask(roi) - _toStickTo(roi), bledNewMask(roi), cv::getStructuringElement(CV_SHAPE_ELLIPSE,cv::Size(dilateAmount/2, dilateAmount/2)),
                      cv::Point(-1,-1), 1, cv::BORDER_REPLICATE);
            
            _mask(roi) += bledNewMask(roi);
            
            if (mask.data != _mask.data) {
                cv::resize(_mask, mask, cv::Size(mask.cols, mask.rows));
            }
            //LOG_IMAGE(mask, "sticktoitiveness.png");
        }
    }
    
    
#define ThickFill_R 0
#define ThickFill_L 1
    
    //https://www.youtube.com/watch?v=6SSh-tej4lo
    struct ThickFillNode {
        cv::Point point;
        cv::Point lastPoint;
        
        int direction;
        cv::Scalar mean;
        cv::Scalar stddev;
        bool dead = false;
    };
    
    void ImageProcessing::thickFloodFill(const cv::Mat &srcImage, cv::Mat &mask, cv::Point fillPoint) {
        
        cv::Scalar mean, stddev;
        Imaging::meanStdDevAtPoint(srcImage, mean, stddev, fillPoint);
        
        std::vector<ThickFillNode>seeds;
        
        ThickFillNode nodeR;
        nodeR.point = fillPoint;
        nodeR.lastPoint = nodeR.point;
        nodeR.direction = ThickFill_R;
        nodeR.mean = mean;
        nodeR.stddev = stddev;
        
        ThickFillNode nodeL = nodeR;
        nodeL.direction = ThickFill_L;
        
        seeds.push_back(nodeR);
        seeds.push_back(nodeL);
        
        int radius = 3;
        int diameter = 2 * radius;
        
        //float maxMeanDistance = 30;
        //float maxMeanDistanceSq = maxMeanDistance * maxMeanDistance;
        
        float maxStdDevDistance = 15;
        float maxStdDevDistanceSq = maxStdDevDistance * maxStdDevDistance;
        
        //hande initial point
        cv::Rect roi = Geometry::rectAtPoint(fillPoint, radius, mask);
        cv::rectangle(mask, roi, cv::Scalar(255), CV_FILLED);
        
        while (seeds.size()) {
            std::vector<ThickFillNode>newSeeds;
            
            bool valid = true;
            ThickFillNode currentNode = seeds[0];
            
            if (currentNode.direction == ThickFill_R) {
                currentNode.point.x += diameter;
                valid = currentNode.point.x <= srcImage.cols - radius;
            } else {
                currentNode.point.x -= diameter;
                valid = currentNode.point.x >= radius;
            }
            
            if (valid) {
                valid = !mask.at<uchar>(currentNode.point);
            }
            
            if (valid) {
                cv::Rect roi;
                Imaging::meanStdDevAtPoint(srcImage, mean, stddev, currentNode.point, radius, 0, &roi);
                
                if (Coloring::euclideanDistanceSq(currentNode.stddev, stddev) < maxStdDevDistance) {
                    //ok
                    currentNode.mean = mean;
                    currentNode.stddev = stddev;
                    currentNode.lastPoint = currentNode.point;
                    
                    //paint
                    cv::rectangle(mask, roi, cv::Scalar(255), CV_FILLED);
                    newSeeds.push_back(currentNode);
                    
                } else {
                    valid = false;
                }
            }
            
            if (valid) {
                
                //invalid
                std::vector<cv::Point2f>twoPoints;
                twoPoints.push_back(cv::Point(currentNode.lastPoint.x, currentNode.lastPoint.y + diameter));
                twoPoints.push_back(cv::Point(currentNode.lastPoint.x, currentNode.lastPoint.y - diameter));
                
                for (int i=0; i<2; i++) {
                    //check above
                    cv::Point newPoint = twoPoints[i];
                    if (newPoint.y > radius && newPoint.y < srcImage.rows - radius && !mask.at<uchar>(newPoint)) {
                        cv::Rect roi;
                        Imaging::meanStdDevAtPoint(srcImage, mean, stddev, newPoint, radius, 0, &roi);
                        if (Coloring::euclideanDistanceSq(currentNode.stddev, stddev) < maxStdDevDistanceSq) {
                            //make two nodes above, L,R
                            ThickFillNode nodeR;
                            nodeR.point = newPoint;
                            nodeR.lastPoint = nodeR.point;
                            nodeR.direction = ThickFill_R;
                            nodeR.mean = mean;
                            nodeR.stddev = stddev;
                            
                            ThickFillNode nodeL = nodeR;
                            nodeL.direction = ThickFill_L;
                            
                            //insert them
                            newSeeds.push_back(nodeR);
                            newSeeds.push_back(nodeL);
                            
                            cv::rectangle(mask, roi, cv::Scalar(255), CV_FILLED);
                        }
                    }
                }
            }
            
            
            //add the rest
            for (int i=1; i<seeds.size(); i++) {
                newSeeds.push_back(seeds[i]);
            }
            seeds = newSeeds;
            
        }
    }
    
#ifdef __ACCELERATE__
    cv::Mat
    ImageProcessing::conv2(const cv::Mat& src, const cv::Mat& kernel)
    {
        //EVLog("Convolution Started (conv2)");
        if (src.type()!=CV_32FC1 || kernel.type()!=CV_32FC1) {
            return cv::Mat();
        }
        int sn = src.rows * src.cols;
        float* mat = new float[sn];
        float* sp = mat;
        for (int i = 0; i < src.rows; ++i) {
            const float* p = src.ptr<float>(i);
            for (int j = 0; j < src.cols; ++j) {
                *sp++ = *p++;
            }
        }
        
        int kn = kernel.rows * kernel.cols;
        float* kern = new float[kn];
        float* kp = kern;
        for (int i = 0; i < kernel.rows; ++i) {
            const float* p = kernel.ptr<float>(kernel.rows - i - 1);
            for (int j = 0; j < kernel.cols; ++j) {
                *kp++ = p[kernel.cols - j - 1];
            }
        }
        
        cv::Mat res(src.rows, src.cols, CV_32FC1);
        
        float* resp = res.ptr<float>(0);
        
        vImage_Buffer src1;
        src1.data = (void*)mat;
        src1.height = src.rows;
        src1.width = src.cols;
        src1.rowBytes = src.cols * sizeof(float);
        vImage_Buffer dest;
        dest.data = resp;
        dest.height = src.rows;
        dest.width = src.cols;
        dest.rowBytes = src.cols*sizeof(float);
        
        // ch un-commented the following call on 10/21/2013
        vImageConvolve_PlanarF(&src1, &dest, 0, 0, 0, kern,
                               kernel.rows, kernel.cols, 0, kvImageBackgroundColorFill);
        
        delete[] mat;
        delete[] kern;
        
        //EVLog("Convolution Completed (conv2)");
        return res;
    }
#else
    
    cv::Mat
    ImageProcessing::conv2(const cv::Mat& src, const cv::Mat& kernel)
    {
        //EVLog("Convolution Started (conv2)");
        if (src.type()!=CV_32FC1 || kernel.type()!=CV_32FC1) {
            return cv::Mat();
        }
        cv::Mat kern(kernel.rows, kernel.cols, CV_32FC1);
        int i, j;
        float* tp=0;
        for (i = 0; i < kernel.rows; ++i) {
            const float* fp = kernel.ptr<float>(i);
            tp = kern.ptr<float>(kernel.rows - i - 1);
            for (j = 0; j < kernel.cols; ++j) {
                tp[kernel.cols - j - 1] = fp[j];
            }
        }
        cv::Mat res;
        cv::filter2D(src, res, src.depth(), kern, cv::Point(-1,-1),
                     0, cv::BORDER_CONSTANT);
        //EVLog("Convolution Completed (conv2)");
        return res;
    }
    
#endif
    
    cv::Mat _getGaborWallKernel(int kernelSize, double angle, double sig) {
        
        double th = toRadians(angle), lm = sig * 2.0, gm = 1.4, ps = toRadians(0.0);
        cv::Mat kernel = cv::getGaborKernel(cv::Size(kernelSize, kernelSize), sig, th, lm, gm, ps, CV_32FC1);
        
        return kernel;
    }
    
    cv::Mat _buildWallKernel(double angles[], int numAngles, int size, double sig) {
        
        cv::Mat kernel;
        for (int i=0; i<numAngles; i++) {
            cv::Mat partialKernel = _getGaborWallKernel(size, angles[i], sig);
            if (i == 0) {
                kernel = partialKernel;
            } else {
                kernel += partialKernel;
            }
        }
        
        return kernel;
    }
    
    void ImageProcessing::gaborEdges(const cv::Mat& bwImage, cv::Mat& result, int size, double sigma, float multiplier)
    {
        double angles[] = {0, 30, -30, 90};
        int numAngles = 4;
        
        cv::Mat kernel = _buildWallKernel(angles, numAngles, size, sigma);
        
        //static cv::Mat fastKernel = _buildWallKernel(angles, numAngles, 9, 1.4); intensity = 4
        //static cv::Mat preciseKernel = _buildWallKernel(angles, numAngles, 11, 2.5); intensity = 1
        
        
        bwImage.convertTo(result, CV_32FC1, 1.0/255);
        
        result = conv2(result, multiplier * kernel);
        
        result.convertTo(result, CV_8UC1, 255.0);
        
        //        cv::Mat blurred;
        //        if (small) {
        //            cv::bilateralFilter(result, blurred, 7, 80, 80);
        //        } else {
        //            cv::bilateralFilter(result, blurred, 9, 30, 50);
        //        }
        //
        //        result = blurred;
    }
    
    void ImageProcessing::clahe(const cv::Mat& bwImage, cv::Mat& result, int tileSize, double clipLimit)
    {
        cv::Ptr<cv::CLAHE> pFilter;
#if OPENCL
        pFilter = ocl::createCLAHE();
#else
        pFilter = cv::createCLAHE();
#endif
        pFilter->setTilesGridSize(cv::Size(tileSize, tileSize));
        pFilter->setClipLimit(clipLimit);
        
        pFilter->apply(bwImage, result);
    }
    
    void ImageProcessing::matchImageExposures(const cv::Mat &imageA, const cv::Mat &maskA,
                                              const cv::Mat &imageB, const cv::Mat &maskB,
                                              cv::Mat &dest, cv::Size blockSize) {
        
        std::vector<cv::Point> corners(2);
        std::vector<cv::UMat> images_warped(2);
        std::vector<cv::UMat> mask_warp(2);
        
        for (int i = 0; i < 2; ++i)
            corners[i] = cv::Point(0,0);
        
        images_warped[0] = imageA.getUMat(cv::ACCESS_READ);
        images_warped[1] = imageB.getUMat(cv::ACCESS_RW);
        
        mask_warp[0] = maskA.getUMat(cv::ACCESS_READ);
        mask_warp[1] = maskB.getUMat(cv::ACCESS_READ);
        
        cv::Ptr<cv::detail::ExposureCompensator> compensator;
        if (blockSize.width) {
            compensator = new cv::detail::BlocksGainCompensator(blockSize.width, blockSize.height);
        } else {
            compensator = cv::detail::ExposureCompensator::createDefault(cv::detail::ExposureCompensator::GAIN);
        }
        compensator->feed(corners, images_warped, mask_warp);
        compensator->apply(1, corners[1], images_warped[1], mask_warp[1]);
        dest = images_warped[1].getMat(cv::ACCESS_READ).clone();
    }
    
    void ImageProcessing::blendImages(const cv::Mat &imageA, const cv::Mat &maskA,
                                      const cv::Mat &imageB, const cv::Mat &maskB,
                                      cv::Mat &mergedImage, cv::Mat &mergedMask, int type) {
        
        cv::Mat imageAS, imageBS;
        imageA.convertTo(imageAS, CV_16S);
        imageB.convertTo(imageBS, CV_16S);
        
        auto blender = cv::detail::Blender::createDefault(type);
        blender->prepare(cv::Rect(0, 0, max(imageAS.cols, imageBS.cols), max(imageAS.rows, imageBS.rows)));
        blender->feed(imageAS, maskA, cv::Point(0,0));
        blender->feed(imageBS, maskB, cv::Point(0,0));
        blender->blend(mergedImage, mergedMask);
        
        mergedImage.convertTo(mergedImage, CV_8UC3);
    }
    
    int ImageProcessing::getLargestContour(const std::vector<std::vector<cv::Point> > &contours) {
        if (contours.size()) {
            
            int largest_area=0;
            int largest_contour_index=0;
            cv::Rect bounding_rect;
            for( int i = 0; i< contours.size(); i++ ) {
                double a=cv::contourArea( contours[i],false);  //  Find the area of contour
                if (a>largest_area) {
                    largest_area=a;
                    largest_contour_index=i;                //Store the index of largest contour
                    bounding_rect=cv::boundingRect(contours[i]); // Find the bounding rectangle for biggest contour
                }
            }
            return largest_contour_index;
        }
        
        return -1;
    }
    
    void ImageProcessing::expandImageToROI(const cv::Mat &src, cv::Mat &dest, const cv::Rect &roi) {
        cv::Mat _dest = cv::Mat::zeros(roi.height, roi.width, src.type());
        src.copyTo(_dest(cv::Rect(-roi.x, -roi.y, src.cols, src.rows)));
        dest = _dest;
    }
}

