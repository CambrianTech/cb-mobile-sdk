//
//  RansacLineAngles.cpp
//  Cambrian
//
//  Created by Joel Teply on 7/25/14.
//  Copyright (c) 2014 Joel Teply. All rights reserved.
//

#include "RansacPlane.h"

#include "Imaging.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "Coloring.h"

#include <iostream>
#include <string>
#include <stdio.h>
#include <time.h>
#include <vector>

namespace imaging {
    
    RansacPlane::RansacPlane(const cv::Mat &rgbImage) : m_rgbImage(rgbImage)
    {
        init();
    }
    
    RansacPlane::~RansacPlane() { }
    
    void RansacPlane::init()
    {
        m_maxBadIterations = 100;
    }
    
    void RansacPlane::clear()
    {
        m_planes.clear();
    }
    
    bool RansacPlane::isValidPlane( const std::vector<cv::Point2f> &plane)
    {
        float area = plane_area(plane) > 0;
        if (area == 0) return false;
        
        for (int i=0; i<m_planes.size(); i++) {
            if (plane_equals(plane, m_planes[i])) return false;
        }
        return true;
        
//        //check lines
//        int numValid = 0;
//        int numAwful = 0;
//        
//        std::vector<cv::Scalar>means;
//        for (int i=0; i<plane.size(); i++) {
//            cv::Point pointA = plane[i];
//            cv::Point pointB = plane[(i+1) % plane.size()];
//            
//            LineSegment line;
//            line.push_back(pointA);
//            line.push_back(pointB);
//            
//            cv::Scalar sampleMean, sampleStdDev;
//            LineProcessing::lineMeanStdDev(m_rgbImage, line, sampleMean, sampleStdDev);
//            
//            double stdDev = (sampleStdDev[0] + sampleStdDev[1] + sampleStdDev[2]) / 3;
//            
//            means.push_back(sampleMean);
//            
//            if (stdDev < 30) {
//                numValid ++;
//            } else if (stdDev > 50) {
//                numAwful ++;
//            }
//        }
//        
//        return numAwful == 0 && numValid > 1;
    }
    
    void RansacPlane::refine(const LineSet &lines, RansacPlaneResult &best)
    {
        std::vector<cv::Point2f> tmp = best.plane;
        
        // Do least squares fit of all inliers
        if (best.inliers.size() > 5)
        {
//            LineSet d(best.inliers.size());
//            for (int i = 0; i < best.inliers.size(); i++)
//            {
//                const line_info &line = best.inliers[i];
//                float dx = m_dx.at<float>(p);
//                float dy = m_dy.at<float>(p);
//                d[i] = cv::normalize(cv::Vec2f(dx, dy));
//            }
//            EllipseEstimator estimator(best.inliers, d);
//            cv::RotatedRect ellipse = estimator.fitEllipse();
//            
//            if (isValidLine(plane))
//            {
//                std::swap(ellipse.size.width, ellipse.size.height);
//                ellipse.angle += 90.f;
//                
//                best.inliers.clear();
//                best.plane = plane;
//                getInliers(points, Ellipse(best.ellipse), best.inliers);
//                
//                auto score = computeScore(points, best.ellipse);
//                //std::cout << best.score << " " << score.first << std::endl;
//                
//                best.score = score.first;
//                best.inlierCount = score.second;
//            }
        }
    }
    
