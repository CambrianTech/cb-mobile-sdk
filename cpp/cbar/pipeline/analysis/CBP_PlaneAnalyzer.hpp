//
//  CBP_PlaneAnalyzer.hpp
//  Cambrian
//
//  Created by Joel Teply on 2/19/18.
//  Copyright © 2018 Joel Teply. All rights reserved.
//

#ifndef CBP_PlaneAnalyzer_hpp
#define CBP_PlaneAnalyzer_hpp

#include <stdio.h>

#include <opencv2/opencv.hpp>
#include <memory>

#include <cbar/pipeline/analysis/CBP_Analyzer.hpp>
#include <cbar/pipeline/CBP_Types.hpp>

namespace cbpipe {
    
    class plane_contour;
    
    class DLL_LOCAL CBP_PlaneAnalyzer : public CBP_AnalyzerThread {
    public:
        CBP_PlaneAnalyzer();
        ~CBP_PlaneAnalyzer();
        
        virtual bool handleFrame(cbar::CBAR_VideoFramePtr frame);
        
        void anchorAdded(const PlaneAnchor &anchordata);
        void anchorUpdated(const PlaneAnchor &anchordata);
        void anchorRemoved(const PlaneAnchor &anchordata);
        
        bool hasGroundPlane() const;
        float secondsStable() const;
        
        void getGroundPlane(Eigen::Vector3f &center, Eigen::Vector3f &normal);
        
        std::vector<PlaneAnchor> getPlanes(int alignment = 0xff);
        
        std::map<std::string, PlaneAnchor> getAnchors();
        
    private:
        struct Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
}

#endif /* CBP_PlaneAnalyzer_hpp */
