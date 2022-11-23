//
//  VideoFrame.cpp
//  Cambrian
//
//  Created by Joel Teply on 9/29/15.
//  Copyright © 2015 Joel Teply. All rights reserved.
//

#include "CBAR_VideoFrame.hpp"

#include <imaging/ImageProcessing.h>

#include <utility/CommonUtility.h>
#include <imaging/Accelerated.h>
#include "CBAR_VideoThread.hpp"
#include <imaging/Imaging.h>
#include <imaging/Drawing.h>

using namespace imaging;

namespace cbar {
    
    static int64_t frameCounter = 0;
    //static CBMutex frameCounterMutex("VideoFrameCounter");
    
    int64_t CBAR_VideoFrame::incrementFrameCounter() {
        return ++frameCounter;
    }
    
    int64_t CBAR_VideoFrame::lastFrameIndex() {
        return frameCounter;
    }
    
    struct CBAR_VideoFrame::Impl
    {
        Impl(CBAR_VideoFrame *parent)
        {
            
        }
        
        ~Impl() {
            if (_bytes) {
                delete[] _bytes;
            }
        }
        
        void import_raw_data() {
            std::lock_guard<CBMutex> lockGuard(_importLock);
            
            if (_hasImported || _isDestroyed) {
                return;
            };
            
            if (!_rgbaImage.empty()) {
                import_rgba();
            } else if (!_yuvImage.empty()) {
                import_yuv();
            }
            
            _hasImported = true;
        }
        
        void import_rgba() {
            
            std::vector<cv::Mat>planes;
            cv::split(_rgbaImage, planes);
            
            if (_alphaIsEdges) {
                _edgesImage = planes[3];
            }
            
            _bwImage = planes[1];
            
            if (_isBGR) {
                cv::Mat blue = planes[0];
                planes[0] = planes[2];
                planes[2] = blue;
            }
            
            planes.pop_back();
            cv::merge(planes, _rgbImage);
        }
        
        void import_yuv() {
            if (_yuvImage.data) {
                cv::cvtColor(_yuvImage, _rgbImage, CV_YUV2BGR_NV21);
                cv::cvtColor(_yuvImage, _bwImage, CV_YUV2GRAY_NV21);
            }
        }
        
        CBMutex _importLock;
        bool _isBGR = false;
        bool _isLuminance = false;
        bool _hasImported = false;
        bool _alphaIsEdges = false;
        bool _isPostProcessed = false;
        bool _isDestroyed = false;
        bool _isVideoFrame = false;
        
        int _frameRotation = 0;
        
        cv::Mat _trainingMask;
        
        cv::Size _frameSize;
        
        cv::Mat _yuvImage;
        cv::Mat _rgbaImage;
        cv::Mat _rgbImage;
        cv::Mat _hsvImage;
        cv::Mat _uprightRGBImage;
        cv::Mat _bwImage;
        cv::Mat _flowImage; CBMutex _flowImageMutex;
        cv::Mat _uprightBWImage;
        cv::Mat _analyzerMask;
        
        cv::Mat _enhancedImage;
        CBMutex _enhancedImageLock;
        
        cv::Mat _edgesImage;
        
        cv::Vec3f _orientation;
        
        uintptr_t _textureID = 0;
        
        uchar *_bytes = nullptr;
    };
    
    CBAR_VideoFrame::CBAR_VideoFrame(int frameRotation, bool alphaIsEdges, bool isBGR) : frameIndex(incrementFrameCounter())
    {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
        
        m_pImpl->_frameRotation = frameRotation;
        m_pImpl->_isBGR = isBGR;
        m_pImpl->_alphaIsEdges = alphaIsEdges;
        
        frameTime = sys_usec_time();
    }
    
    CBAR_VideoFrame::CBAR_VideoFrame(const RawFrame &rawFrame)
      : CBAR_VideoFrame(rawFrame.frameRotation, false, true)
    {
        m_pImpl->_frameSize = cv::Size(int(rawFrame.width), int(rawFrame.height));
        m_pImpl->_isVideoFrame = true;
        m_pImpl->_textureID = rawFrame.textureID;
        
        if (rawFrame.frameType == frame_type_420YpCbCr8 || rawFrame.frameType == frame_type_YUV_420_888) {
            //iOS 420YpCbCr8
            int dataHeight = int(rawFrame.height * 3 / 2);
            long dataLen = rawFrame.stride * dataHeight;
            long yuvDataLen = rawFrame.stride * rawFrame.height;
            long chromaDataLen = dataLen - yuvDataLen;
            
            m_pImpl->_bytes = new uchar[dataLen];
            memcpy(m_pImpl->_bytes, rawFrame.data, yuvDataLen);
            memcpy(m_pImpl->_bytes + yuvDataLen, rawFrame.chromaData, chromaDataLen);
            
            m_pImpl->_yuvImage = cv::Mat(dataHeight, int(rawFrame.width), CV_8UC1, m_pImpl->_bytes, rawFrame.stride);
        } else if (rawFrame.dataLength) {
            m_pImpl->_rgbaImage = cv::Mat(int(rawFrame.height), int(rawFrame.width), CV_8UC4);
            memcpy(m_pImpl->_rgbaImage.data, rawFrame.data, rawFrame.dataLength);
        }
    }
    
