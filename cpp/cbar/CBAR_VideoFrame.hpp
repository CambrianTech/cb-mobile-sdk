//
//  VideoFrame.hpp
//  Cambrian
//
//  Created by Joel Teply on 9/29/15.
//  Copyright © 2015 Joel Teply. All rights reserved.
//

#ifndef VideoFrame_hpp
#define VideoFrame_hpp

#include <queue>
#include <stdio.h>
#include <cambrian.h>
#include <utility/Threads.h>
#include <cbar/CBAR_Common.hpp>
#include <imaging/LineProcessing.h>

#if __ANDROID__
#   define DEVICE_IS_BGRA 0
#else
#   define DEVICE_IS_BGRA 1
#endif

#define OFLOW_IMAGE_PYR_STEPS 1
#define OFLOW_IMAGE_SCALE powf(0.5, OFLOW_IMAGE_PYR_STEPS)

namespace cbar {
    
class DLL_PUBLIC CBAR_VideoFrame {
public:
    
    CBAR_VideoFrame(const RawFrame &rawFrame);
    
    CBAR_VideoFrame(cv::Mat rawImage,
                  int frameRotation,
                  bool alphaIsEdges=false,
                  bool isBGR=DEVICE_IS_BGRA);
    
    CBAR_VideoFrame(size_t width, size_t height,
                  int frameRotation,
                  unsigned char *outputBuf=0,
                  size_t outputBufLength=0,
                  bool isBGR=DEVICE_IS_BGRA);
    
    ~CBAR_VideoFrame();

    int64_t frameTime;
    
    bool isLuminanceSource();
    bool isColorSource();
    
    const cv::Mat & getEnhancedImage(const std::vector<imaging::LineSegment> &lines);
    void setEnhancedImage(const cv::Mat &image);
    
    const cv::Size &frameSize() const;
    float aspectRatio() const;
    
    const bool isVideoFrame() const;
    void setIsVideoFrame(bool isVideo);
    
    const cv::Mat & getRGBAImage();
    const cv::Mat & getRGBImage();
    const cv::Mat & getHSVImage();
    const cv::Mat & uprightRGBImage();
    const cv::Mat & uprightBWImage();
    const cv::Mat & getBWImage();
    const cv::Mat & getEdgesImage();
    const cv::Mat & getOpticalFlowImage();
    
    const cv::Mat & getTrainingMask() const;
    void setTrainingMask(const cv::Mat &trainingMask);
    
    cv::Mat getDebuggingImage(const std::vector<cv::Point2f> &points = std::vector<cv::Point2f>(),
                              const std::vector<imaging::LineSegment> &lines= std::vector<imaging::LineSegment>(),
                              int debugMode=0);
    
    void generateDebuggingImage(int debugMode, const std::vector<imaging::LineSegment> &lines);
    
    cv::Rect rotatedROI(const cv::Rect &roi) const;
    
    static int64_t lastFrameIndex();
    static int64_t incrementFrameCounter();
    
    cv::Vec3f getOrientation() const;
    int getFrameRotation() const;
    
    const int64_t frameIndex;
    
private:
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;
    
    CBAR_VideoFrame(int frameRotation, bool alphaIsEdges, bool isBGR=DEVICE_IS_BGRA);
};

typedef cv::Ptr<CBAR_VideoFrame> CBAR_VideoFramePtr;
    
}

#endif /* VideoFrame_hpp */