    bool RansacPlane::createPlane(const LineSet &horizontalLines, std::vector<cv::Point2f> &plane, std::vector<int> &usedIndexes)
    {
        //DECLARE_TIMING(RansacEllipse__fitEllipse);
        //START_TIMING(RansacEllipse__fitEllipse);
        
        LineSegment topLine;
        int usedIndexTop;
        LineSegment bottomLine;
        int usedIndexBottom;
        
        for (int i=0; i<horizontalLines.size(); i++) {
            LineSegment line = horizontalLines[i];
            if (topLine.invalid || (line.midpoint.y < topLine.midpoint.y)) {
                topLine = line;
                usedIndexTop = i;
            }
            if (bottomLine.invalid || line.midpoint.y > bottomLine.midpoint.y) {
                bottomLine = line;
                usedIndexBottom = i;
            }
        }
        
        if (topLine.invalid || bottomLine.invalid) return false;
        
        LineSegment topLineCopy = topLine;
        if (topLine[0].x > topLine[1].x) {
            topLine[0] = topLineCopy[1];
            topLine[1] = topLineCopy[0];
        }
        
        LineSegment bottomLineCopy = bottomLine;
        if (bottomLine[0].x > bottomLine[1].x) {
            bottomLine[0] = bottomLineCopy[1];
            bottomLine[1] = bottomLineCopy[0];
        }

        if (bottomLine[0].x > topLine[1].x
            || topLine[0].x > bottomLine[1].x) {
            
            return false;
        }

        
        if (bottomLine.length < topLine.length) {
            LineSegment leftLineDown = topLine;
            leftLineDown[1] = topLine[0];
            leftLineDown[1].y = topLine[0].y + 1000;//todo:calculate based upon vertical
            
            LineSegment rightLineDown = topLine;
            rightLineDown[0] = topLine[1];
            rightLineDown[1].y = topLine[1].y + 1000;
            
            LineSegment extendedBottomLine = LineProcessing::extendLine(bottomLine, 3, 0, -1);
            if (bottomLine[0].x > topLine[0].x) {
                //move bottomLine[0].x to topLine[0].x
                cv::Point2f intersection;
                if (Geometry::segmentsIntersect(extendedBottomLine[0], extendedBottomLine[1],
                                                leftLineDown[0], leftLineDown[1], intersection)) {
                    bottomLine[0] = intersection;
                }
            }
            if (bottomLine[1].x < topLine[1].x) {
                //move bottomLine[0].x to topLine[0].x
                cv::Point2f intersection;
                if (Geometry::segmentsIntersect(extendedBottomLine[0], extendedBottomLine[1],
                                                rightLineDown[0], rightLineDown[1], intersection)) {
                    bottomLine[1] = intersection;
                }
            }
        }
        
        if (topLine.length < bottomLine.length) {
            LineSegment leftLineUp = bottomLine;
            leftLineUp[1] = bottomLine[0];
            leftLineUp[1].y = bottomLine[0].y - 1000;//todo:calculate based upon vertical
            
            LineSegment rightLineUp = bottomLine;
            rightLineUp[0] = bottomLine[1];
            rightLineUp[1].y = bottomLine[1].y - 1000;
            
            LineSegment extendedTopLine = LineProcessing::extendLine(topLine, 0.5, 0, -1);
            if (topLine[0].x > bottomLine[0].x) {
                //move bottomLine[0].x to topLine[0].x
                cv::Point2f intersection;
                if (Geometry::segmentsIntersect(extendedTopLine[0], extendedTopLine[1],
                                                leftLineUp[0], leftLineUp[1], intersection)) {
                    topLine[0] = intersection;
                }
            }
            if (topLine[1].x < bottomLine[1].x) {
                //move bottomLine[0].x to topLine[0].x
                cv::Point2f intersection;
                if (Geometry::segmentsIntersect(extendedTopLine[0], extendedTopLine[1],
                                                rightLineUp[0], rightLineUp[1], intersection)) {
                    topLine[1] = intersection;
                }
            }
        }
        
        if (fabs(bottomLine[0].x - topLine[0].x) > 1 || fabs(bottomLine[1].x - topLine[1].x) > 1 )  {
            return false;
        }
        
        usedIndexes.push_back(usedIndexTop);
        usedIndexes.push_back(usedIndexBottom);
        
        plane.push_back(topLine[0]);
        plane.push_back(topLine[1]);
        plane.push_back(bottomLine[1]);
        plane.push_back(bottomLine[0]);

        //STOP_TIMING(RansacEllipse__fitEllipse);
        //std::cout << "RansacEllipse__fitEllipse: " << GET_TIMING(RansacEllipse__fitEllipse) << std::endl;
        
        return true;
    }
    
