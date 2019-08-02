//
//  ImageProcessing.h
//  Cambrian
//
//  Created by Joel Teply on 6/15/12.
//
//

#ifndef __ImageProcessing__
#define __ImageProcessing__

#include <opencv2/opencv.hpp>
#include <cambrian.h>
#include <cbcommon/CB_Types.h>
#include <utility/Diagnostics.h>
#include "LineProcessing.h"

typedef struct _CBSample
{
    cv::Point origin;
    int radius;
    cv::Size size;
    
    cv::Scalar  rgbMean;
    cv::Scalar  rgbDeviation;
    
    cv::Scalar  hsvMean;
    cv::Scalar  hsvDeviation;
}
CBSample;

namespace imaging {
    
    
    class DLL_LOCAL ImageProcessing
    {
    public:
        static void getContourAndArea(const cv::Mat &mask, std::vector<cv::Point> &contour, double &contourArea);
        
        static cv::Mat getEdgesFromBWImage(cv::Mat imgBW, CBSample sampleInfo);
        
        static void drawSobel(const cv::Mat &imgBW, cv::Mat &sobel);
        static void freiChen(const cv::Mat &imgBW, cv::Mat &dest, double intensity=1.0);
        
        static void retinex(const cv::Mat &img, cv::Mat &imgOut, double amount=0.4f);
        
        static vector<vector<cv::Point> > findContours(cv::Mat greyscale,
                                                                 int threshold=0, int maxThreshold=0);
        static vector<vector<cv::Point> > findPolygons(cv::Mat greyscale, double epsilon=3,
                                                                 int threshold=0, int maxThreshold=0);
        
        static std::vector<std::vector<cv::Point> > getPolygonsForContours(std::vector<std::vector<cv::Point> > contours, double epsilon);
        
        static void drawPolygons(const cv::Mat &srcBW, cv::Mat &destImage, cv::Scalar color=cv::Scalar::all(-1), int thickness=2,
                                 double epsion=3, int threshold=0, int maxThreshold=0);
        
        static void drawContours(const cv::Mat &srcBW, cv::Mat &destImage, cv::Scalar color=cv::Scalar::all(-1), int thickness=2,
                                 int threshold=0, int maxThreshold=0);
        static void drawContours(const std::vector<std::vector<cv::Point> > &contours, cv::Mat &destImage, cv::Scalar color=cv::Scalar::all(-1), int thickness=2);
        static int numContourPoints(const std::vector<std::vector<cv::Point> > &contours);
        static double contourArea(const std::vector<std::vector<cv::Point> > &contours);
        
        static int createMaskFromPaintPoint(const cv::Mat &image,
                                            cv::Mat &mask,
                                            cv::Point2f seedPoint,
                                            int *utilizedThreshold=0,
                                            int minThreshold=2,
                                            int maxThreshold=10,
                                            bool fast=false,
                                            int maxIterations=4,
                                            std::vector<LineSegment>* linesFound=0);
        
        static int createMaskFromMultiplePaintPoints(const cv::Mat &image,
                                                     cv::Mat &mask,
                                                     const std::vector<cv::Point2f>&seedPoints,
                                                     int *utilizedThreshold,
                                                     int minThreshold=2,
                                                     int maxThreshold=10,
                                                     bool fast=false,
                                                     int maxIterations=4,
                                                     std::vector<LineSegment>* linesFound=0);
        
        static cv::Scalar getPaintDiffFromSamples(const std::vector<CBSample> &samples);
        
        static void createOverlayFromMask(const cv::Mat &imgMask, cv::Mat &destImg, cv::Scalar color, int channels=4);
        
        static int applyFloodFill(const cv::Mat &rgbImage,
                                  cv::Mat &mask,
                                  const cv::Point &position,
                                  cv::Scalar color,
                                  cv::Scalar diff,
                                  bool search);
        
        static bool getPolygonalMask(const cv::Mat &maskSrc, cv::Mat &maskDest, std::vector<LineSegment>&foundLines, double epsilon=7, int minLength=0);
        static bool getPolygonalMask(const cv::Mat &maskSrc, cv::Mat &maskDest, std::vector<LineSegment>&longLines, std::vector<cv::Point> &polygons, double epsilon, int minLength);
        
        static bool getLongLinesInMask(const cv::Mat &maskSrc, std::vector<LineSegment>&longLines, std::vector<std::vector<cv::Point> > &contours, double epsilon=7, int minLength=0);
        
        static void refineMask(const cv::Mat &colorImage,
                               const cv::Mat &maskSrc,
                               const cv::Mat &noMask,
                               cv::Mat &maskDest,
                               int erodeAmount = 10,
                               int dilateAmount = 10,
                               bool fast = true,
                               int postDilateAmount=0,
                               int postErodeAmount=0) __attribute__ ((deprecated));
        
