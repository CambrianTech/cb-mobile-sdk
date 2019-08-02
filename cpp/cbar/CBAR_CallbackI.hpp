//
//  CBAR_CallbackI.hpp
//  Cambrian
//
//  Created by Joel Teply on 4/13/17.
//  Copyright © 2017 Joel Teply. All rights reserved.
//

#ifndef CBAR_CallbackI_hpp
#define CBAR_CallbackI_hpp

#include <stdio.h>
#include <cambrian.h>
#include <cbar/pipeline/CBP_Types.hpp>
#include <cbar/pipeline/machine-learning/CBP_MLTypes.hpp>
#include <cbar/pipeline/util/util.h>

namespace cbar {
    
    struct DLL_PUBLIC CBAR_CallbackI {
        
        CBAR_CallbackI(void *context) : m_context(context) {
    
        }
        
        ~CBAR_CallbackI() {
            m_context = 0;
        }
        
        virtual void sceneImageLoaded(const cv::Mat &image) {
            
        }
        
        virtual void historyChanged(std::shared_ptr<cbpipe::UndoState> undo, bool forward) {
            
        }
        
        virtual void colorsCallback(std::vector<cbar::ColorInfo>&colors) {
            
        }
        
        virtual void drawingUpdated(const cv::Mat &image, const cv::Mat &color) {
            
        }
        
        virtual void saveScreenshot(const std::string &path, bool isJPEG, int qualityLevel);
        
        virtual void uploadDiagnosticImageAtPath(const std::string &filePath) { };
        
        virtual void orientationNormalFound(const Eigen::Vector3f &normal) {
            
        }
        
        virtual std::vector<cbpipe::HitTestResult> hitTestAtPoint(const cv::Point2f &point, cbpipe::HitTestResultType resultType);
        
        virtual void loadDeepModel(const cbpipe::CBP_DeepModelInfo &info) = 0;
        
        virtual cv::Mat predict(const cbpipe::CBP_DeepModelInfo &info, const std::map<std::string, cv::Mat> &images) = 0;
        
        virtual void vibrateDevice() {}; //may not be implemented
                
    protected:
        const void *m_context;
    };
}
#endif /* CBAR_CallbackI_hpp */