    CBAR_VideoFrame::CBAR_VideoFrame(size_t width, size_t height, int frameRotation,
                                 unsigned char *outputBuf, size_t outputBufLength, bool isBGR)
    : CBAR_VideoFrame(frameRotation, false, isBGR)
    {
        m_pImpl->_frameSize = cv::Size(int(width), int(height));
        m_pImpl->_rgbaImage = cv::Mat(int(height), int(width), CV_8UC4);
        m_pImpl->_isVideoFrame = true;
        memcpy(m_pImpl->_rgbaImage.data, outputBuf, outputBufLength);
    }
    
    CBAR_VideoFrame::CBAR_VideoFrame(cv::Mat rawImage, int frameRotation, bool alphaIsEdges, bool isBGR)
        : CBAR_VideoFrame(frameRotation, alphaIsEdges, isBGR)
    {
        m_pImpl->_frameSize = rawImage.size();
        m_pImpl->_isLuminance = rawImage.channels() == 1;
        if (m_pImpl->_isLuminance) {
            m_pImpl->_bwImage = rawImage;
        } else {
            if (rawImage.channels() == 3) {
                cv::cvtColor(rawImage, m_pImpl->_rgbaImage, CV_RGB2RGBA);
            } else {
                m_pImpl->_rgbaImage = rawImage;
            }
        }
    }
    
    CBAR_VideoFrame::~CBAR_VideoFrame() {        
        m_pImpl->_isDestroyed = true;
    }
    
    
    
    const cv::Mat & CBAR_VideoFrame::getRGBAImage() {
        m_pImpl->import_raw_data();
        
        if (m_pImpl->_rgbaImage.empty()) {
            if (m_pImpl->_rgbImage.empty()) {
                cv::cvtColor(m_pImpl->_bwImage, m_pImpl->_rgbaImage, CV_GRAY2RGBA);
            } else {
                cv::cvtColor(m_pImpl->_rgbImage, m_pImpl->_rgbaImage, CV_RGB2RGBA);
            }
        }
        return m_pImpl->_rgbaImage;
    }
    
    const cv::Mat & CBAR_VideoFrame::getRGBImage() {
        m_pImpl->import_raw_data();
        
        if (m_pImpl->_rgbImage.empty()) {
            cv::cvtColor(m_pImpl->_bwImage, m_pImpl->_rgbImage, CV_GRAY2RGB);
        }
        return m_pImpl->_rgbImage;
    }
    
    
    const cv::Mat & CBAR_VideoFrame::getHSVImage() {
        if(m_pImpl->_hsvImage.empty()) {
            cv::cvtColor(getRGBImage(), m_pImpl->_hsvImage, CV_RGB2HSV);
        }
        return m_pImpl->_hsvImage;
    }
    
    const cv::Mat & CBAR_VideoFrame::uprightRGBImage() {
        m_pImpl->import_raw_data();
        
        if (m_pImpl->_uprightRGBImage.empty()) {
            ImageProcessing::rotate_image_90n(getRGBImage(), m_pImpl->_uprightRGBImage, m_pImpl->_frameRotation);
        }
        return m_pImpl->_uprightRGBImage;
    }
    
    float CBAR_VideoFrame::aspectRatio() const {
        return float(m_pImpl->_frameSize.width) / float(m_pImpl->_frameSize.height);
    }
    
    const cv::Mat & CBAR_VideoFrame::getBWImage() {
        if (m_pImpl->_bwImage.empty()) m_pImpl->import_raw_data();
        
        return m_pImpl->_bwImage;
    }
    
    const cv::Mat & CBAR_VideoFrame::uprightBWImage() {
        if (m_pImpl->_bwImage.empty()) m_pImpl->import_raw_data();
        
        if (m_pImpl->_uprightBWImage.empty()) {
            ImageProcessing::rotate_image_90n(getBWImage(), m_pImpl->_uprightBWImage, m_pImpl->_frameRotation);
        }
        return m_pImpl->_uprightBWImage;
    }
    
    const cv::Mat& CBAR_VideoFrame::getOpticalFlowImage() {
        std::lock_guard<CBMutex> lockGuard(m_pImpl->_flowImageMutex);
        if (m_pImpl->_flowImage.empty()) {
            for (int i=0; i<OFLOW_IMAGE_PYR_STEPS; i++) {
                cv::Mat src = m_pImpl->_flowImage.empty() ? getBWImage() : m_pImpl->_flowImage;
                cv::pyrDown(src, m_pImpl->_flowImage);
            }
        }
        
        return m_pImpl->_flowImage;
    }
    
