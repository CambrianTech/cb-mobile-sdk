//
//  Accelerated.cpp
//
//
//  Created by Joel Teply on 7/9/12.
//
//

#include "Accelerated.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <utility/Diagnostics.h>

#ifdef __ANDROID__
#include <fastcv/fastcv.h>
#endif

void Accelerated::dilate(const cv::Mat &src, cv::Mat &dest, int size, bool isElipse)
{
    int iterations = 1;
    int borderType = cv::BORDER_CONSTANT;
    static cv::Scalar borderValue = cv::Scalar(0);

#ifdef __ANDROID__
    if (dest.empty()) {
        dest = cv::Mat(src.rows, src.cols, CV_8UC1);
    }
    
    //Diagnostics::SaveDiagnosticMask(src, "dilate_src.png");
    
    //FASTCV
    fcvStatus result = fcvFilterDilateNxNu8(src.data,
                         src.cols,
                         src.rows,
                         src.step1(),
                         size,
                         dest.data,
                         dest.step1());
    
    Diagnostics::fastCVStatus(result, "Accelerated::dilate");
    
    //Diagnostics::SaveDiagnosticMask(dest, "dilate_dest.png");
#else
    cv::dilate(src, dest, cv::getStructuringElement(isElipse ? CV_SHAPE_ELLIPSE : CV_SHAPE_RECT, cv::Size(size, size)),
               cv::Point(-1,-1), iterations, borderType, borderValue);
#endif
}

void Accelerated::erode(const cv::Mat &src, cv::Mat &dest, int size, bool isElipse)
{
#ifdef __ANDROID__
    if (dest.empty()) {
        dest = cv::Mat(src.rows, src.cols, CV_8UC1);
    }
    
    //Diagnostics::SaveDiagnosticMask(src, "erode_src.png");
    
    //FASTCV
    fcvStatus result = fcvFilterErodeNxNu8(src.data,
                                            src.cols,
                                            src.rows,
                                            src.step1(),
                                            size,
                                            dest.data,
                                            dest.step1());
    
    //Diagnostics::SaveDiagnosticMask(dest, "erode_dest.png");
    
    Diagnostics::fastCVStatus(result, "Accelerated::erode");
#else
    cv::erode(src, dest, cv::getStructuringElement(isElipse ? CV_SHAPE_ELLIPSE : CV_SHAPE_RECT, cv::Size(size, size)));
#endif
}

static void roughErodeDilate(bool isDilate, const cv::Mat &image, cv::Mat &dest, cv::Size size, bool ellipse, double minScale)
{
    cv::Size originalImageSize = cv::Size(image.cols, image.rows);
    cv::Size reducedSize = size;
    double scale = 1.0;
    int maxSize = 7;
    if (reducedSize.width > maxSize) {
        double newScale = MAX(double(maxSize) / double(size.width), minScale);
        if (newScale < scale) {
            scale = newScale;
        }
    }
    if (reducedSize.height > maxSize) {
        double newScale = MAX(double(maxSize) / double(size.height), minScale);
        if (newScale < scale) {
            scale = newScale;
        }
    }
    
    reducedSize.width = scale * size.width;
    reducedSize.height = scale * size.height;
    
    if (reducedSize.width % 2 == 0) {
        reducedSize.width++;
    }
    if (reducedSize.height % 2 == 0) {
        reducedSize.height++;
    }
    
    //int shape = ellipse ? cv::MORPH_ELLIPSE : cv::MORPH_RECT;
    
    auto kernel = cv::getStructuringElement(CV_SHAPE_ELLIPSE, cv::Size(fmax(3,reducedSize.width), fmax(3, reducedSize.height)));
    
    if (reducedSize != size) {
        
        cv::Size newImageSize = cv::Size(scale * image.cols, scale * image.rows);
        cv::resize(image, dest, newImageSize, 0, 0, cv::INTER_NEAREST);
        
        
        if (isDilate) {
            cv::dilate(dest, dest, kernel);
        } else {
            cv::erode(dest, dest, kernel);
        }
        
        cv::resize(dest, dest, originalImageSize, 0, 0, cv::INTER_NEAREST);
    } else {
        if (isDilate) {
            cv::dilate(image, dest, kernel);
        } else {
            cv::erode(image, dest, kernel);
        }
    }
}

void Accelerated::roughErode(const cv::Mat &image, cv::Mat &dest, cv::Size size, bool elipse, double minScale)
{
    roughErodeDilate(false, image, dest, size, elipse, minScale);
}

