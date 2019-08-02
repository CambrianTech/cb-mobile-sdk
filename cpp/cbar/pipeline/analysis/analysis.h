//
//  CBP_AnalyzerThreads.h
//  Cambrian
//
//  Created by Joel Teply on 10/12/16.
//  Copyright © 2016 Joel Teply. All rights reserved.
//

#ifndef CBP_analysis_h
#define CBP_analysis_h

#include <cambrian.h>

#include "CBP_AnalysisUtil.hpp"
#include "CBP_Analyzer.hpp"
#include "CBP_AmbienceSampler.hpp"
#include "CBP_ColorFinder.hpp"

#include "CBP_AnalysisTypes.hpp"
#include "CBP_Segmenter.hpp"
#include "CBP_WallSegmenter.hpp"
#include "CBP_FloorSegmenter.hpp"

//deep networks
#include "CBP_AreaAnalyzer.hpp"
#include "CBP_SemanticAnalyzer.hpp"
#include "CBP_NormalsAnalyzer.hpp"
#include "CBP_ShadowsAnalyzer.hpp"
#include "CBP_ElevationAnalyzer.hpp"

//strucural analysis
#include "CBP_LargeImage.hpp"
#include "CBP_SurfaceAnalyzer.hpp"
#include "CBP_GroundSurfaceAnalyzer.hpp"
#include "CBP_SurfaceAccumulator.hpp"
#include "CBP_DualRangeAccumulator.hpp"
#include "CBP_WallFinder.hpp"
#include "CBP_LineFinder.hpp"
#include "CBP_PlaneAnalyzer.hpp"

#endif