    bool RansacPlane::step(const LineSet &subsetHorizontal,
                           const LineSet &horizontal,
                           RansacPlaneResult &best)
    {
        bool valid = true;
        
        m_sample.resize(4);
        
        // get N random lines
        for (int i = 0; i < m_sample.size(); i++)
            m_sample[i] = subsetHorizontal[m_rng.uniform((int)0, (int)subsetHorizontal.size())];
        
        // min distance check
        for(int i = 0; i < m_sample.size(); i++) {
            for(int j = 0; j < m_sample.size(); j++) {
                double distance = Geometry::distanceBetweenSegments(m_sample[i][0], m_sample[i][1], m_sample[j][0], m_sample[j][1]);
                if((i != j) &&  distance < m_minLineSeparation)
                    return false;
            }
        }
        
        cv::Point2f center;
        std::vector<cv::Point2f> pl;
        
        std::vector<int>usedIndexes;
        bool success = createPlane(m_sample, pl, usedIndexes);
        if (!success) return false;
        
        if (!isValidPlane(pl)) {
            return false;
        }
        
        LineSet samples;
        for(int i = 0; i < usedIndexes.size(); i++) {
            samples.push_back(m_sample[usedIndexes[i]]);
        }
        m_sample = samples;
        
        //adjust plane so that top and bottom extend to match vertical lines
        std::pair<float,float> scores(0,0);
        auto score = computeScore(horizontal, pl);
        
        if(score.first > best.score)
        {
            best.plane = pl;
            best.inlierCount = score.second;
            best.score = score.first;
            best.sample = m_sample;
        }
        
        return valid;
    }
    
    RansacPlaneResult RansacPlane::ransac(const LineSet &horizontalSeedLines, int numIterations, int subIterations )
    {
        m_rng.state = 0xFFFFFFFF;
        
        RansacPlaneResult best;
        
        best.score = 0;
        float score = best.score;
        
        int i = 0, j = 0, valid = 0, bogus = 0;
        //    EVLog("numIter=%d, subIter %d\n", numIterations, subIterations);
        while(i < numIterations)
        {
            valid = step(horizontalSeedLines, horizontalSeedLines, best);
            i += valid;
            bogus = (valid == 0) ? (bogus + 1) : 0;
            if(bogus >= m_maxBadIterations)
            {
                break;
            }
            
            // std::cout << "score[" << i << "] :" << score << std::endl;
            
            if(best.score < score)
            {
                score = best.score;
                
                best.inliers.clear();
                getInliers(horizontalSeedLines, best.plane, best.inliers);
                
                j = bogus = 0;
                int bogusSubiterations = 0;
                while(best.inliers.size() && (j < subIterations) && (bogusSubiterations < m_maxBadIterations))
                {
                    valid = step(best.inliers, horizontalSeedLines, best);
                    j += valid;
                    bogusSubiterations = (valid == 0) ? (bogusSubiterations + 1) : 0;
                    if(best.score < score)
                    {
                        score = best.score; // don't update the inliers at in the sub iteration
                    }
                }
            }
        }
        
        // std::cout << best.ellipse.center << " " << best.ellipse.size.width << " " << best.ellipse.size.height << std::endl;
        best.inliers.clear();
        getInliers(horizontalSeedLines, best.plane, best.inliers);
        
        /*
        if( best.score < std::numeric_limits<float>::infinity() && m_recipe.doLeastSquaresRefinement)
        {
            for (int n = 0; n < 10; n++)
            {
                RansacPlaneResult tmp = best;
                refine(horizontalSeedLines, tmp);
                if(tmp.score < best.score)
                {
                    best = tmp;
                }
                else break;
            }
        }*/
        
        return best;
    }
    
    int RansacPlane::getInliers(const LineSet &lines, const std::vector<cv::Point2f> &pl, LineSet &inliers)
    {
        float r2 = m_recipe.radiusTolerance * m_recipe.radiusTolerance;
        bool inside = false;
        inliers.reserve(inliers.size() + lines.size());
        for (int i = 0; i < lines.size(); i++)
        {
            double distance = computeDistance(lines[i], pl, inside);
            if (distance < r2)
                inliers.push_back(lines[i]);
        }
        return int(inliers.size());
    }
    
    inline bool sortLinesByAngle(const LineSegment &p, const LineSegment &q)
    {
        return (p.angle < q.angle) || ((p.angle == q.angle) && (p.length > q.length));
    }
    
    inline bool sortLinesByIndex(const LineSegment &p, const LineSegment &q)
    {
        return p.index > q.index;
    }
    
