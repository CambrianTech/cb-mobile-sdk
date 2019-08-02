//
//  Imaging.cpp
//
//  Created by Joel Teply on 6/14/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "Imaging.h"
#include "Geometry.h"
#include "Drawing.h"

#include <stdio.h>
#include <zlib.h>
#include <fstream>

#include <utility/Directory.h>
#include <utility/Diagnostics.h>

namespace imaging {
    void Imaging::meanStdDevForPolygon(const cv::Mat &image, std::vector<cv::Point2f> points, cv::OutputArray mean, cv::OutputArray stddev)
    {
        cv::Mat mask = cv::Mat(image.size(), CV_8UC1, cv::Scalar(0));
        
        cv::fillConvexPoly(mask, points, cv::Scalar(1));
        cv::meanStdDev(image, mean, stddev, mask);
    }
    
    cv::Mat Imaging::rotateImage(const cv::Mat &source, double radians)
    {
        cv::Point2f src_center(source.cols / 2.0F, source.rows / 2.0F);
        
        return rotateImage(source, radians, src_center);
    }
    
    cv::Mat Imaging::rotateImage(const cv::Mat &source, double radians, cv::Point2f aroundPoint)
    {
        cv::Mat rot_mat = cv::getRotationMatrix2D(aroundPoint, radians, 1.0);
        cv::Mat dst;
        
        if (source.cols > 1 && source.rows > 1)
        {
            warpAffine(source, dst, rot_mat, source.size());
            return dst;
        }
        return dst;
    }
    
    
    //regardless of sizes
    cv::Scalar Imaging::imageDifferencePerPixel(const cv::Mat &imageA, const cv::Mat &imageB, float angleBetween)
    {
        cv::Mat compareA, compareB;
        
        //scale both to the same size
        if (imageA.cols == imageB.cols)
        {
            compareA = imageA;
            compareB = imageB;
        }
        else if (imageA.cols < imageB.cols)
        {
            compareA = imageA;
            cv::resize(imageB, compareB, imageA.size());
        }
        else
        {
            cv::resize(imageA, compareA, imageB.size());
            compareB = imageB;
        }
        
        cv::Mat diffImage;
        cv::absdiff(compareA, compareB, diffImage);
        cv::Scalar sum = cv::sum(diffImage);
        int totalPixels = compareA.cols * compareA.rows;
        return cv::Scalar(sum.val[0] / (float)totalPixels,
                          sum.val[1] / (float)totalPixels,
                          sum.val[2] / (float)totalPixels);
    }
    
    cv::Scalar Imaging::meanAtPoint(const cv::Mat &img, cv::Point point, float radius)
    {
        cv::Rect roi = Geometry::rectAtPoint(point, radius, img);
        
        return cv::mean(img(roi));
    }
    
    cv::Rect Imaging::meanStdDevAtPoint(const cv::Mat &img, cv::Scalar &mean, cv::Scalar &stddev, cv::Point point,
                                        float outerRadius, float innerRadius, cv::Rect *destRoi)
    {
        cv::Rect roi = Geometry::rectAtPoint(point, outerRadius, img);
        if (destRoi) *destRoi = roi;
        
        cv::Mat mask;
        if (innerRadius) {
            mask = cv::Mat(roi.height, roi.width, CV_8UC1);
            mask.setTo(255);
            cv::Rect innerROI = Geometry::rectAtPoint(cv::Point(roi.width/2, roi.height/2), innerRadius, img(roi));
            mask(innerROI).setTo(0);
        }
        
        cv::meanStdDev(img(roi), mean, stddev, mask);
        
        return roi;
    }
    
    void Imaging::meanStdDevForPoints(const cv::Mat &srcImage, const std::vector<cv::Point>&points,
                                              cv::Scalar &sampleMean, cv::Scalar &sampleStdDev) {
        std::vector<cv::Vec3b>samples;
        
        for (int i=0; i<points.size(); i++) {
            cv::Point point = points[i];
            
            if (point.x >= 0 && point.y >= 0 && point.x < srcImage.cols && point.y < srcImage.rows) {
                cv::Vec3b sample = srcImage.at<cv::Vec3b>(point.y, point.x);
                samples.push_back(sample);
            }
        }
        
        cv::meanStdDev(samples, sampleMean, sampleStdDev);
    }
    