    const cv::Mat & CBAR_VideoFrame::getEdgesImage() {
        m_pImpl->import_raw_data();
        
        if (m_pImpl->_edgesImage.empty()) {
            
            double scale = 1;
            double delta = 0;
            cv::Mat sobelX, sobelY;
            cv::Sobel(m_pImpl->_bwImage, sobelX, CV_8U, 1, 0, 5, scale, delta, cv::BORDER_DEFAULT);
            cv::Sobel(m_pImpl->_bwImage, sobelY, CV_8U, 0, 1, 5, scale, delta, cv::BORDER_DEFAULT);
            addWeighted(sobelX, 0.5, sobelY, 0.5, 0, m_pImpl->_edgesImage);
        }
        return m_pImpl->_edgesImage;
    }
    
    const cv::Mat& CBAR_VideoFrame::getEnhancedImage(const std::vector<imaging::LineSegment> &lines) {
        std::lock_guard<CBMutex> lockGuard(m_pImpl->_enhancedImageLock);
        if (m_pImpl->_enhancedImage.empty()) {
            m_pImpl->_enhancedImage = getRGBImage().clone();
            for (const auto &line : lines) {
                cv::line(m_pImpl->_enhancedImage, line.point0, line.point1, cv::Scalar(0,255,0,255), 1, cv::LINE_AA);
            }
        }
        return m_pImpl->_enhancedImage;
    }
    
    cv::Mat CBAR_VideoFrame::getDebuggingImage(const std::vector<cv::Point2f> &points,
                                               const std::vector<imaging::LineSegment> &lines,
                                               int debugMode) {

        m_pImpl->import_raw_data();
        
        cv::Mat debugImage;
        
        if (debugMode == 2) {
            debugImage = getEdgesImage().clone();
        } else if (debugMode == 3) {
            debugImage = getOpticalFlowImage().clone();
        } else {
            debugImage = getRGBImage().clone();
        }
        
        for (const auto &point : points) {
            cv::drawMarker(debugImage, point, cv::Scalar(255,255,0,255));
        }

        for (const auto &line : lines) {
            cv::line(debugImage, line.point0, line.point1, cv::Scalar(255,120,0,255), 1, cv::LINE_AA);
        }
        
        if (debugImage.channels() == 3) {
            cv::cvtColor(debugImage, debugImage, m_pImpl->_isBGR ? CV_RGB2BGRA: CV_RGB2RGBA);
        } else if (debugImage.channels() == 1) {
            cv::cvtColor(debugImage, debugImage, CV_GRAY2RGBA);
        } else if (m_pImpl->_isBGR) {
            cv::cvtColor(debugImage, debugImage, CV_RGBA2BGRA);
        }
        return debugImage;
    }
    
    cv::Rect CBAR_VideoFrame::rotatedROI(const cv::Rect &roi) const {
        cv::Rect rotated;
        
        switch (m_pImpl->_frameRotation) {
            case 0:
                rotated = roi;
                break;
            case 90:
                rotated = cv::Rect(m_pImpl->_frameSize.height - roi.y - roi.height, roi.x, roi.height, roi.width);
                break;
            case 180:
                rotated = cv::Rect(m_pImpl->_frameSize.width - roi.x - roi.width, m_pImpl->_frameSize.height - roi.y - roi.height, roi.height, roi.width);
                break;
            case 270:
                rotated = cv::Rect(roi.y, m_pImpl->_frameSize.width - roi.x - roi.width, roi.height, roi.width);
                break;
        }
        
        return rotated;
    }
    
    const cv::Mat & CBAR_VideoFrame::getTrainingMask() const {
        return m_pImpl->_trainingMask;
    }
    
    void CBAR_VideoFrame::setTrainingMask(const cv::Mat &trainingMask) {
        m_pImpl->_trainingMask = trainingMask;
    }
    
    bool CBAR_VideoFrame::isLuminanceSource() { return m_pImpl->_isLuminance; };
    bool CBAR_VideoFrame::isColorSource() { return !m_pImpl->_isLuminance; };
    
    const cv::Size & CBAR_VideoFrame::frameSize() const {
        return m_pImpl->_frameSize;
    };
    
    const bool CBAR_VideoFrame::isVideoFrame() const {
        return m_pImpl->_isVideoFrame;
    };
    
    void CBAR_VideoFrame::setIsVideoFrame(bool isVideo) {
        m_pImpl->_isVideoFrame = isVideo;
    };
    
    cv::Vec3f CBAR_VideoFrame::getOrientation() const {
        return m_pImpl->_orientation;
    };
    
    int CBAR_VideoFrame::getFrameRotation() const {
        return m_pImpl->_frameRotation;
    };
}
