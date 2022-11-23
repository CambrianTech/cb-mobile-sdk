//
//  CBP_ColorFinder.hpp
//  Cambrian
//
//  Created by Joel Teply on 10/12/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#include "CBP_ColorFinder.hpp"

#include <cbar/pipeline/rendering/rendering.h>
#include <cbcommon/CB_Config.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Coloring.h>

#define DEBUG_POSITIONS 0

using namespace imaging;

namespace cbpipe {
    
    struct CBP_ColorFinder::Impl
    {
        Impl(CBP_ColorFinder *parent) : m_parent(parent) {
            
        }

        ~Impl() {}
        
        CBP_ColorFinder *m_parent;
        bool m_hasSentColors = false;
        
        float m_colorFinderUpdateFrequency = 3.0;
        int64_t m_colorFinderLastFrameIndex = -1;
        int64_t m_colorFinderLastUpdateTime = 0;
        
        bool _handle_frame(cbar::CBAR_VideoFramePtr frame) {
            
            auto renderer = CBP_RenderingEngine::sharedInstance(); if (!renderer) return false;
            
            if (!renderer->getARView()
                || !renderer->getCallback()
                || (frame->isVideoFrame() && frame->frameIndex < 60)) return false;
            
            auto toolMode = renderer->getARView()->getToolMode();
            if (toolMode != ToolModeFindColor) return false;
            
            std::vector<cbar::ColorInfo> commonColors;
            
            auto sinceLast = seconds_elapsed(m_colorFinderLastUpdateTime);
            
            bool videoChanged = frame->isVideoFrame() && sinceLast >= m_colorFinderUpdateFrequency;
            bool stillChanged = !frame->isVideoFrame() && frame->frameIndex != m_colorFinderLastFrameIndex;
            
            
            if (stillChanged || videoChanged) {
                commonColors = _get_common_colors(frame);
                
                if (commonColors.size()) {
                    m_colorFinderLastFrameIndex = frame->frameIndex;
                    m_colorFinderLastUpdateTime = sys_usec_time();
                    m_hasSentColors = true;
                    renderer->getCallback()->colorsCallback(commonColors);
                }
            } else {
                return false;
            }
            
            return true;
        }
        
        std::vector<cbar::ColorInfo> _get_common_colors(cbar::CBAR_VideoFramePtr frame) {
            
            CBError("Getting Common Colors");
            std::vector<cbar::ColorInfo> commonColors;
            
            //get colors
            cv::Mat reducedRGB;
            cv::Point margin = cv::Point(frame->frameSize().width * 0.2,
                                         frame->frameSize().height * 0.08);
            
            //move up a bit
            cv::Rect roi = cv::Rect(margin.x, margin.y/2,
                                    frame->frameSize().width - (margin.x * 2),
                                    frame->frameSize().height - (margin.y * 2));
            
            cv::Mat innerRGB = frame->getRGBImage()(roi);
                        
            std::vector<cv::Scalar> colors = Coloring::getMostCommonColors(innerRGB, reducedRGB, 5);
            std::vector<cv::Point> positions = Coloring::getColorPositions(innerRGB, reducedRGB, colors);
            
            for (auto &position : positions) {
                position += margin;
            }
            
#if DEBUG_POSITIONS
            cv::Mat debug = frame->getRGBImage().clone();
            for (int i = 0; i < colors.size(); i++) {
                auto rect = cv::Rect(positions[i].x-25, positions[i].y-25, 50, 50);
                cv::rectangle(debug, rect, colors[i], cv::FILLED);
                cv::rectangle(debug, rect, cv::Scalar::all(0));
            }
            cv::cvtColor(debug, debug, CV_BGR2RGB);
            Diagnostics::SaveDiagnosticImage(false, debug, "colorPositions.png");
#endif
            
            int rotation = frame->getFrameRotation();
            
            for (int i = 0; i < colors.size(); i++) {
                cbar::ColorInfo color = cbar::ColorInfo();

                color.color = Coloring::bgrToRGB(colors[i]);
                
                auto position = Geometry::normalizePoint(positions[i], frame->frameSize());
                color.normalizedPosition = Geometry::rotatePoint90n(position, rotation);
                
                //CBLog("color %i x: %i, y: %i", i, positions[i].x, positions[i].y);
                commonColors.push_back(color);
            }
            
            return commonColors;
        }
    };
    
    CBP_ColorFinder::CBP_ColorFinder() : CBP_AnalyzerThread("CBP_ColorFinder") {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this));
    }
    
    CBP_ColorFinder::~CBP_ColorFinder() {
        
    }
    
    bool CBP_ColorFinder::handleFrame(cbar::CBAR_VideoFramePtr frame) {
        return m_pImpl->_handle_frame(frame);
    }
};
