//
//  Drawing.h
//  Cambrian
//
//  Created by Joel Teply on 6/15/12.
//
//

#include <opencv2/opencv.hpp>
#include <cambrian.h>

namespace imaging {

//FASTEST DRAWING METHODS ARE THESE DEFINES, faster than class:
// 
//
//for (int y=0; y<cvImage.rows; y++) {
//    uchar *rowPtr = ROW_PTR(cvImage, y);
//    for (int x=0; x<cvImage.cols; x++) {
//        CvScalar color = getPixel4(x, rowPtr);
//        
//        if (color.val[3] > 0) {
//            setPixel4(x, rowPtr, destColor);
//        }
//    }
//}

    
#define COLOR_BLACK cv::Scalar(0,0,0,255)
#define COLOR_WHITE cv::Scalar::all(255)
#define COLOR_DARKGREY cv::Scalar(70,70,70,255)
#define COLOR_GREY cv::Scalar(120,120,120,255)
    
    
#define COLOR_RED cv::Scalar(255,0,0,255)
#define COLOR_GREEN cv::Scalar(0,255,0,255)
#define COLOR_BLUE cv::Scalar(0,0,255,255)
    
#define COLOR_YELLOW cv::Scalar(250,250,0,255)
#define COLOR_ORANGE cv::Scalar(255,150,0,255)
#define COLOR_PINK cv::Scalar(250,0,234,255)
#define COLOR_AQUA cv::Scalar(0,240,255,255)

#define ROW_PTR(img, y) ((uchar*)((img).data + (img).step * y))

//use ROW_PTR from above in these functions
#define getPixel1(x, rowPtr) rowPtr[x]
#define getPixel3(x, rowPtr) cv::Vec3b(rowPtr[x*3], rowPtr[x*3+1], rowPtr[x*3+2])
#define getPixel4(x, rowPtr) cv::Vec4b(rowPtr[x*4], rowPtr[x*4+1], rowPtr[x*4+2], rowPtr[x*4+3])

#define setPixel1(x, rowPtr, color) { rowPtr[x] = color }

#define setPixel3(x, rowPtr, color) \
{ \
int x3 = x*3;\
rowPtr[x3] = color.val[0];\
rowPtr[x3+1] = color.val[1];\
rowPtr[x3+2] = color.val[2];\
}

#define setPixel4(x, rowPtr, color) \
{ \
int x4 = x*4;\
rowPtr[x4] = color.val[0];\
rowPtr[x4+1] = color.val[1];\
rowPtr[x4+2] = color.val[2];\
rowPtr[x4+3] = color.val[3];\
}

#ifndef __Drawing__
#define __Drawing__
    
    struct DLL_LOCAL DrawPoint {
        cv::Point point;
        int radius;
    };

    class DLL_LOCAL Drawing
    {
    public:
        
        static uchar getSingleChannelPixelAt(const cv::Point &point, const cv::Mat &img);
        static cv::Vec3b getTriChannelPixelAt(const cv::Point &point, const cv::Mat &img);
        static cv::Vec4b getQuadChannelPixelAt(const cv::Point &point, const cv::Mat &img);
        static cv::Scalar getPixelAt(const cv::Point &point, const cv::Mat &img);
        //
        static void setSingleChannelPixelAt(const cv::Point &point, cv::Mat &img, uchar color);
        static void setTriChannelPixelAt(const cv::Point &point, cv::Mat &img, const cv::Scalar &color);
        static void setQuadChannelPixelAt(const cv::Point &point, cv::Mat &img, const cv::Scalar &color);
        static void setPixelAt(const cv::Point &point, cv::Mat &img, const cv::Scalar &color);
        
        static void setOutputArray(cv::OutputArray outputArray, const cv::Scalar &scalar, int channels=4);
        
        static std::vector<cv::Point2f> createBezierCurve(const std::vector<cv::Point2f> &inPoints, float minDistance);
        static std::vector<cv::Point2f> createBezierCurve(const std::vector<cv::Point2f> &inPoints, int totalPoints);
        
        static void drawCurve(cv::Mat &img, const std::vector<DrawPoint> &points, const cv::Scalar &color, bool rounded =1);
    };

#endif
    
};