        static cv::Mat refineMask(const cv::Mat &reducedMask,
                               const cv::Mat &rgbImage,
                               int dilate,
                               int erode,
                               cv::Mat &dest,
                               cv::Size maxSize = cv::Size(1024,1024));
        
        static void refineROI(const cv::Mat &srcRGB, cv::Mat &mask, const cv::Rect &roi, int dilate, int erode);
        
        static void watershed(const cv::Mat &rgbImage, const cv::Mat &markers, cv::Mat &dest);
        
        static void getPlane(const cv::Mat& img, cv::Mat& dest, int plane);
        static bool hasPixelData(const cv::Mat& img, int inChannel=-1);
        
        static cv::Mat combine(const cv::Mat& red, const cv::Mat& green, const cv::Mat& blue);
        
        static void adjustContrast(const cv::Mat &image, cv::Mat &dest, double alpha, double beta);
        
        static void rotate_image_90n(const cv::Mat &src, cv::Mat &dst, int angle);
        
        static cv::Scalar getWhiteBalanceGainAdjustment(const cv::Mat &src, const cv::Mat &mask=cv::Mat());
        static void adjustWhiteBalance(const cv::Mat &src, cv::Mat &dest, const cv::Mat &mask=cv::Mat(), double gainThreshold=0.03);
        static void applyGain(const cv::Mat &src, cv::Mat &dest, cv::Scalar gain, double gainThreshold=0.03);
        
        static double getOverlayIntensity(const cv::Mat &bwImage, const cv::Mat &msk);
        
        static double getBackgroundIntensity(const cv::Mat &bwImage, const cv::Mat &canny);
        
        static void calculateAlphaBetaGamma(double overlayIntensity,
                                            double backgroundIntensity,
                                            double *alpha,
                                            double *beta,
                                            double *gamma);
        
        static float median(const cv::Mat &hist);
        
        static float getBrightness(const cv::Mat &bwImage, const cv::Mat &mask=cv::Mat());
        
        static void scaleSaturation(const cv::Mat &image, cv::Mat &new_image, double byAmount);
        
        static void overlayMaskOntoRGB(const cv::Mat &mask, cv::Mat &destImage, cv::Scalar color=cv::Scalar(0, 255, 0, 128));
        
        static void makeTexturelessImage(const cv::Mat &rgb, const cv::Mat &hsv, const cv::Mat &bwImage, cv::Mat &dest);
        static int kmeansColor(const cv::Mat &src, cv::Mat &dest, int k=0, cv::Mat *centers=0, cv::Mat *labels=0);
        
        static void maskInRange(const cv::Mat &src, cv::Mat &destMask, int delta=0, const cv::Mat& mask=cv::Mat());
        
        static void sticktoitiveness(cv::Mat &mask, const cv::Mat &toStickTo, int dilateAmount=0, cv::Size maxSize=cv::Size(300,300));
        
        static void shiftImage(const cv::Mat &srcImage, cv::Mat &destImage, cv::Point offset);
        
        static void shiftMask(const cv::Mat &srcMask, cv::Mat &mask, cv::Point offset, bool stretchToBorder);
        
        static void thickFloodFill(const cv::Mat &srcImage, cv::Mat &destImage, cv::Point fillPoint);
        
        static cv::Mat conv2(const cv::Mat& src, const cv::Mat& kernel);
        static void gaborEdges(const cv::Mat& bwImage, cv::Mat& result, int size, double sig, float multiplier=1.0);
        
        static void clahe(const cv::Mat& bwImage, cv::Mat& result, int tileSize = 20, double clipLimit = 0.5);
        
        static cv::Mat normalMap(const cv::Mat& bwTexture, double pStrength = 0.5);
        
        static cv::Mat createShadowsAndHighlights(const cv::Mat &rgbImg, const cv::Size &destSize=cv::Size());
        
        static void matchImageExposures(const cv::Mat &imageA, const cv::Mat &maskA,
                                        const cv::Mat &imageB, const cv::Mat &maskB,
                                        cv::Mat &dest, cv::Size blockSize = cv::Size());
        
        static void blendImages(const cv::Mat &imageA, const cv::Mat &maskA,
                                const cv::Mat &imageB, const cv::Mat &maskB,
                                cv::Mat &mergedImage, cv::Mat &mergedMask,
                                int type = cv::detail::Blender::FEATHER);
        
        static int getLargestContour(const std::vector<std::vector<cv::Point> > &contours);
        
        static void expandImageToROI(const cv::Mat &src, cv::Mat &dest, const cv::Rect &roi);
        
    private:
        static bool _isValidFloodFill(const cv::Mat &img, const cv::Mat &mask, const cv::Point &position, std::vector<cv::Point2f>*validTestPoints=0);
        
        static void _drawStretchRectangles(cv::Mat &mask, const cv::Point &offset);
        static void _drawStretchRectanglesForPos(cv::Mat &mask, cv::Point startPos);
    };
};

#endif /* defined(__ImageProcessing__) */
