//
//  CBP_LargeImage.cpp
//  Cambrian
//
//  Created by Joel Teply on 2/19/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#include "CBP_LargeImage.hpp"
#include <utility/Threads.h>

namespace cbpipe {
    
    struct CBP_LargeImage::Impl
    {
        Impl(CBP_LargeImage *parent, const cv::Mat& image, const cv::Rect& roi)
            : m_parent(parent), m_image(image), m_region(roi) {
            
        }
        
        ~Impl() {
            
        }
        
        cv::Mat operator()( const cv::Rect& roi ) {
            cv::Rect newROI = m_region | roi;
            if (newROI.area() != m_region.area()) {
                //expanding
                cv::Point offset = m_region.tl() - newROI.tl();
                cv::Mat newImage = cv::Mat::zeros(newROI.height, newROI.width, m_image.type());
                
                m_image.copyTo(newImage(cv::Rect(offset.x, offset.y, m_image.cols, m_image.rows)));
                m_image = newImage;
                m_region = newROI;
            }
            cv::Rect adaptedROI = cv::Rect(roi.x - m_region.x, roi.y - m_region.y, roi.width, roi.height);
            return m_image(adaptedROI);
        }
        
        cv::Rect _region() const {
            return m_region;
        }
        
        CBP_LargeImage *m_parent;
        cv::Mat m_image;
        CBMutex m_mutex;
        
        cv::Rect m_region;
    };
    
    CBP_LargeImage::CBP_LargeImage() {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, cv::Mat(), cv::Rect()));
    }
    
    CBP_LargeImage::CBP_LargeImage(const CBP_LargeImage& other) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, other.get(), other.region()));
    }

    CBP_LargeImage::CBP_LargeImage(const cv::Mat& image, const cv::Rect& roi) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, image, roi));
    }
    
    CBP_LargeImage::~CBP_LargeImage() {
        
    }
    
    CBP_LargeImage& CBP_LargeImage::operator = (const CBP_LargeImage &t) {
        m_pImpl->m_image = t.get();
        m_pImpl->m_region = t.region();
        return *this;
    }
    
    cv::Mat CBP_LargeImage::operator()( const cv::Rect& roi ) {
        return m_pImpl->operator()(roi);
    }
    
    cv::Rect CBP_LargeImage::region() const {
        return m_pImpl->_region();
    }
    
    cv::Mat CBP_LargeImage::get() const {
        return m_pImpl->m_image;
    }
};

