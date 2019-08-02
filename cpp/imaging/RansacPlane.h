//
//  RansacLineAngles.h
//  Cambrian
//
//  Created by Joel Teply on 7/25/14.
//  Copyright (c) 2014 Joel Teply. All rights reserved.
//

#ifndef __Cambrian__RansacLineAngles__
#define __Cambrian__RansacLineAngles__

#define SCALE_FACTOR 1
#define SCALE_AREA(X) (X*SCALE_FACTOR*SCALE_FACTOR)
#define SCALE(X) (X*SCALE_FACTOR)
#define SHIFT(X) (X+0)

#define LINE_RANSAC_OUTER_ITERATIONS                 20
#define LINE_RANSAC_INNER_ITERATIONS                 20
#define LINE_RANSAC_RADIUS_TOLERANCE                 4.0
#define LINE_RANSAC_DO_LEAST_SQUARES_REFINEMENT      1
#define LINE_RANSAC_MAX_BAD_ITERATIONS               200

#include <opencv2/core/core.hpp>
#include <cambrian.h>
#include "LineProcessing.h"

namespace imaging {
    
    inline int plane_area(const std::vector<cv::Point2f> &plane) {
        if (plane.size()) {
            return cv::contourArea(plane);
        }
        return 0;
    }
    
    inline int distance_from_plane(const std::vector<cv::Point2f> &plane, const cv::Point2f &point) {
        if (plane.size() < 3) return 0;
        
        return cv::pointPolygonTest(plane, point, true);
    }
    
    inline int distance_from_plane(const std::vector<cv::Point2f> &plane, const LineSegment &line) {
        
        int distance0 = distance_from_plane(plane, line[0]);
        int distance1 = distance_from_plane(plane, line[1]);
        return MIN(distance0, distance1);
    }
    
    inline bool plane_equals(const std::vector<cv::Point2f> &plane1, const std::vector<cv::Point2f> &plane2) {
        return plane1[0] == plane2[0]
            && plane1[1] == plane2[1]
            && plane1[2] == plane2[2]
            && plane1[3] == plane2[3];
    }
    
struct DLL_LOCAL RansacPlaneResult
{
    RansacPlaneResult() : score(0.f), inlierCount(0)  {}
    float score;
    float penalty;
    int inlierCount; // in cases where we don't explicitly store the inliers
    std::vector<cv::Point2f> plane;
    std::pair<float,float> scores;
    std::vector<LineSegment> inliers;
    std::vector<LineSegment> sample;
};

class DLL_LOCAL RansacPlane
{
public:
    
    typedef std::vector<LineSegment> LineSet;
    
    typedef void (LinePruneFuncType)(LineSet &, const std::vector<cv::Point2f> &, const LineSet &);
    
    RansacPlane(const cv::Mat &rgbImage);
    ~RansacPlane();
    
    void refine(const LineSet &lines, RansacPlaneResult &result);
    
    void init();
    void clear();
    
    void setRadiusTolerance(float tolerance) { m_recipe.radiusTolerance = tolerance; }
    float getRadiusTolerance() const { return m_recipe.radiusTolerance; }
    
    void setMaxBadIterations(int iterations) { m_maxBadIterations = iterations; }
    
    bool isValidPlane( const std::vector<cv::Point2f> &plane);
    bool isValidSubset( const LineSet &lines);
    int sequential(const std::vector<LineSegment> &haystack,
                   const std::vector<int> &horizontalSeedIndexes,
                   const std::vector<int> &otherHorizontalIndexes,
                   const std::vector<int> &verticalSeedIndexes,
                   const std::vector<int> &otherVerticalIndexes,
                   int numModels, std::vector<RansacPlaneResult> &results, LinePruneFuncType pruner);

    RansacPlaneResult ransac(const LineSet &horizontalSeedLines)
        { return ransac(horizontalSeedLines, m_recipe.outerIterations, m_recipe.innerIterations); }
    
    RansacPlaneResult ransac(const LineSet &horizontalSeedLines,
                             int numIterations, int subIterations = 10 );
    bool step(const LineSet &subsetHorizontal,
              const LineSet &horizontal,
              RansacPlaneResult &best);
    
    float computeDistance(const LineSegment &line, const std::vector<cv::Point2f> &plane, bool &inside);
    std::pair<float,int> computeScore(const LineSet &horizontalLines, const std::vector<cv::Point2f> & pl, std::vector<LineSegment> *inliers=0);
    std::pair<float,int> _computeScore(const LineSet &lines,
                                       bool vertical,
                                       const std::vector<cv::Point2f> & pl,
                                       std::vector<LineSegment> *inliers);
    
    int getInliers(const LineSet &lines, const std::vector<cv::Point2f> &plane, LineSet &inliers);
    
    std::vector<std::vector<cv::Point2f> > m_planes;
    int m_maxBadIterations;
    cv::RNG m_rng;
    
    struct Recipe
    {
        // Ellipse RANSAC parameters:
        int outerIterations           = LINE_RANSAC_OUTER_ITERATIONS;
        int innerIterations           = LINE_RANSAC_INNER_ITERATIONS;
        float radiusTolerance         = LINE_RANSAC_RADIUS_TOLERANCE;
        bool doLeastSquaresRefinement = LINE_RANSAC_DO_LEAST_SQUARES_REFINEMENT;
        
        Recipe() { }
    };
    
private:
    const cv::Mat &m_rgbImage;
    
    LineSet m_sample;
    LineSet m_otherHorizontal;
    LineSet m_verticalSeedLines;
    LineSet m_otherVertical;
    
    int m_rhtScoreTotal = 0;
    float m_rhtScoreMean = 0.f;
    
    float m_minLineSeparation = 10;
    
    Recipe m_recipe;
    Recipe & getRecipe() { return m_recipe; }
    
    bool createPlane(const LineSet &lines, std::vector<cv::Point2f> &plane, std::vector<int> &usedIndexes);
    void _buildLineSet(const std::vector<LineSegment> &haystack,
                       const std::vector<int> &subsetIndexes,
                       LineSet &lineSet);
};

};

#endif /* defined(__Cambrian__RansacLineAngles__) */