void Accelerated::roughDilate(const cv::Mat &image, cv::Mat &dest, cv::Size size, bool elipse, double minScale)
{
    roughErodeDilate(true, image, dest, size, elipse, minScale);
}

void
Accelerated::goodFeaturesToTrack(const cv::Mat &imagePrev,
                                 std::vector<cv::Point2f> &corners,
                                 int maxCorners,
                                 double qualityLevel,
                                 double minDistance,
                                 cv::InputArray mask,
                                 int blockSize,
                                 bool useHarrisDetector,
                                 double k)
{
    
#if 0
    uint8_t * src = imagePrev.data;
    unsigned int srcWidth = imagePrev.cols;
    unsigned int srcHeight = imagePrev.rows;
    unsigned int srcStride = imagePrev.step1();
    
    int cornerThreshold = 5; //barrier
    unsigned int border = 7;
    
    uint32_t* featureXY = new uint32_t[2 * maxCorners];
    uint32_t nCorners;
    int score = 10;
    
    fcvCornerHarrisu8(src,
                     srcWidth,
                     srcHeight,
                     srcStride,
                     border,
                     featureXY,
                     maxCorners,
                     &nCorners,
                     score);
    
    corners.resize(nCorners);
    for (int i=0, ii=0; i<nCorners; i++, ii+=2) {
        corners[i].x = featureXY[ii];
        corners[i].y = featureXY[ii+1];
    }
    
    delete[] featureXY;
    
#else
    cv::goodFeaturesToTrack(imagePrev,
                            corners,
                            maxCorners,
                            qualityLevel,
                            minDistance,
                            mask,
                            blockSize,
                            useHarrisDetector,
                            k);
#endif
}

void
Accelerated::calcOpticalFlowPyrLK(const cv::Mat &imagePrev,
                                  const cv::Mat &imageNext,
                                  const std::vector<cv::Point2f> &features_current,
                                  std::vector<cv::Point2f> &features_next,
                                  std::vector<uchar> &status,
                                  std::vector<float> &err,
                                  cv::Size winSize,
                                  int maxLevel,
                                  cv::TermCriteria criteria,
                                  int flags,
                                  double minEigThreshold)
{
    
#ifdef __ANDROID__
    
    static bool setOpMode = false;
    if (!setOpMode) {
        setOpMode = true;
        fcvSetOperationMode((fcvOperationMode) FASTCV_OP_PERFORMANCE);
    }
    
    uint8_t * src1 = imagePrev.data;
    uint8_t * src2 = imageNext.data;
    uint32_t stride = imagePrev.step1();
    uint32_t nPyramidLevels = maxLevel;
    uint32_t width = imagePrev.cols;
    uint32_t height = imagePrev.rows;
    uint32_t featureLen = features_current.size();
    
    //populate CV values
    features_next.resize(featureLen);
    status.resize(featureLen);
    err.resize(featureLen);
    
    //Initialize pyramid structures
    auto src1Pyr = new fcvPyramidLevel_v2[nPyramidLevels];
    auto src2Pyr = new fcvPyramidLevel_v2[nPyramidLevels];
    
    fcvPyramidAllocate_v2( src1Pyr, width, height, stride, 1, nPyramidLevels, 0 );
    fcvPyramidAllocate_v2( src2Pyr, width, height, stride, 1, nPyramidLevels, 0 );
    
    fcvPyramidCreateu8_v2( src1, width, height, stride, nPyramidLevels, src1Pyr );
    fcvPyramidCreateu8_v2( src2, width, height, stride, nPyramidLevels, src2Pyr );
    
    //fcvPyramidSobelGradientCreatei8( src1Pyr, dx1Pyr, dy1Pyr, nPyramidLevels );
    
    float32_t * featureXY = new float32_t[2 * featureLen];
    for (int i=0, ii=0; i<featureLen; i++, ii+=2) {
        featureXY[ii] = features_current[i].x;
        featureXY[ii+1] = features_current[i].y;
    }
    
    float32_t * featureXY_estimate = featureXY;
    float32_t * featureXY_out = new float32_t[2 * featureLen];
    int32_t * featureStatus = new int32_t[featureLen];
    
    int32_t windowWidth = winSize.width;
    int32_t windowHeight = winSize.height;
    int32_t maxIterations = criteria.maxCount;
    float32_t maxEpsilon = criteria.epsilon;
    int32_t use_initial_estimate = 0;
    
    fcvTerminationCriteria termCriteria = FASTCV_TERM_CRITERIA_BOTH;
    if (criteria.type == cv::TermCriteria::COUNT) {
        termCriteria = FASTCV_TERM_CRITERIA_ITERATIONS;
    } else if (criteria.type == cv::TermCriteria::EPS) {
        termCriteria = FASTCV_TERM_CRITERIA_EPSILON;
    }
    
    fcvStatus result = fcvTrackLKOpticalFlowu8_v3	(
                                                     src1,
                                                     src2,
                                                     width,
                                                     height,
                                                     stride,
                                                     src1Pyr,
                                                     src2Pyr,
                                                     featureXY,
                                                     featureXY_estimate,
                                                     featureXY_out,
                                                     featureStatus,
                                                     featureLen,
                                                     windowWidth,
                                                     windowHeight,
                                                     nPyramidLevels,
                                                     termCriteria,
                                                     maxIterations,
                                                     maxEpsilon,
                                                     use_initial_estimate
                                                     );
    
    Diagnostics::fastCVStatus(result, "fcvTrackLKOpticalFlowu8_v3");
    
    fcvPyramidDelete_v2(src1Pyr, nPyramidLevels, 1);
    fcvPyramidDelete_v2(src2Pyr, nPyramidLevels, 1);
    
// featureStatus
//    TRACKED 1
//    NOT_FOUND -1
//    SMALL_DET -2
//    MAX_ITERATIONS -3
//    OUT_OF_BOUNDS -4
//    LARGE_RESIDUE -5
//    SMALL_EIGVAL -6
//    INVALID -99
    
    for (int i=0, ii=0; i<featureLen; i++, ii+=2) {
        features_next[i].x = featureXY_out[ii];
        features_next[i].y = featureXY_out[ii+1];
        status[i] = featureStatus[i] > 0;
    }
    
    delete[] featureXY;
    delete[] featureXY_out;
    delete[] featureStatus;
    
#else
    
    cv::calcOpticalFlowPyrLK(imagePrev,
                             imageNext,
                             features_current,
                             features_next,
                             status,
                             err,
                             winSize,
                             maxLevel,
                             criteria,
                             flags,
                             minEigThreshold);
    
#endif
    
}