    cv::Mat Imaging::cropImage(const cv::Mat &img, cv::Rect toRect)
    {
        cv::Rect adjustedRect = toRect;
        
        if (adjustedRect.x < 0)
            adjustedRect.x = 0;
        if (adjustedRect.y < 0)
            adjustedRect.y = 0;
        if (adjustedRect.x + adjustedRect.width > img.cols)
            adjustedRect.width = img.cols - adjustedRect.x;
        if (adjustedRect.y + adjustedRect.height > img.rows)
            adjustedRect.height = img.rows - adjustedRect.y;
        return cv::Mat(img, adjustedRect);
    }
    
    cv::Mat Imaging::cropImage(const cv::Mat &img, double desiredAspectRatio, bool isRotated90)
    {
        int width = isRotated90 ? img.rows : img.cols;
        int height = isRotated90 ? img.cols : img.rows;
        
        double currentAspectRatio = ((double)width) / ((double)height);
        
        if (currentAspectRatio < desiredAspectRatio)
            height = width / desiredAspectRatio;
        else if (currentAspectRatio > desiredAspectRatio)
            width = height / desiredAspectRatio;
        
        cv::Rect aspectClip = isRotated90 ?
        cv::Rect((img.cols - height) / 2, (img.rows - width) / 2, height, width)
        : cv::Rect((img.cols - width) / 2, (img.rows - height) / 2, width, height);
        
        return cropImage(img, aspectClip);
    }
    
    cv::Mat Imaging::scaleImage(const cv::Mat &img, double scale, int type)
    {
        cv::Mat result;
        
        cv::resize(img, result, cv::Size(scale * img.cols, scale * img.rows), 0, 0, type);
        return result;
    }
    
    void Imaging::restrictImageToSize(const cv::Mat &img, cv::Mat &dest, cv::Size size, int scaleType)
    {
        double srcWidth = img.cols;
        double srcHeight = img.rows;
        
        double resizeWidth = srcWidth;
        double resizeHeight = srcHeight;
        
        double aspectRatio = resizeWidth / resizeHeight;
        
        if (resizeWidth > size.width)
        {
            resizeWidth = size.width;
            resizeHeight = resizeWidth / aspectRatio;
        }
        
        if (resizeHeight > size.height)
        {
            resizeHeight = size.height;
            resizeWidth = resizeHeight * aspectRatio;
        }
        
        if (resizeWidth < srcWidth)
        {
            cv::resize(img, dest, cv::Size(resizeWidth, resizeHeight), scaleType);
        } else {
            dest = img;
        }
    }
    
    void Imaging::getCvMat(cv::Mat& rawMat, uint8_t* rawData, cv::Size size, int numChannels)
    {
        int cols = size.width;
        int rows = size.height;
        const uint8_t* fp = rawData;
        
        switch (numChannels) {
            case 1: {
                rawMat.create(size.height, size.width, CV_8UC1);
                uint8_t* p = rawMat.data;
                for (int i = 0; i < rows; i++) {
                    for (int j = 0; j < cols; j++) {
                        *p++ = *fp++;
                        fp+=3;
                    }
                }
                break;
            }
            case 2: {
                rawMat.create(size.height, size.width, CV_8UC2);
                uint8_t* p = rawMat.data;
                for (int i = 0; i < rows; i++) {
                    for (int j = 0; j < cols; j++) {
                        *p++ = *fp++;
                        *p++ = *fp++;
                        fp+=2;
                    }
                }
                break;
            }
            case 3: {
                rawMat.create(size.height, size.width, CV_8UC3);
                uint8_t* p = rawMat.data;
                for (int i = 0; i < rows; i++) {
                    for (int j = 0; j < cols; j++) {
                        *p++ = *fp++;
                        *p++ = *fp++;
                        *p++ = *fp++;
                        fp+=1;
                    }
                }
                break;
            }
            case 4: {
                rawMat.create(size.height, size.width, CV_8UC4);
                
                uint8_t *ptr = rawData;
                int width_bytes = size.width * 4;
                
                for (int i = 0; i < size.height; i++) {
                    memcpy(rawMat.ptr(i), ptr, width_bytes);
                    ptr += width_bytes;
                }
                break;
            }
        }
    }
    