    void RansacPlane::_buildLineSet(const std::vector<LineSegment> &haystack,
                                    const std::vector<int> &subsetIndexes,
                                    LineSet &lineSet) {
        
        for (int ki=0; ki<subsetIndexes.size(); ki++) {
            int index = subsetIndexes[ki];
            LineSegment lineInfo = haystack[index];
            lineSet.push_back(lineInfo);
        }
    }
    
    int RansacPlane::sequential(const std::vector<LineSegment> &haystack,
                                const std::vector<int> &horizontalSeedIndexes,
                                const std::vector<int> &otherHorizontalIndexes,
                                const std::vector<int> &verticalSeedIndexes,
                                const std::vector<int> &otherVerticalIndexes,
                                int numModels, std::vector<RansacPlaneResult> &results, LinePruneFuncType pruner)
    {
        m_planes.clear();
        
        m_rhtScoreTotal = 0;
        m_rhtScoreMean = 0.f;
        
        m_rng.state = 0xFFFFFFFF;
        
        //horizontal
        LineSet horizontalSeedLines_;
        _buildLineSet(haystack, horizontalSeedIndexes, horizontalSeedLines_);
        std::sort(horizontalSeedLines_.begin(), horizontalSeedLines_.end(), sortLinesByAngle);
        LineSet horizontalSeedLines = horizontalSeedLines_;
        
        _buildLineSet(haystack, otherHorizontalIndexes, m_otherHorizontal);
        std::sort(m_otherHorizontal.begin(), m_otherHorizontal.end(), sortLinesByAngle);
        
        //vertical
        _buildLineSet(haystack, verticalSeedIndexes, m_verticalSeedLines);
        
        LineSet otherVerticalLines;
        _buildLineSet(haystack, otherVerticalIndexes, m_otherVertical);
        
        results.reserve(results.size() + ((numModels > horizontalSeedIndexes.size()) ? numModels : horizontalSeedIndexes.size()));
        for (int i = 0; (i < numModels) && (horizontalSeedIndexes.size() > 1); i++)
        {
            RansacPlaneResult result = ransac(horizontalSeedLines, m_recipe.outerIterations, m_recipe.innerIterations);
            if(result.inliers.size() == 0 || result.plane.size() != 4)
                break;
            
            if (pruner) {
                //pruner(horizontalLines, result.plane, result.inliers);
            }
            else {
                //remove items in plane itself
                std::vector<LineSegment> inliersFiltered;
                for (int j=0; j<result.inliers.size(); j++) {
                    bool partOfPlane = false;
                    for (int k=0; k<m_sample.size(); k++) {
                        if (result.inliers[j].index == m_sample[k].index) {
                            partOfPlane = true;
                            break;
                        }
                    }
                    if (!partOfPlane) {
                        inliersFiltered.push_back(result.inliers[j]);
                    }
                }
                result.inliers = inliersFiltered;
                
                LineSet backup = horizontalSeedLines;
                horizontalSeedLines.clear();
                std::sort(result.inliers.begin(), result.inliers.end(), sortLinesByAngle);
                std::set_difference(backup.begin(), backup.end(), result.inliers.begin(), result.inliers.end(),
                                    std::back_inserter(horizontalSeedLines), sortLinesByAngle);
            }
            
            auto score = computeScore(horizontalSeedLines_, result.plane); // compute score with original points
            result.score = score.first;
            result.inlierCount = score.second;
            results.push_back(result);
            m_planes.push_back(result.plane);
        }
        
        { // Sort the planes
            auto greater = [](const std::vector<cv::Point2f> &x, const std::vector<cv::Point2f> &y) { return plane_area(x) > plane_area(y); };
            std::sort(m_planes.begin(), m_planes.end(), greater); // sort in descending order
        }
        
        { // Sort the RANSAC planes
            auto lesser = [](const RansacPlaneResult &x, const RansacPlaneResult &y) { return x.score < y.score; };
            std::sort(results.begin(), results.end(), lesser);
        }
        
        m_rhtScoreMean = (m_rhtScoreMean / (float(m_rhtScoreTotal)+1e-6f) );
        
        return int(results.size());
    };
    
    float RansacPlane::computeDistance(const LineSegment &line, const std::vector<cv::Point2f> &pl, bool &inside)
    {
        
        float d = distance_from_plane(pl, line);
        inside = (d < 0);
        
        return fabs(d);
    }
    