int
Accelerated::floodFillMask(const cv::Mat &image, cv::Mat &mask,
          cv::Point seedPoint, cv::Scalar newVal,
          cv::Scalar loDiff, cv::Scalar upDiff)
{
    seedPoint.x = MAX(seedPoint.x, 0);
    seedPoint.x = MIN(seedPoint.x, image.cols - 1);
    
    seedPoint.y = MAX(seedPoint.y, 0);
    seedPoint.y = MIN(seedPoint.y, image.rows - 1);
    
    int result = 0;
    
    cv::Mat imgMask = cv::Mat::zeros(image.rows + 2, image.cols + 2, CV_8UC1);
    
#if 0
    fcvFloodfillSimpleu8(const uint8_t* __restrict src,
                         uint32_t             srcWidth,
                         uint32_t             srcHeight,
                         uint32_t             srcStride,
                         uint8_t* __restrict dst,
                         uint32_t             dstStride,
                         uint32_t             xBegin,
                         uint32_t             yBegin,
                         uint8_t             newVal, //new Val can't be zero. zero is background.
                         fcvConnectedComponent*            cc,
                         uint8_t             connectivity,
                         void*            lineBuffer);
#else
    cv::Mat img = image;
    int flags = cv::FLOODFILL_MASK_ONLY | 4 | 1 << 8;
    result = cv::floodFill(img,
                           imgMask,
                           seedPoint,
                           newVal,
                           0,
                           loDiff,
                           upDiff,
                           flags);
    //undo white border
    static cv::Scalar color = cv::Scalar(0);
    cv::line(imgMask, cv::Point(0,0), cv::Point(imgMask.cols-1, 0), color);
    cv::line(imgMask, cv::Point(imgMask.cols-1,0), cv::Point(imgMask.cols-1, imgMask.rows-1), color);
    cv::line(imgMask, cv::Point(0,imgMask.rows-1), cv::Point(imgMask.cols-1, imgMask.rows-1), color);
    cv::line(imgMask, cv::Point(0,imgMask.rows-1), cv::Point(0, 0), color);
    
    mask = imgMask(cv::Rect(1,1, image.cols, image.rows));
#endif
    
    return result;
}
