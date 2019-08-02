//
//  LineProcessing.h
//  Cambrian
//
//  Created by Joel Teply on 7/24/14.
//  Copyright (c) 2014 Joel Teply. All rights reserved.
//



// Surface Matching in OpenCV using Point Pair Features: https://www.youtube.com/watch?v=uFnqLFznuZU

#ifndef __Cambrian__LineProcessing__
#define __Cambrian__LineProcessing__

#include <opencv2/opencv.hpp>
#include <cbcommon/CB_Types.h>
#include <utility/Diagnostics.h>
#include <vector>
#include <cambrian.h>
#include <utility/CommonUtility.h>
#include <imaging/Geometry.h>
#include <imaging/LineSegment.hpp>

namespace imaging {
    
    struct RansacPlaneResult;
    
    //typedef std::vector<cv::Point> LineSegment;
    
    struct angle_distance {
        double angle;
        double distance;
    };
    
    struct vertice {
        cv::Point vertex;
        cv::Point pointA;
        cv::Point pointB;
        double area;
    };
    
    class DLL_LOCAL LineProcessing
    {
    public:
        
        //static std::vector<LineSegment> findLines(const cv::Mat &srcImage, std::vector<int> rValues=std::vector<int>(), int lineCount=200);
        static std::vector<LineSegment> refineLines(const cv::Mat &srcImage, const cv::Mat &bwImage,
                                                     const std::vector<LineSegment> &lines);
        static void sortLinesByDistance(std::vector<LineSegment> &lines);
        static std::vector<LineSegment> extendLines(const std::vector<LineSegment> &lines, double percentage, bool extensionsOnly);
        
        static void extendLineSegment(LineSegment &line, double amount);
        static LineSegment extendLine(const LineSegment &line, double percentage, double length=0, int fromIndex=-1); //__attribute__ ((deprecated));
        static cv::Point pointOnLine(const LineSegment &line, int distance);
        static bool segmentsIntersect(const LineSegment &line1, const LineSegment &line2, cv::Point2f &intersection);
        static std::vector<LineSegment> linesForPolygons(const std::vector<std::vector <cv::Point> > &polygons);
        
        static void getParallelLines(const LineSegment &nativeLine, int atDistance, LineSegment &lineA, LineSegment &lineB);
        
        static std::vector<vertice> findVerticesForLines(const std::vector<LineSegment> &lines, int count);
        static std::vector<std::vector<vertice> > linkVertices(const std::vector<vertice> &vertices);
        
        static std::vector<LineSegment> combineCollinearLines(const cv::Mat &srcImage,
                                                               const std::vector<LineSegment> &lines,
                                                               double minDegreeThreshold=2,
                                                               double maxDegreeThreshold=12,
                                                               double degreeFactor = 0.3,
                                                               double minDistanceThreshold=3,
                                                               double maxDistanceThreshold=20);
        
        struct line_match_params {
            double sameStdThreshold = 5;
            double sameMeanThreshold = 40;
            
            double meanCandidateThreshold = 20;
            double stdCandidateThreshold = 3;
            
            double extensionMeanThreshold = 70;
            double extensionStdThreshold = 30;
        };
        
        static void extendLinesToVertices(const cv::Mat &srcImage, std::vector<LineSegment> &refinedlines, int lengthThreshold=0);
        static void extendLinesToVertices(const cv::Mat &srcImage, std::vector<LineSegment> &refinedlines,
                                          const std::vector<int> &indexesToProcess, const line_match_params &params);
        
        static bool isValidLineExtension(const cv::Mat &srcImage, const LineSegment &nativeLine, const cv::Point &proposedEndpoint,
                                         int fromIndex, const line_match_params &params);
        static bool linesMatchStatistically(const cv::Mat &srcImage, const LineSegment &nativeLine, const LineSegment &testLine,
                                            const line_match_params &params);
        static void lineMeanStdDev(const cv::Mat &srcImage, const LineSegment &nativeLine,
                                   cv::Scalar &sampleMean, cv::Scalar &sampleStdev);
        
        static void parallelLinesMeanStdDev(const cv::Mat &srcImage, const LineSegment &nativeLine,
                                            cv::Scalar &sampleMeanA, cv::Scalar &sampleStdDevA,
                                            cv::Scalar &sampleMeanB, cv::Scalar &sampleStdDevB,
                                            int startDistance=3, int endDistance=6);
        
        static void filterLinesByDistance(std::vector<LineSegment> &lines, int distanceThreshold);
        
        
        static void findPrimarySecondaryLines(const cv::Mat &srcImage,
                                              const std::vector<LineSegment> &lines,
                                              std::vector<int> &primaryLines,
                                              std::vector<int> &secondaryLines,
                                              double angle=0,
                                              const std::vector<int> &toIgnore = std::vector<int>(), cv::Rect roi=cv::Rect());
        
        static std::vector<RansacPlaneResult> findPlanes(const cv::Mat &rgbImage,
                                                         const std::vector<LineSegment> &haystack,
                                                         const std::vector<int> &horizontalSeedLines,
                                                         const std::vector<int> &otherHorizontalLines,
                                                         const std::vector<int> &verticalSeedLines,
                                                         const std::vector<int> &otherVerticalSeedLines);
        
        static std::vector<LineSegment> guessLines(const std::vector<LineSegment> &lines, int range);
        static int getAngleDistances(const std::vector<LineSegment> &lines, double deltaAngle, std::vector<angle_distance>&angles);
        
        static void drawLines(cv::Mat &img, const std::vector<LineSegment> &lines, cv::Scalar color=cv::Scalar(-1,-1,-1),
                              int thickness=0, int confirmationMultiplier=0);
        static void getLongLinesInContour(const std::vector<cv::Point> &contour, std::vector<LineSegment>&longLines, cv::Size imageSize, int minLength=0);
        
        static void joinCloseParallelLines(std::vector<imaging::LineSegment> &lines, std::vector<int> &lineIndexes,
                                           float parallelAngleTolerance = toRadians(2.0),
                                           float parallelDistanceTolerance = 7,
                                           int useAngle=0);  //useAngle = 0:average 1:first angle, 2:second angle
        static std::vector<imaging::LineSegment> joinCloseParallelLines(std::vector<imaging::LineSegment> &linesA,
                                                                         const std::vector<int> &lineAIndexes,
                                                                         std::vector<imaging::LineSegment> &linesB,
                                                                         const std::vector<int> &lineBIndexes,
                                                                         float parallelAngleTolerance = toRadians(2.0),
                                                                         float parallelDistanceTolerance = 7,
                                                                         int useAngle=0);
                
    private:
        
        
    };
};

#endif /* defined(__Cambrian__LineProcessing__) */