    std::pair<float,int> RansacPlane::_computeScore(const LineSet &lines,
                                                    bool vertical,
                                                    const std::vector<cv::Point2f> & pl,
                                                    std::vector<LineSegment> *inliers) {
        
        std::pair<float, int> score(0,0);
        if (pl.size() != 4) return score;
        
        double firstAngle, secondAngle;
        //get a line bisecting both top and bottom segments, used for then aligning sublines
        LineSegment midline;
        if (vertical) {
            firstAngle = Geometry::angleOfPoints(pl[0], pl[3]);
            secondAngle = Geometry::angleOfPoints(pl[1], pl[2]);
            midline = LineSegment(Geometry::midpoint(pl[0], pl[3]), Geometry::midpoint(pl[1], pl[2]));
        }
        else {
            firstAngle = Geometry::angleOfPoints(pl[0], pl[1]);
            secondAngle = Geometry::angleOfPoints(pl[2], pl[3]);
            midline = LineSegment(Geometry::midpoint(pl[0], pl[1]), Geometry::midpoint(pl[2], pl[3]));
        }
        
        firstAngle = Geometry::lineAngleDifference(firstAngle, 0);
        secondAngle = Geometry::lineAngleDifference(secondAngle, 0);
        
        double midlineLength = Geometry::euclideanDistance(midline[0], midline[1]);
        double deltaAngle = secondAngle - firstAngle;
        double deltaAnglePer = deltaAngle / midlineLength;
        double angleError = MAX(fabs(deltaAngle) / 3.0f, 3.0f);
        
        //float r2 = m_recipe.radiusTolerance * m_recipe.radiusTolerance;
        
        for (int i = 0; i < lines.size(); i++)
        {
            bool inside = false;
            float d2 = computeDistance(lines[i], pl, inside);
            if (inside || d2==0)
            {
                LineSegment extendedLine = LineProcessing::extendLine(lines[i], 0, 1000);
                cv::Point2f intersection;
                Geometry::segmentsIntersect(extendedLine[0], extendedLine[1], midline[0], midline[1], intersection);
                
                double topDistance = Geometry::euclideanDistance(intersection, midline[0]);
                double lineAngle = lines[i].angle;
                lineAngle = Geometry::lineAngleDifference(lineAngle, 0);
                
                double predictedAngle;
                
                predictedAngle = firstAngle + deltaAnglePer * topDistance;
                double diffAngle = Geometry::lineAngleDifference(predictedAngle, lineAngle);
                
                if (diffAngle <= angleError)
                {
                    double scoreValue;
                    if (diffAngle == 0 && d2 == 0) {
                        //on the plane line
                        scoreValue = powf(lines[i].length, 2);
                    } else {
                        scoreValue = lines[i].length / sqrt(MAX(d2, 1));
                    }
                    
                    score.first += scoreValue;
                    score.second ++;
                    if (inliers) inliers->push_back(lines[i]);
                }
                
            }
        }
        
        return score;
    }
    
    std::pair<float,int> RansacPlane::computeScore(const LineSet &horizontalLines, const std::vector<cv::Point2f> & pl, std::vector<LineSegment> *inliers)
    {
        std::pair<float, int> score(0,0);
        
        if (pl.size() == 0) {
            return score;
        }
        
        std::vector<LineSegment> moreInliers;
        std::pair<float, int>hScore = _computeScore(horizontalLines, false, pl, inliers);
        score.first += hScore.first;
        score.second += hScore.second;
        
        std::pair<float, int>hExtraScore = _computeScore(m_verticalSeedLines, true, pl, 0);
        score.first += hExtraScore.first;
        score.second += hExtraScore.second;
        
        double totalLength = 0;
        for (int i=0; i<m_sample.size(); i++) {
            totalLength += m_sample[i].length;
        }
        double generatedLength = Geometry::euclideanDistance(pl[0], pl[1]);
        generatedLength += Geometry::euclideanDistance(pl[2], pl[3]);
        generatedLength -= totalLength;
        
        score.first *= sqrt(plane_area(pl));
        score.first *= pow(totalLength - generatedLength, 3);
  
        return score;
    }
    
    
    
};