    uchar*
    Imaging::getMatData(const cv::Mat& mat)
    {
        //GPU coordinates are flipped vertically.
        //The most common approach is to flip at load, so I did it here - Joel
        int nchan = mat.channels();
        if (nchan < 3) {
            nchan = 3;
        }
        
        uchar* data = new uchar[mat.rows * mat.cols * nchan];
        uchar* p = data;
        if (mat.type() == CV_8UC1) {
            for (int i = 0; i < mat.rows; i++) {
                const uint8_t* fp = mat.ptr(i);
                for (int j = 0; j < mat.cols; j++) {
                    *p++ = *fp;
                    *p++ = *fp;
                    *p++ = *fp++;
                }
            }
        }
        else if (mat.type() == CV_8UC2) {
            for (int i = 0; i < mat.rows; i++) {
                const uint8_t* fp = mat.ptr(i);
                for (int j = 0; j < mat.cols; j++) {
                    *p++ = *fp++;
                    *p++ = *fp;
                    *p++ = *fp++;
                }
            }
        }
        else if (mat.type() == CV_8UC3 || mat.type()==CV_8UC4) {
            int nbytes = mat.cols * nchan;
            for (int i = 0; i < mat.rows; i++) {
                const uint8_t* fp = mat.ptr(i);
                memcpy(p, fp, nbytes);
                p += mat.cols*nchan;
            }
        }
        else {
            return 0;
        }
        
        return data;
    }
    
    bool Imaging::importMaskedPNG(cv::Mat &largeImage, cv::Mat &alphaMask) {
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
                    
                    //correct premultiplication
                    if (alphaValue > 0) {
                        //premultiplied, so turn back
                        imageRowPtr[x4] = (imageRowPtr[x4] * 255 / alphaValue);
                        imageRowPtr[x4+1] = (imageRowPtr[x4+1] * 255 / alphaValue);
                        imageRowPtr[x4+2] = (imageRowPtr[x4+2] * 255 / alphaValue);
                        imageRowPtr[x4+3] = 255;//set it to 255
                    }
                }
            }
        }
        
        hasAlpha = alphaCount > 2 * (largeImage.cols + largeImage.rows);
        
        return hasAlpha;
    }
    
    cv::Mat Imaging::blend_multiply(const cv::Mat& level1, const cv::Mat& level2, float fopacity)
    {
        CV_Assert(level1.size() == level2.size());
        CV_Assert(level1.type() == level2.type());
        CV_Assert(level1.channels() == level2.channels());
        
        // Get 4 channel float images
        cv::Mat4f src1, src2;
        
        if (level1.channels() == 3)
        {
            cv::Mat4b tmp1, tmp2;
            cvtColor(level1, tmp1, cv::COLOR_BGR2BGRA);
            cvtColor(level2, tmp2, cv::COLOR_BGR2BGRA);
            tmp1.convertTo(src1, CV_32F, 1. / 255.);
            tmp2.convertTo(src2, CV_32F, 1. / 255.);
        }
        else
        {
            level1.convertTo(src1, CV_32F, 1. / 255.);
            level2.convertTo(src2, CV_32F, 1. / 255.);
        }
        
        cv::Mat4f dst(src1.rows, src1.cols, cv::Vec4f(0., 0., 0., 0.));
        
        // Loop on every pixel
        
        float comp_alpha, new_alpha;
        
        for (int r = 0; r < src1.rows; ++r)
        {
            for (int c = 0; c < src2.cols; ++c)
            {
                const cv::Vec4f& v1 = src1(r, c);
                const cv::Vec4f& v2 = src2(r, c);
                cv::Vec4f& out = dst(r, c);
                
                comp_alpha = fmin(v1[3], v2[3]) * fopacity;
                new_alpha = v1[3] + (1.f - v1[3]) * comp_alpha;
                
                if ((comp_alpha > 0.) && (new_alpha > 0.))
                {
                    float ratio = comp_alpha / new_alpha;
                    
                    out[0] = fmax(0.f, fmin(v1[0] * v2[0], 1.f)) * ratio + (v1[0] * (1.f - ratio));
                    out[1] = fmax(0.f, fmin(v1[1] * v2[1], 1.f)) * ratio + (v1[1] * (1.f - ratio));
                    out[2] = fmax(0.f, fmin(v1[2] * v2[2], 1.f)) * ratio + (v1[2] * (1.f - ratio));
                }
                else
                {
                    out[0] = v1[0];
                    out[1] = v1[1];
                    out[2] = v1[2];
                }
                
                out[3] = v1[3];
                
            }
        }
        
        cv::Mat3b dst3b;
        cv::Mat4b dst4b;
        dst.convertTo(dst4b, CV_8U, 255.);
        cvtColor(dst4b, dst3b, cv::COLOR_BGRA2BGR);
        
        return dst3b;
    }
}
