//
//  LineProcessing.cpp
//  Cambrian
//
//  Created by Joel Teply on 7/24/14.
//  Copyright (c) 2014 Joel Teply. All rights reserved.
//

#include "LineProcessing.h"

#include "Coloring.h"
#include "Drawing.h"
#include "Geometry.h"
#include "ImageProcessing.h"
#include "Imaging.h"
#include <cbcommon/CB_Config.h>
#include "RansacPlane.h"
#include <imaging/Accelerated.h>

#define LOG_LINE(identifier, LineSegment) CBLog("%s Line (%d,%d) to (%d,%d)", identifier, int(LineSegment[0].x), int(LineSegment[0].y), int(LineSegment[1].x), int(LineSegment[1].y));

using namespace std;

namespace imaging {

    inline int euclideanLineDistance(LineSegment lineA, LineSegment lineB);
    
    static bool isIndexed(int index, const std::vector<int>&indexesCombined) {
        
        for (int i=0; i<indexesCombined.size(); i++) {
            if (index == indexesCombined[i]) {
                return true;
            }
        }
        
        return false;
    }
    
    inline double _similarLineScore(double distance, double angleDifference) {
        return distance * pow(angleDifference, 2);
    }
    
    
    void _getMatchingLines(const std::vector<LineSegment> &haystack, const std::vector<int> &indexesToSearch,
                           int lines_index, std::vector<int> &matches,
                           int distanceThreshold, int angularThreshold, int matchSizeLimit) {
        
        const LineSegment line = haystack[lines_index];
        double lineAngle = Geometry::angleOfPoints(line[0], line[1]);
        
        double minScoreThreshold = _similarLineScore(distanceThreshold * 2, angularThreshold);
        
        for (int ki=0; matches.size() < matchSizeLimit && ki<indexesToSearch.size(); ki++) {
            int index = indexesToSearch[ki];
            
            if (index == lines_index) continue;
            
            LineSegment testLine = haystack[index];
            
            double distance = Geometry::distanceBetweenSegments(line[0], line[1], testLine[0], testLine[1]);
            if (distance > distanceThreshold) {
                //too far away, not a match
                continue;
            }
            
            double testAngle = Geometry::angleOfPoints(testLine[0], testLine[1]);
            double angleDifference = Geometry::lineAngleDifference(lineAngle, testAngle);
            if (angleDifference > angularThreshold) {
                //too different angle, not a match
                continue;
            }
            
            double score = _similarLineScore(distance, angleDifference);
            
            if (score < minScoreThreshold) {
                matches.push_back(index);
            }
        }
    }
    
    void _findMostSimilarLines(const std::vector<LineSegment> &haystack, const std::vector<int> &indexesToSearch,
                               int lines_index, std::vector<int> &matches, int matchSizeLimit,
                               int distanceThreshold, int angularThreshold) {
        
        _getMatchingLines(haystack, indexesToSearch, lines_index, matches, distanceThreshold, angularThreshold, matchSizeLimit);
    }
    
    int hasSimilarLine(const std::vector<LineSegment> &haystack, const std::vector<int> &indexesToSearch,
                       int lines_index, int distanceThreshold = 10, int angularThreshold = 10) {
        
        std::vector<int>matches;
        _getMatchingLines(haystack, indexesToSearch, lines_index, matches, distanceThreshold, angularThreshold, 1);
        
        return matches.size() != 0;
    }
    
    struct DLL_LOCAL line_run_element {
        int index;
        LineSegment line;
        double angle;
        
        line_run_element(LineSegment ln, int i) : line(ln), index(i)
        {
            angle = Geometry::angleOfPoints(line[0], line[1]);
        }
    };
    
    std::vector<RansacPlaneResult> LineProcessing::findPlanes(const cv::Mat &rgbImage,
                                                              const std::vector<LineSegment> &haystack,
                                                              const std::vector<int> &horizontalSeedLines,
                                                              const std::vector<int> &otherHorizontalLines,
                                                              const std::vector<int> &verticalSeedLines,
                                                              const std::vector<int> &otherVerticalSeedLines) {
        
        RansacPlane ransacPlane = RansacPlane(rgbImage);
        
        std::vector<RansacPlaneResult>results;
        
        ransacPlane.sequential(haystack, horizontalSeedLines, otherHorizontalLines, verticalSeedLines, otherVerticalSeedLines, 5, results, 0);
        
        CBLog("Results = %zu", results.size());
//        for (int ki=0; ki<indexesToSearch.size(); ki++) {
//            int index = indexesToSearch[ki];
//            
//            std::vector<int>seedCandidates;
//            LineSegment line = haystack[index];
//            double length = Geometry::euclideanDistance(line[0], line[1]);
//            
//            _findMostSimilarLines(haystack, indexesToSearch, index, seedCandidates, length / 2, 30, 5);
//            
//            if (seedCandidates.size()) {
//                LOG_LINE("Run Line Candidate", haystack[index]);
//                CBLog("Searching found %d matches", seedCandidates.size());
//                
//            }
//        }
        
        //sort
        
        return results;
    }
    
    bool sortLinesByDistanceComparator(const LineSegment &lineA, const LineSegment &lineB)
    {
        double distanceA = Geometry::euclideanDistance(lineA[0], lineA[1]);
        double distanceB = Geometry::euclideanDistance(lineB[0], lineB[1]);
        
        return distanceA > distanceB;
    }
    
    void LineProcessing::sortLinesByDistance(std::vector<LineSegment> &lines) {
        sort(lines.begin(), lines.end(), sortLinesByDistanceComparator);
    }
    
    void LineProcessing::filterLinesByDistance(std::vector<LineSegment> &lines, int distanceThreshold) {
        int index = 0;
        for (index=0; index<lines.size(); index++) {
            double distance = Geometry::euclideanDistance(lines[index][0], lines[index][1]);
            if (distance < distanceThreshold) break;
        }
        lines = std::vector<imaging::LineSegment>(lines.begin(), lines.begin() + index);
    }
    
    
    inline uchar getBWPointAt(const cv::Mat &bwImage, cv::Point point) {
        if (point.y > 0 && point.x > 0
            && point.x < bwImage.cols && point.y < bwImage.rows) {
            return bwImage.at<uchar>(point.y, point.x);
        }
        return 0;
    }
    
    std::vector<LineSegment> LineProcessing::refineLines(const cv::Mat &srcImage, const cv::Mat &bwImg,
                                                           const std::vector<LineSegment> &lines)
    {
        std::vector<LineSegment> extendedlines = extendLines(lines, 0.4, false);
        //std::vector<LineSegment> lineGuesses = guessLines(lines, bwImage.rows/10);
        
        cv::Mat markers = cv::Mat(srcImage.rows, srcImage.cols, CV_8UC1);
        markers.setTo(cv::Scalar(127));
        
        std::vector<angle_distance>angles;
        //int totalLength = getAngleDistances(lines, 2, angles);
        
        //        for (int i=0; i<angles.size(); i++) {
        //            CBLog("angle:%d, %d, which is %f", int(angles[i].angle), int(angles[i].distance), angles[i].distance/totalLength);
        //        }
        
        //drawLines(markers, lineGuesses, cv::Scalar(200), 2);
        drawLines(markers, extendedlines, cv::Scalar(0), 2);
        
        cv::Mat linesImg = cv::Mat::zeros(markers.rows, markers.cols, CV_8UC1);
        drawLines(linesImg, lines, cv::Scalar(255), 2);
        //cv::erode(linesImg, linesImg, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
        //cv::blur(linesImg, linesImg, cv::Size(5, 4));
        //cv::GaussianBlur(linesImg, linesImg, cv::Size(5,5), 1, 1, cv::BORDER_DEFAULT);
        
        markers += linesImg;
        
        //        cv::Mat sobel, sobelMarkers;
        //        drawSobel(bwImage, sobel);
        //        cv::threshold(sobel, sobel, 100, 255, CV_THRESH_BINARY);
        //        cv::erode(sobel, sobel, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(1, 1)));
        //        //LOG_IMAGE(sobel, "sobel.png");
        //        markers |= sobel;
        
        //LOG_IMAGE(markers, "markers.png");
        
        //cv::Mat watershedResult = cv::Mat::zeros(srcImage.rows, srcImage.cols, CV_8UC1);
        cv::Mat watershedResult;
        ImageProcessing::watershed(srcImage, markers, watershedResult);
        
        cv::threshold(watershedResult, watershedResult, 254, 255, CV_THRESH_BINARY);
        
        cv::Mat canny;
        cv::equalizeHist(bwImg, canny);
        cv::Canny(canny, canny, 45, 250);
        Accelerated::roughDilate(canny, canny, cv::Size(2, 2), false);
        //LOG_IMAGE(canny, "canny.png");
        
        watershedResult |= canny;
        
        //LOG_IMAGE(watershedResult, "watershedResult.png");
        
       //LOG_IMAGE(watershedResult, "watershedResult.png");
        //        cv::Mat polygons = cv::Mat::zeros(srcImage.rows, srcImage.cols, CV_8UC1);
        //        ImageProcessing::drawPolygons(watershedResult, polygons, cv::Scalar::all(255));
        //        LOG_IMAGE(polygons, "polygons.png");
        
        std::vector<LineSegment> refinedlines = lines;
        
        //return refinedlines;
        //LOG_IMAGE(watershedResult, "watershed.png");
        
        //now go over extended lines and increase lines while validating results
        for (int i=0; i<refinedlines.size(); i++) {
            LineSegment nativeLine = refinedlines[i];
            double lineLength = Geometry::euclideanDistance(nativeLine[0], nativeLine[1]);
            
            uchar value0 = 1;
            uchar value1 = 1;
            double maxExtension = lineLength * 2;
            
            for (int d=1; (value0 || value1) && d<maxExtension; d++) {
                
                LineSegment testLine = extendLine(nativeLine, 0, d);
                LineSegment testLineA, testLineB;
                getParallelLines(testLine, 2, testLineA, testLineB);
                
                if (value0)  {
                    value0 = getBWPointAt(watershedResult, testLine[0]);
                    if (value0) {
                        refinedlines[i][0].x = testLine[0].x;
                        refinedlines[i][0].y = testLine[0].y;
                    } else if ((value0 = getBWPointAt(watershedResult, testLineA[0]))) {
                        refinedlines[i][0].x = testLineA[0].x;
                        refinedlines[i][0].y = testLineA[0].y;
                    } else if ((value0 = getBWPointAt(watershedResult, testLineB[0]))) {
                        refinedlines[i][0].x = testLineB[0].x;
                        refinedlines[i][0].y = testLineB[0].y;
                    }
                }
                
                if (value1) {
                    value1 = getBWPointAt(watershedResult, testLine[1]);
                    if (value1) {
                        refinedlines[i][1].x = testLine[1].x;
                        refinedlines[i][1].y = testLine[1].y;
                    } else if ((value1 = getBWPointAt(watershedResult, testLineA[1]))) {
                        refinedlines[i][1].x = testLineA[1].x;
                        refinedlines[i][1].y = testLineA[1].y;
                    } else if ((value1 = getBWPointAt(watershedResult, testLineB[1]))) {
                        refinedlines[i][1].x = testLineB[1].x;
                        refinedlines[i][1].y = testLineB[1].y;
                    }
                }
                //CBLog("testPoint: %i,%i, %d", testPointA.x, testPointA.y, value);
            }
        }
        
//        cv::Mat logImage = cv::Mat::zeros(watershedResult.rows, watershedResult.cols, CV_8UC1);
//        drawLines(logImage, refinedlines, cv::Scalar(255), 2);
//        LOG_IMAGE(logImage, "refined.png");
        
        return refinedlines;
    }
    
    LineSegment createLineSegment(std::vector<cv::Point>points) {
        
        double maxDistance = 0;
        cv::Point maxDistancePoint1;
        cv::Point maxDistancePoint2;
        for (int i=0; i<points.size(); i++) {
            cv::Point point1 = points[i];
            for (int j=i+1; j<points.size(); j++) {
                cv::Point point2 = points[j];
                double distance = Geometry::euclideanDistance(point1, point2);
                if (distance > maxDistance) {
                    maxDistance = distance;
                    maxDistancePoint1 = point1;
                    maxDistancePoint2 = point2;
                }
            }
        }
        
        return LineSegment(maxDistancePoint1, maxDistancePoint2);
    }
    
#define TRACKED_LINE_INDEX 2
    
    void LineProcessing::extendLinesToVertices(const cv::Mat &srcImage,
                                                std::vector<LineSegment> &refinedlines,
                                                int lengthThreshold)
    {
        std::vector<int> indexesToProcess;
        
        for (int i=0; i<refinedlines.size(); i++) {
            LineSegment nativeLine = refinedlines[i];
            double lineLength = Geometry::euclideanDistance(nativeLine[0], nativeLine[1]);
            
            if (lengthThreshold && lineLength < lengthThreshold) break;
            
            indexesToProcess.push_back(i);
        }
        
        line_match_params params;
        extendLinesToVertices(srcImage, refinedlines, indexesToProcess, params);
    }
    
    void LineProcessing::extendLinesToVertices(const cv::Mat &srcImage,
                                                std::vector<LineSegment> &refinedlines,
                                                const std::vector<int> &indexesToProcess,
                                                const line_match_params &params) {
        
        std::vector<LineSegment> borders;
        LineSegment segmentN = LineSegment(cv::Point(0,0), cv::Point(srcImage.cols, 0));
        borders.push_back(segmentN);
        
        LineSegment segmentE = LineSegment(cv::Point(srcImage.cols, 0), cv::Point(srcImage.cols, srcImage.rows));
        borders.push_back(segmentE);
        
        LineSegment segmentS = LineSegment(cv::Point(0,srcImage.rows), cv::Point(srcImage.cols, srcImage.rows));
        borders.push_back(segmentS);
        
        LineSegment segmentW = LineSegment(cv::Point(0, 0), cv::Point(0, srcImage.rows));
        borders.push_back(segmentW);
        
        //now go over extended lines and increase lines while validating results
        for (int ki=0; ki<indexesToProcess.size(); ki++) {
            int index = indexesToProcess[ki];
            LineSegment nativeLine = refinedlines[index];
            double lineLength = Geometry::euclideanDistance(nativeLine[0], nativeLine[1]);
            
            LineSegment nativeLineExtended = extendLine(nativeLine, 0.3);
            
            cv::Point bestIntersection0 = cv::Point();
            int bestJ0 = -1;
            cv::Point bestIntersection1 = cv::Point();
            int bestJ1 = -1;
            double bestLineDistanceToInt0 = INT_MAX, bestLineDistanceToInt1 = INT_MAX;
            double bestTestDistanceToInt0 = INT_MAX, bestTestDistanceToInt1 = INT_MAX;
            bool bestIsPerpendicular0 = false, bestIsPerpendicular1 = false;
            bool bestIsParallel0 = false, bestIsParallel1 = false;
            
            
            
            bool isTrackedLine = index == TRACKED_LINE_INDEX;
            
            if (isTrackedLine) {
                LOG_LINE("Tracked line", nativeLine);
            }
            
            bool isAtIntersectionAlready0 = false, isAtIntersectionAlready1 = false;
            
            //extend to vertex, if nearby
            for (int j=0; j<refinedlines.size() && (!isAtIntersectionAlready0 || !isAtIntersectionAlready1); j++) {
                
                if (j==index) continue;
                
                LineSegment testLine = refinedlines[j];
                
                //double testLineLength = Geometry::euclideanDistance(nativeLine[0], nativeLine[1]);
                LineSegment testLineExtended = extendLine(testLine, 0.3);
                
                cv::Point2f intersection;
                
                
                //check if already at an intersection
                bool intersect = Geometry::segmentsIntersect(nativeLine[0], nativeLine[1],
                                                             testLine[0], testLine[1], intersection);
                if (intersect) {
                    double distanceTo0 = Geometry::euclideanDistance(nativeLine[0], intersection);
                    double distanceTo1 = Geometry::euclideanDistance(nativeLine[1], intersection);
                    
                    if (distanceTo0 < distanceTo1) {
                        if (distanceTo0 < MAX(lineLength / 15, 5)) {
                            isAtIntersectionAlready0 = true;
                            refinedlines[index][0] = intersection;
                        }
                    } else {
                        if (distanceTo1 < MAX(lineLength / 15, 5)) {
                            isAtIntersectionAlready1 = true;
                            refinedlines[index][1] = intersection;
                        }
                    }
                }
                
                intersect = Geometry::segmentsIntersect(nativeLineExtended[0], nativeLineExtended[1],
                                                        testLineExtended[0], testLineExtended[1], intersection);
                
                
                if (intersect) {
                    
                    double angleA = Geometry::angleOfPoints(testLineExtended[0], testLineExtended[1]);
                    double angleB = Geometry::angleOfPoints(nativeLineExtended[0], nativeLineExtended[1]);
                    
                    double angleDiff = Geometry::lineAngleDifference(angleA, angleB);
                    double distanceFrom90 = abs(angleDiff - 90);
                    
                    bool isPerpendicular = distanceFrom90 < 20;
                    //isPerpendicular = false;
                    bool isParallel = angleDiff < 10;
                    
                    double distanceTo0 = Geometry::euclideanDistance(nativeLine[0], intersection);
                    double distanceTo1 = Geometry::euclideanDistance(nativeLine[1], intersection);
                    double testLineMinDistance = MIN(Geometry::euclideanDistance(testLine[0], intersection),
                                                     Geometry::euclideanDistance(testLine[1], intersection));
                    
                    
                    
                    if (distanceTo0 < distanceTo1) {
                        double extensionDistance = MIN(testLineMinDistance, distanceTo0);
                        //intersects at 0
                        //double distanceAfter = Geometry::euclideanDistance(refinedlines[i][1], intersection);
                        if ((extensionDistance < MIN(bestTestDistanceToInt0, bestLineDistanceToInt0)) && distanceTo1 > lineLength) {
                            bestTestDistanceToInt0 = testLineMinDistance;
                            bestLineDistanceToInt0 = distanceTo0;
                            bestIntersection0 = intersection;
                            bestJ0 = j;
                            bestIsPerpendicular0 = isPerpendicular;
                            bestIsParallel0 = isParallel;
                        }
                    } else {
                        double extensionDistance = MIN(testLineMinDistance, distanceTo1);
                        //intersects at 1
                        //double distanceAfter = Geometry::euclideanDistance(refinedlines[i][0], intersection);
                        if ((extensionDistance < MIN(bestTestDistanceToInt1, bestLineDistanceToInt1)) && distanceTo0 > lineLength) {
                            bestTestDistanceToInt1 = testLineMinDistance;
                            bestLineDistanceToInt1 = distanceTo1;
                            bestIntersection1 = intersection;
                            bestJ1 = j;
                            bestIsPerpendicular1 = isPerpendicular;
                            bestIsParallel1 = isParallel;
                        }
                    }
                }
            }
            
            if (!isAtIntersectionAlready0 && bestJ0 >= 0) {
                
                if (((bestIsPerpendicular0 || bestIsParallel0) && (bestLineDistanceToInt0 < lineLength * 0.1) && (bestTestDistanceToInt0 < lineLength * 0.1))
                    || isValidLineExtension(srcImage, refinedlines[index], bestIntersection0, 0, params)) {
                    refinedlines[index][0] = bestIntersection0;
                    double jLineLength = Geometry::euclideanDistance(refinedlines[bestJ0][0], refinedlines[bestJ0][1]);
                    double distanceTo0 = Geometry::euclideanDistance(refinedlines[bestJ0][0], bestIntersection0);
                    double distanceTo1 = Geometry::euclideanDistance(refinedlines[bestJ0][1], bestIntersection0);
                    
                    if (distanceTo0 < distanceTo1 && distanceTo1 > jLineLength) {
                        if (isValidLineExtension(srcImage, refinedlines[bestJ0], bestIntersection0, 0, params)) {
                            refinedlines[bestJ0][0] = bestIntersection0;
                        }
                    } else if (distanceTo1 < distanceTo0 && distanceTo0 > jLineLength) {
                        if (isValidLineExtension(srcImage, refinedlines[bestJ0], bestIntersection0, 1, params)) {
                            refinedlines[bestJ0][1] = bestIntersection0;
                        }
                    }
                    
                }
            }
            
            if (!isAtIntersectionAlready1 && bestJ1 >= 0) {
                
                if (((bestIsPerpendicular1 || bestIsParallel1) && (bestLineDistanceToInt1 < lineLength * 0.1) && (bestTestDistanceToInt1 < lineLength * 0.1))
                    || isValidLineExtension(srcImage, refinedlines[index], bestIntersection1, 1, params)) {
                    refinedlines[index][1] = bestIntersection1;
                    
                    double jLineLength = Geometry::euclideanDistance(refinedlines[bestJ1][0], refinedlines[bestJ1][1]);
                    double distanceTo0 = Geometry::euclideanDistance(refinedlines[bestJ1][0], bestIntersection1);
                    double distanceTo1 = Geometry::euclideanDistance(refinedlines[bestJ1][1], bestIntersection1);
                    
                    if (distanceTo0 < distanceTo1 && distanceTo1 > jLineLength) {
                        if (isValidLineExtension(srcImage, refinedlines[bestJ1], bestIntersection1, 0, params)) {
                            refinedlines[bestJ1][0] = bestIntersection1;
                        }
                    } else if (distanceTo1 < distanceTo0 && distanceTo0 > jLineLength) {
                        if (isValidLineExtension(srcImage, refinedlines[bestJ1], bestIntersection1, 1, params)) {
                            refinedlines[bestJ1][1] = bestIntersection1;
                        }
                    }
                }
            }
            
            for (int k=0; k<borders.size(); k++) {
                cv::Point2f intersection;
                LineSegment border = borders[k];
                bool intersect = Geometry::segmentsIntersect(nativeLineExtended[0], nativeLineExtended[1],
                                                             border[0], border[1], intersection);
                
                if (intersect) {
                    double distanceTo0 = Geometry::euclideanDistance(nativeLine[0], intersection);
                    double distanceTo1 = Geometry::euclideanDistance(nativeLine[1], intersection);
                    if (distanceTo0 < distanceTo1) {
                        if (distanceTo0 < 10 || isValidLineExtension(srcImage, nativeLine, intersection, 0, params)) {
                            refinedlines[index][0] = intersection;
                        }
                    } else {
                        if (distanceTo1 < 10 || isValidLineExtension(srcImage, nativeLine, intersection, 1, params)) {
                            refinedlines[index][1] = intersection;
                        }
                    }
                }
            }
            
//            if (bestIsParallel0 && bestIsParallel1) {
//                CBLog("both");
//            } else if (bestIsParallel0) {
//                CBLog("combine 00000");
//            } else if (bestIsParallel1) {
//                CBLog("combine 11111");
//            }
        }
    }
    
    bool LineProcessing::segmentsIntersect(const LineSegment &line1, const LineSegment &line2, cv::Point2f &intersection)
    {
        return Geometry::segmentsIntersect(line1[0], line1[1], line2[0], line2[1], intersection);
    }
    
    void LineProcessing::findPrimarySecondaryLines(const cv::Mat &srcImage,
                                                   const std::vector<LineSegment> &lines,
                                                   std::vector<int> &primaryLines,
                                                   std::vector<int> &secondaryLines,
                                                   double seedAngle,
                                                   const std::vector<int> &toIgnore,
                                                   cv::Rect roi)
    {
        std::vector<int>topmostIndexes;
        //int yThreshold = srcImage.rows / 2;
        
        int lengthThreshold = (srcImage.cols + srcImage.rows) / 30;
        lengthThreshold = 0;
        
        int qtRotations = seedAngle / 90;
        
        if (!roi.width) {
            double sizeCoefficient = 0.6;
            if (qtRotations == 0) {
                //0 degrees, top region
                roi = cv::Rect(0, 0, srcImage.cols, sizeCoefficient * double(srcImage.rows));
            } else if (qtRotations == 1) {
                //90 degrees, left region
                roi = cv::Rect(0, 0, sizeCoefficient * double(srcImage.cols), srcImage.rows);
            } else if (qtRotations == 2) {
                //180 degrees, bottom region
                roi = cv::Rect(0, sizeCoefficient * double(srcImage.rows), srcImage.cols, sizeCoefficient * double(srcImage.rows));
            } else {
                //270 degrees, right region
                roi = cv::Rect(sizeCoefficient * double(srcImage.cols), 0, sizeCoefficient * double(srcImage.cols), srcImage.rows);
            }
        }
        
        for (int i=0; i<lines.size(); i++) {
            
            if (isIndexed(i, toIgnore)) continue;
            
            LineSegment nativeLine = lines[i];
            
            bool touchesBorder = false;
            if (qtRotations == 0) {
                touchesBorder = nativeLine[0].y <= 5 || nativeLine[1].y <= 5;
            } else if (qtRotations == 1) {
                touchesBorder = nativeLine[0].x <= 5 || nativeLine[1].x <= 5;
            } else if (qtRotations == 2) {
                touchesBorder = nativeLine[0].y >= srcImage.rows - 5 || nativeLine[1].y >= srcImage.rows - 5;
            } else {
                touchesBorder = nativeLine[0].x >= srcImage.cols - 5 || nativeLine[1].x >= srcImage.cols - 5;
            }
            
            if (!nativeLine[0].inside(roi) && !nativeLine[1].inside(roi)) {
                continue;
            }
            
            double angle = Geometry::angleOfPoints(nativeLine[0], nativeLine[1]);
            
            angle = Geometry::lineAngleDifference(angle, seedAngle);
            
            bool isHorizontal = angle < 60;
            
            
            bool topTouchingLine = touchesBorder && angle < 80;
            
            if (topTouchingLine) {
                LOG_LINE("Border line", nativeLine);
            }
            
            if (!isHorizontal && !topTouchingLine) continue;
            
            double length = Geometry::euclideanDistance(nativeLine[0], nativeLine[1]);
            
            if (length > lengthThreshold || topTouchingLine) {
                topmostIndexes.push_back(i);
            } else if (length < lengthThreshold / 2) {
                break;
            }
        }
        
        //return topmostIndexes;
        std::vector<int>flaggedInvalid;
        //iterate back over picks, eliminating lower lines
        for (int i=0; i<topmostIndexes.size(); i++) {
            int index = topmostIndexes[i];
            LineSegment nativeLine = lines[index];
            
            double lineLength = Geometry::euclideanDistance(nativeLine[0], nativeLine[1]);
            double lineAngle = Geometry::angleOfPoints(nativeLine[0], nativeLine[1]);
            cv::Point startPoint = Geometry::midpoint(nativeLine[0], nativeLine[1]);
            
            cv::Point outLine1, outLine2;
            Geometry::getPerpendicularPointsForLine(nativeLine[0], nativeLine[1], 500, outLine1, outLine2);
            
            cv::Point topPoint;
            cv::Point bottomPoint;
            
            if (qtRotations == 0) {
                topPoint = outLine1.y < outLine2.y ? outLine1 : outLine2;
                bottomPoint = outLine1.y < outLine2.y ? outLine2 : outLine1;
            } else if (qtRotations == 1) {
                topPoint = outLine1.x < outLine2.x ? outLine1 : outLine2;
                bottomPoint = outLine1.x < outLine2.x ? outLine2 : outLine1;
            } else if (qtRotations == 2) {
                topPoint = outLine1.y > outLine2.y ? outLine1 : outLine2;
                bottomPoint = outLine1.y > outLine2.y ? outLine2 : outLine1;
            } else {
                topPoint = outLine1.x > outLine2.x ? outLine1 : outLine2;
                bottomPoint = outLine1.x > outLine2.x ? outLine2 : outLine1;
            }
            
            
            //int midpointY = Geometry::midpoint(nativeLine[0], nativeLine[1]).y;
            
            bool isValid = !isIndexed(index, flaggedInvalid);
            
            for (int j=0; isValid && j<topmostIndexes.size(); j++) {
                if (j == i) continue;
                int jindex = topmostIndexes[j];
                
                if (isIndexed(jindex, flaggedInvalid)) {
                    continue;
                }
                
                LineSegment testLine = lines[jindex];
                //int testMidpointY = Geometry::midpoint(testLine[0], testLine[1]).y;
                
                double testLineLength = Geometry::euclideanDistance(testLine[0], testLine[1]);
                double testLineAngle = Geometry::angleOfPoints(testLine[0], testLine[1]);
                double angleBetween = Geometry::lineAngleDifference(lineAngle, testLineAngle);
                
                double distanceBetween = Geometry::distanceBetweenSegments(testLine[0], testLine[1], nativeLine[0], nativeLine[1]);
                
                bool closeBy = distanceBetween < MAX(lineLength, testLineLength) / 12;
                bool fairlyCloseBy = distanceBetween < MAX(lineLength, testLineLength) / 5;
                bool parallel = angleBetween < MAX(10, MIN(100 / sqrt(lineLength), 45));
                double lengthSimilarity = abs(lineLength - testLineLength) / (testLineLength + lineLength);
                //bool similarLength = lengthSimilarity < 0.2;
                
                cv::Point2f intersection;
                
                bool linesIntersect = Geometry::segmentsIntersect(testLine[0], testLine[1], nativeLine[0], nativeLine[1], intersection);
                bool intersectsAbove = Geometry::segmentsIntersect(testLine[0], testLine[1], startPoint, topPoint, intersection);
                bool intersectsBelow = Geometry::segmentsIntersect(testLine[0], testLine[1], bottomPoint, topPoint, intersection);
                
                if (intersectsAbove || intersectsBelow) {
                    //test intersection above
                    if ((!parallel && closeBy) || (linesIntersect && angleBetween > 30)) {
                        if (lineLength < testLineLength) {
                            isValid = false;
                        } else {
                            flaggedInvalid.push_back(jindex);
                        }
                    } else if (lengthSimilarity > 0.5 && lineLength < testLineLength && fairlyCloseBy) {
                        isValid = false;
                    }
                }
            }
            
            if (isValid) {
                if (hasSimilarLine(lines, primaryLines, index)) {
                    secondaryLines.push_back(topmostIndexes[i]);
                } else {
                    primaryLines.push_back(topmostIndexes[i]);
                }
            }
        }
    }
    
    
    bool sortVerticeAreaCompare(const vertice &vert1, const vertice &vert2) { return vert1.area > vert2.area; }
    
    std::vector<vertice> LineProcessing::findVerticesForLines(const std::vector<LineSegment> &lines, int count) {
        
        std::vector<vertice> vertices;
        
        for (int i=0; i<lines.size() && i<count; i++) {
            LineSegment nativeLine = lines[i];
            //double lineAngle = Geometry::angleOfPoints(nativeLine[0], nativeLine[1]);
            //double lineDistance = Geometry::euclideanDistance(nativeLine[0], nativeLine[1]);
            
            
            LineSegment nativeLineExtended = extendLine(nativeLine, 0.1);
            
            for (int j=i+1; j<lines.size(); j++) {
                LineSegment testLine = lines[j];
                //double testLineAngle = Geometry::angleOfPoints(testLine[0], testLine[1]);
                //double degreeDifference = Geometry::lineAngleDifference(testLineAngle, lineAngle);
                //double testLineDistance = Geometry::euclideanDistance(testLine[0], testLine[1]);
                
                LineSegment testLineExtended = extendLine(testLine, 0.1);
                
                cv::Point2f testVertex;
                bool intersect = Geometry::segmentsIntersect(nativeLineExtended[0], nativeLineExtended[1],
                                                             testLineExtended[0], testLineExtended[1], testVertex);
                
                if (intersect) {
                    double distanceToOrigin0 = Geometry::euclideanDistance(nativeLine[0], testVertex);
                    double distanceToOrigin1 = Geometry::euclideanDistance(nativeLine[1], testVertex);
                    //double distanceToOrigin = distanceToOrigin0 < distanceToOrigin1 ? distanceToOrigin0 : distanceToOrigin1;
                    
                    
                    vertice vertice;
                    vertice.pointA = distanceToOrigin0 > distanceToOrigin1 ? nativeLine[0] : nativeLine[1];
                    
                    double distanceToTest0 = Geometry::euclideanDistance(testLine[0], testVertex);
                    double distanceToTest1 = Geometry::euclideanDistance(testLine[1], testVertex);
                    
                    vertice.pointB = distanceToTest0 > distanceToTest1 ? testLine[0] : testLine[1];
                    vertice.vertex = testVertex;
                    
                    
                    //heron's formula for area
                    //s = (a+b+c)/2
                    double a = distanceToOrigin0 > distanceToOrigin1 ? distanceToOrigin0 : distanceToOrigin1;
                    double b = distanceToTest0 > distanceToTest1 ? distanceToTest0 : distanceToTest1;
                    double c = Geometry::euclideanDistance(vertice.pointA, vertice.pointB);
                    double s = (a + b + c) / 2.0f;
                    
                    //Area = √( s(s-a)(s-b)(s-c) )
                    vertice.area = sqrt(s*(s-a) * (s-b) * (s-c));
                    
                    vertices.push_back(vertice);
                }
            }
        }
        
        sort(vertices.begin(), vertices.end(), sortVerticeAreaCompare);
        
        return vertices;
    }
    
    std::vector<std::vector<vertice> > LineProcessing::linkVertices(const std::vector<vertice> &vertices) {
        std::vector<std::vector<vertice> >linkedVertices;
        
        
        std::vector<int>indexesCombined;
        
        for (int i=0; i<vertices.size(); i++) {
            
            if (isIndexed(i, indexesCombined)) continue;
            
            vertice currentVertice = vertices[i];
            
            std::vector<vertice> verticeSet;
            verticeSet.push_back(currentVertice);
            
            for (int j=i+1; j<vertices.size(); j++) {
                vertice testVertice = vertices[j];
                
                if (currentVertice.pointA == testVertice.pointA
                    || currentVertice.pointA == testVertice.pointB
                    || currentVertice.pointA == testVertice.vertex
                    || currentVertice.pointB == testVertice.pointA
                    || currentVertice.pointB == testVertice.pointB
                    || currentVertice.pointB == testVertice.vertex
                    || currentVertice.vertex == testVertice.pointA
                    || currentVertice.vertex == testVertice.pointB
                    || currentVertice.vertex == testVertice.vertex) {
                    
                    indexesCombined.push_back(j);
                    verticeSet.push_back(testVertice);
                }
            }
            
            linkedVertices.push_back(verticeSet);
        }
        
        return linkedVertices;
    }
    
    bool LineProcessing::isValidLineExtension(const cv::Mat &srcImage,
                                               const LineSegment &nativeLine, const cv::Point &proposedEndpoint,
                                               int fromIndex, const line_match_params &params) {
        
        LineSegment testLine = LineSegment(nativeLine[fromIndex], proposedEndpoint);
        
        return linesMatchStatistically(srcImage, nativeLine, testLine, params);
    }
    
    /*
     struct line_match_params {
     double sameStdThreshold = 5;
     double sameMeanThreshold = 40;
     
     double meanCandidateThreshold = 20;
     double stdCandidateThreshold = 3;
     
     double extensionMeanThreshold = 80;
     double extensionStdThreshold = 0.8;
     };
     */
    
    bool LineProcessing::linesMatchStatistically(const cv::Mat &srcImage,
                                                  const LineSegment &nativeLine,
                                                  const LineSegment &testLine,
                                                  const line_match_params &paramsIn) {
        line_match_params params = paramsIn;
        //params.sameStdThreshold = 3;
        //params.extensionMeanThreshold = 70;
        
        cv::Scalar nativeSampleMean, nativeSampleStdev;
        lineMeanStdDev(srcImage, nativeLine, nativeSampleMean, nativeSampleStdev);
        double nativeStdDev = MAX(nativeSampleStdev[0], MAX(nativeSampleStdev[1], nativeSampleStdev[2]));
        
        cv::Scalar testSampleMean, testSampleStdev;
        lineMeanStdDev(srcImage, testLine, testSampleMean, testSampleStdev);
        double testStdDev = MAX(testSampleStdev[0], MAX(testSampleStdev[1], testSampleStdev[2]));
        
        //double mean = MAX(sampleMean[0], MAX(sampleMean[1], sampleMean[2]));
        double meanDifference = Coloring::euclideanDistance(nativeSampleMean, testSampleMean);
        double stdDifference = fabs(nativeStdDev - testStdDev);
        
        //CBLog("mean stddev: %f,%f", mean, stddev);
        //return true;
        
        bool sameStd = testStdDev < params.sameStdThreshold || (stdDifference < params.sameStdThreshold && testStdDev < 2 * params.sameStdThreshold);
        bool sameMean = meanDifference < params.sameMeanThreshold;
        
        bool extendsEquivalently = sameStd && sameMean;
        
        if (extendsEquivalently) {
            
            LineSegment nativeLineA, nativeLineB;
            getParallelLines(nativeLine, 5, nativeLineA, nativeLineB);
            
            cv::Scalar nativeSampleMeanA, nativeSampleStdevA;
            lineMeanStdDev(srcImage, nativeLineA, nativeSampleMeanA, nativeSampleStdevA);
            double nativeStdDevA = MAX(nativeSampleStdevA[0], MAX(nativeSampleStdevA[1], nativeSampleStdevA[2]));
            
            cv::Scalar nativeSampleMeanB, nativeSampleStdevB;
            lineMeanStdDev(srcImage, nativeLineB, nativeSampleMeanB, nativeSampleStdevB);
            double nativeStdDevB = MAX(nativeSampleStdevB[0], MAX(nativeSampleStdevB[1], nativeSampleStdevB[2]));
            
            double meanDifferenceAB = Coloring::euclideanDistance(nativeSampleMeanA, nativeSampleMeanB);
            double stdDifferenceAB = fabs(nativeStdDevA - nativeStdDevB);
            
            if (meanDifferenceAB > params.meanCandidateThreshold || stdDifferenceAB > params.stdCandidateThreshold) {
                LineSegment testLineA, testLineB;
                getParallelLines(testLine, 3, testLineA, testLineB);
                
                cv::Scalar testSampleMeanA, testSampleStdevA;
                lineMeanStdDev(srcImage, testLineA, testSampleMeanA, testSampleStdevA);
                //double testStdDevA = MAX(testSampleStdevA[0], MAX(testSampleStdevA[1], testSampleStdevA[2]));
                
                cv::Scalar testSampleMeanB, testSampleStdevB;
                lineMeanStdDev(srcImage, testLineB, testSampleMeanB, testSampleStdevB);
                //double testStdDevB = MAX(testSampleStdevB[0], MAX(testSampleStdevB[1], testSampleStdevB[2]));
                
                double meanDifferenceAA = Coloring::euclideanDistance(nativeSampleMeanA, testSampleMeanA);
                double meanDifferenceBB = Coloring::euclideanDistance(nativeSampleMeanB, testSampleMeanB);
                
                double stdDifferenceAA = Coloring::euclideanDistance(nativeSampleStdevA, testSampleStdevA);
                double stdDifferenceBB = Coloring::euclideanDistance(nativeSampleStdevB, testSampleStdevB);
                
                bool sameStdParallel = stdDifferenceAA < params.extensionStdThreshold && stdDifferenceBB < params.extensionStdThreshold;
                bool sameMeanParallel = meanDifferenceAA < params.extensionMeanThreshold && meanDifferenceBB < params.extensionMeanThreshold;
                
                return sameStdParallel && sameMeanParallel;
            }
        }
        
        return false;
    }
    
    void LineProcessing::getParallelLines(const LineSegment &nativeLine, int atDistance,
                                           LineSegment &lineA, LineSegment &lineB) {
        cv::Point line11, line12, line21, line22;
        Geometry::getParallelLines(nativeLine[0], nativeLine[1], atDistance, line11, line12, line21, line22);
        
        lineA = LineSegment(line11, line12);
        lineB = LineSegment(line21, line22);
    }
    
    void LineProcessing::lineMeanStdDev(const cv::Mat &srcImage, const LineSegment &nativeLine,
                                         cv::Scalar &sampleMean, cv::Scalar &sampleStdev) {
        std::vector<cv::Vec3b>samples;
        
        std::vector<cv::Point>points;
        Geometry::getPointsOnSegment(nativeLine[0], nativeLine[1], points);
        
        Imaging::meanStdDevForPoints(srcImage, points, sampleMean, sampleStdev);
    }

    void LineProcessing::parallelLinesMeanStdDev(const cv::Mat &srcImage, const LineSegment &nativeLine,
                                                  cv::Scalar &sampleMeanA, cv::Scalar &sampleStdDevA,
                                                  cv::Scalar &sampleMeanB, cv::Scalar &sampleStdDevB,
                                                  int startDistance, int endDistance) {
        
        std::vector<cv::Point>aPoints;
        std::vector<cv::Point>bPoints;
        
        for (int i=startDistance; i<endDistance; i++) {
            LineSegment testLineA, testLineB;
            getParallelLines(nativeLine, i, testLineA, testLineB);
            
            std::vector<cv::Point>newAPoints;
            Geometry::getPointsOnSegment(testLineA[0], testLineA[1], newAPoints);
            aPoints.insert(aPoints.end(), newAPoints.begin(), newAPoints.end());
            
            std::vector<cv::Point>newBPoints;
            Geometry::getPointsOnSegment(testLineB[0], testLineB[1], newBPoints);
            bPoints.insert(bPoints.end(), newBPoints.begin(), newBPoints.end());
        }
        
        Imaging::meanStdDevForPoints(srcImage, aPoints, sampleMeanA, sampleStdDevA);
        Imaging::meanStdDevForPoints(srcImage, bPoints, sampleMeanB, sampleStdDevB);
    }
    
    std::vector<LineSegment> LineProcessing::combineCollinearLines(const cv::Mat &srcImage,
                                                                     const std::vector<LineSegment> &lines,
                                                                     double minDegreeThreshold,
                                                                     double maxDegreeThreshold,
                                                                     double degreeFactor,
                                                                     double minDistanceThreshold,
                                                                     double maxDistanceThreshold)
    {
        std::vector<LineSegment> filteredLines;
        
        //vector<LineSegment>::iterator it;
        std::vector<int>indexesCombined;
        
        for (int i=0; i<lines.size(); i++) {
            if (isIndexed(i, indexesCombined)) continue;
            
            LineSegment line = lines[i];
            LineSegment longerLine = extendLine(line, 0.3);
            double lineAngle = Geometry::angleOfPoints(line[0], line[1]);
            double lineDistance = Geometry::euclideanDistance(line[0], line[1]);
            
            bool combined = false;
            std::vector<int> indexesToJoin;
            
            std::vector<cv::Point>pointsToJoin;
            for (int j=i+1; j<lines.size(); j++) {
                //if (isIndexed(i, indexesCombined)) continue;
                
                LineSegment testLine = lines[j];
                
                double testLineAngle = Geometry::angleOfPoints(testLine[0], testLine[1]);
                double degreeDifference = Geometry::lineAngleDifference(testLineAngle, lineAngle);
                
                double testLineDistance = Geometry::euclideanDistance(testLine[0], testLine[1]);
                
                double degreeThreshold = MIN(maxDegreeThreshold, MAX(powf(abs(lineDistance - testLineDistance), degreeFactor), minDegreeThreshold));
                //degreeThreshold = 3;
                
                cv::Point2f intersection;
                bool segmentsIntersect = Geometry::segmentsIntersect(line[0], line[1], testLine[0], testLine[1], intersection);
                
                //double intersectionDistance = MIN(Geometry::euclideanDistance(intersection, line[0]), Geometry::euclideanDistance(intersection, line[1]));
                
                if (degreeDifference < degreeThreshold) {
                    //test to see how close
                    LineSegment longerTestLine = extendLine(testLine, 0.3);
                    
                    //double testLineDistance = Geometry::euclideanDistance(testLine[0], testLine[1]);
                    double distanceThreshold = MIN(maxDistanceThreshold, MAX(degreeDifference, minDistanceThreshold));
                    
                    //todo: algorithm
                    //if (lineDistance > 200 && testLineDistance > 200) distanceThreshold = 7;
                    //else if (lineDistance > 100 && testLineDistance > 100) distanceThreshold = 5;
                    
                    double closestDistance = Geometry::distanceBetweenSegments(longerLine[0], longerLine[1], longerTestLine[0], longerTestLine[1]);
                    //double closestDistance = Geometry::distanceBetweenSegments(line[0], line[1], testLine[0], testLine[1]);
                    if (closestDistance < distanceThreshold || segmentsIntersect) {
                        indexesToJoin.push_back(j);
                        pointsToJoin.push_back(testLine[0]);
                        pointsToJoin.push_back(testLine[1]);
                        combined = true;
                    }
                }
            }
            
            if (combined) {
                pointsToJoin.push_back(line[0]);
                pointsToJoin.push_back(line[1]);
                LineSegment combinedSegment = createLineSegment(pointsToJoin);
                
                cv::Scalar nativeSampleMean, nativeSampleStdev;
                lineMeanStdDev(srcImage, line, nativeSampleMean, nativeSampleStdev);
                
                cv::Scalar combinedSampleMean, combinedSampleStdev;
                lineMeanStdDev(srcImage, combinedSegment, combinedSampleMean, combinedSampleStdev);
                
                double stddevDifference = Coloring::euclideanDistance(nativeSampleStdev, combinedSampleStdev);
                double meanColorDifference = Coloring::euclideanDistance(nativeSampleMean, combinedSampleMean);
                
                if (stddevDifference < 10
                    || meanColorDifference < 50) {
                    filteredLines.push_back(combinedSegment);
                    indexesCombined.insert( indexesCombined.end(), indexesToJoin.begin(), indexesToJoin.end());
                } else {
                    filteredLines.push_back(line);
                }
                
                //CBLog("Found %d angles matching", indexesToCombine.size());
            } else {
                filteredLines.push_back(line);
            }
        }
        
        //CBLog("combined a total of %d points", indexesCombined.size());
        return filteredLines;
    }
    
    std::vector<LineSegment> LineProcessing::linesForPolygons(const std::vector<std::vector <cv::Point> > &polygons) {
        std::vector<LineSegment> lines;
        
        for (int i=0; i<polygons.size(); i++) {
            std::vector<cv::Point> polygon = polygons[i];
            
            size_t numPoints = polygon.size();
            if (numPoints == 2) {
                lines.push_back(LineSegment(polygon[0], polygon[1]));
            } else if (numPoints > 2) {
                for (int j=1; j<numPoints; j++) {
                    LineSegment segment(polygon[j-1], polygon[j]);
                    lines.push_back(segment);
                }
            }
            //lines.push_back(polygon[i]
        }
        
        return lines;
    }
    
    //    def segments_intersect((x11, y11, x12, y12, x21, y21, x22, y22):
    //                           """ whether two segments in the plane intersect:
    //                           one segment is (x11, y11) to (x12, y12)
    //                           the other is   (x21, y21) to (x22, y22)
    //                           """
    //                           dx1 = x12 - x11
    //                           dy1 = y12 - y11
    //                           dx2 = x22 - x21
    //                           dy2 = y22 - y21
    //                           delta = dx2 * dy1 - dy2 * dx1
    //                           if delta == 0: return False  # parallel segments
    //                           s = (dx1 * (y21 - y11) + dy1 * (x11 - x21)) / delta
    //                           t = (dx2 * (y11 - y21) + dy2 * (x21 - x11)) / (-delta)
    //                           return (0 <= s <= 1) and (0 <= t <= 1)
    
    
    std::vector<LineSegment> LineProcessing::extendLines(const std::vector<LineSegment> &lines, double percentage, bool extensionsOnly)
    {
        std::vector<LineSegment> extendedlines;
        for (int i=0; i<lines.size(); i++) {
            LineSegment line = lines[i];
            LineSegment newSegment = extendLine(line, percentage);
            
            if (extensionsOnly) {
                LineSegment newSegmentA, newSegmentB;
                
                double distance1 = Geometry::euclideanDistance(line[0], newSegment[0]);
                double distance2 = Geometry::euclideanDistance(line[0], newSegment[1]);
                
                if (distance1 < distance2) {
                    newSegmentA = LineSegment(newSegment[0], line[0]);
                    newSegmentB = LineSegment(line[1], newSegment[1]);

                } else {
                    newSegmentA = LineSegment(newSegment[1], line[0]);
                    newSegmentB = LineSegment(line[1], newSegment[0]);
                }
                
                extendedlines.push_back(newSegmentA);
                extendedlines.push_back(newSegmentB);
            } else {
                extendedlines.push_back(newSegment);
            }
        }
        return extendedlines;
    }
    
    //    std::vector<cv::Point> ImageProcessing::extendPolygon(const std::vector<cv::Point> polygon, double percentage) {
    //        const std::vector<cv::Point> extended;
    //
    //        return extended;
    //    }
    
    void LineProcessing::joinCloseParallelLines(std::vector<imaging::LineSegment> &lines, std::vector<int> &lineIndexes,
                                                float parallelAngleTolerance, float parallelDistanceTolerance, int useAngle)
    {
        std::vector<imaging::LineSegment> newLines = joinCloseParallelLines(lines, lineIndexes, lines, lineIndexes,
                                                                             parallelAngleTolerance, parallelDistanceTolerance, useAngle);
        for (imaging::LineSegment segment : newLines) {
            lineIndexes.push_back(int(lines.size()));
            lines.push_back(segment);
        }
    }
    
    std::vector<imaging::LineSegment>
    LineProcessing::joinCloseParallelLines(std::vector<imaging::LineSegment> &linesA, const std::vector<int> &lineAIndexes,
                                           std::vector<imaging::LineSegment> &linesB, const std::vector<int> &lineBIndexes,
                                           float parallelAngleTolerance, float parallelDistanceTolerance, int useAngle) {
        
        std::vector<imaging::LineSegment> newLines;
        
        for (int i=0; i<lineAIndexes.size(); i++) {
            int v = lineAIndexes[i];
            if (linesA[v].invalid || (linesA[v].point0Terminated && linesA[v].point1Terminated)) continue;
            
            LineSegment extendedLine1 = LineProcessing::extendLine(linesA[v], 0.4f);
            
            //check vertical lines against vertical, join
            for (int j=0; j<lineBIndexes.size(); j++) {
                if (j == i) continue;
                int v2 = lineBIndexes[j];
                if (linesB[v2].invalid || (linesB[v2].point0Terminated && linesB[v2].point1Terminated)) continue;
                
                bool closestVIs0 = Geometry::euclideanDistanceSq(linesB[v2].midpoint, linesA[v][0])
                < Geometry::euclideanDistanceSq(linesB[v2].midpoint, linesA[v][1]);
                bool closestV2Is0 = Geometry::euclideanDistanceSq(linesA[v].midpoint, linesB[v2][0])
                < Geometry::euclideanDistanceSq(linesA[v].midpoint, linesB[v2][1]);
                
                if ((linesA[v].point0Terminated && closestVIs0) || (linesA[v].point1Terminated && !closestVIs0)
                    || (linesB[v2].point0Terminated && closestV2Is0) || (linesB[v2].point1Terminated && !closestV2Is0)) {
                    continue;
                }
                
                LineSegment extendedLine2 = LineProcessing::extendLine(linesB[v2], 0.4f);
                
                float distance = Geometry::distanceBetweenSegments(extendedLine1[0], extendedLine1[1], extendedLine2[0], extendedLine2[1]);
                float angleDiff = Geometry::lineAngleDifference(extendedLine1.angle, extendedLine2.angle);
                if (distance <= parallelDistanceTolerance && fabs(angleDiff) < parallelAngleTolerance) {
                    
                    cv::Point2f pointA = closestVIs0 ? linesA[v][1] : linesA[v][0];
                    cv::Point2f pointB = closestV2Is0 ? linesB[v2][1] : linesB[v2][0];
                    cv::Point2f midpoint = Geometry::midpoint(pointA, pointB);
                    
                    float newAngle = Geometry::angleOfPoints(pointA, pointB);
                    
                    float compareAngle;
                    if (useAngle == 1) {
                        compareAngle = extendedLine1.angle;
                    } else if (useAngle == 2) {
                        compareAngle = extendedLine2.angle;
                    } else {
                        //Average
                        compareAngle = extendedLine1.angle - angleDiff / 2.0f;
                    }
                    
                    float rotationCorrection = Geometry::lineAngleDifference(compareAngle, newAngle);
                    
                    pointA = Geometry::rotatePoint(pointA, midpoint, rotationCorrection);
                    pointB = Geometry::rotatePoint(pointB, midpoint, rotationCorrection);
                    
                    LineSegment newLine = LineSegment(pointA, pointB);
                    newLine.confirmations = 1 + linesA[v].confirmations + linesB[v2].confirmations;
                    newLine.age = min(linesA[v].age, linesB[v2].age);
                    newLine.userIndex = MAX(linesA[v].userIndex, linesB[v2].userIndex);
                    newLines.push_back(newLine);
                    
                    linesA[v].invalid = true;
                    linesB[v2].invalid = true;
                    break;
                }
            }
        }
        
        return newLines;
    }
    
    void LineProcessing::extendLineSegment(LineSegment &line, double amount)
    {
        Geometry::scaleLineSegment(line.point0, line.point0, amount);
    }
    
    LineSegment LineProcessing::extendLine(const LineSegment &line, double percentage, double length, int fromIndex)
    {
        double distance = Geometry::euclideanDistance(line[0], line[1]);
        
        cv::Point2f startPoint;
        cv::Point2f endPoint;
        if (!length) {
            length = percentage * distance;
        }
        
        double length0 = fromIndex == -1 || fromIndex == 0 ? length : 0;
        double length1 = fromIndex == -1 || fromIndex == 1 ? length : 0;
        
        if (line[0].x == line[1].x) {
            startPoint.x =  endPoint.x = line[0].x;
            if (line[0].y < line[1].y) {
                startPoint.y = line[0].y - length0;
                endPoint.y = line[1].y + length1;
            } else {
                startPoint.y = line[1].y - length1;
                endPoint.y = line[0].y + length0;
            }
        } else if (line[0].y == line[1].y) {
            startPoint.y =  endPoint.y = line[0].y;
            if (line[0].x < line[1].x) {
                startPoint.x = line[0].x - length0;
                endPoint.x = line[1].x + length1;
            } else {
                startPoint.x = line[1].x - length1;
                endPoint.x = line[0].x + length0;
            }
        }
        else {
            //y=mx+C
            
            double dx = line[1].x - line[0].x;
            double dy = line[1].y - line[0].y;
            
            startPoint.x = line[0].x - dx * (double(length0) / distance);
            startPoint.y = line[0].y - dy * (double(length0) / distance);
            endPoint.x = line[1].x + dx * (double(length1) / distance);
            endPoint.y = line[1].y + dy * (double(length1) / distance);
        }
        
        LineSegment newSegment = LineSegment(startPoint, endPoint);
        
        return newSegment;
    }
    
    cv::Point LineProcessing::pointOnLine(const LineSegment &line, int amount)
    {
        cv::Point resultPoint;
        
        if (line[0].x == line[1].x) {
            //vertical line
            resultPoint.x = line[0].x;
            if (line[0].y < line[1].y) {
                resultPoint.y = amount > 0 ? line[0].y + amount : line[1].y - amount;
            } else {
                resultPoint.y = amount > 0 ? line[1].y + amount : line[0].y - amount;
            }
        } else if (line[0].y == line[1].y) {
            //horizontal line
            resultPoint.y = line[0].y;
            if (line[0].x < line[1].x) {
                resultPoint.x = amount > 0 ? line[0].x + amount : line[1].x - amount;
            } else {
                resultPoint.x = amount > 0 ? line[1].x + amount : line[0].x - amount;
            }
        }
        else {
            //y=mx+C
            
            double dx = line[1].x - line[0].x;
            double dy = line[1].y - line[0].y;
            //C.x = B.x + (B.x - A.x) / lenAB * length;
            //C.y = B.y + (B.y - A.y) / lenAB * length;
            
            if (line[0].x > line[1].x) {
                resultPoint.x = amount < 0 ? line[0].x : line[1].x;
                resultPoint.y = amount < 0 ? line[0].y : line[1].y;
            }
            else {
                resultPoint.x = amount < 0 ? line[1].x : line[0].x;
                resultPoint.y = amount < 0 ? line[1].y : line[0].y;
            }
            double lineLength = Geometry::euclideanDistance(line[0], line[1]);
            resultPoint.x +=  dx * (double(amount) / lineLength);
            resultPoint.y +=  dy * (double(amount) / lineLength);
        }
        
        return resultPoint;
    }
    
    inline int euclideanLineDistance(LineSegment lineA, LineSegment lineB) {
        int resultDistance = INT_MAX;
        
        int testDistance = Geometry::euclideanDistance(lineA[0], lineB[0]);
        if (Geometry::euclideanDistance(lineA[0], lineB[0]) <= testDistance) {
            resultDistance = testDistance;
        }
        if (Geometry::euclideanDistance(lineA[1], lineB[0]) <= testDistance) {
            resultDistance = testDistance;
        }
        if (Geometry::euclideanDistance(lineA[0], lineB[1]) <= testDistance) {
            resultDistance = testDistance;
        }
        if (Geometry::euclideanDistance(lineA[1], lineB[1]) <= testDistance) {
            resultDistance = testDistance;
        }
        
        return resultDistance;
    }
    
    std::vector<LineSegment> LineProcessing::guessLines(const std::vector<LineSegment> &lines, int range)
    {
        std::vector<LineSegment> lineGuesses;
        
        for (int i=0; i<lines.size(); i++) {
            LineSegment line = lines[i];
            for (int j=0; j<lines.size(); j++) {
                if (j==i) continue;
                LineSegment testLine = lines[j];
                
                if (Geometry::euclideanDistance(line[0], testLine[0]) <= range) {
                    LineSegment guess = LineSegment(line[0], testLine[0]);
                    lineGuesses.push_back(guess);
                }
                if (Geometry::euclideanDistance(line[1], testLine[0]) <= range) {
                    LineSegment guess = LineSegment(line[1], testLine[0]);
                    lineGuesses.push_back(guess);
                }
                
                if (Geometry::euclideanDistance(line[0], testLine[1]) <= range) {
                    LineSegment guess = LineSegment(line[0], testLine[1]);
                    lineGuesses.push_back(guess);
                }
                if (Geometry::euclideanDistance(line[1], testLine[1]) <= range) {
                    LineSegment guess = LineSegment(line[1], testLine[1]);
                    lineGuesses.push_back(guess);
                }
                
            }
        }
        
        return lineGuesses;
    }
    
    bool sortAngleDistances(const angle_distance &ad1, const angle_distance &ad2) { return ad1.distance > ad2.distance; }
    
    int LineProcessing::getAngleDistances(const std::vector<LineSegment> &lines, double deltaAngle, std::vector<angle_distance>&angles)
    {
        int totalLength = 0;
        
        for(unsigned int i=0; i<lines.size(); i++) {
            LineSegment line = lines[i];
            double distance = Geometry::euclideanDistance(line[0], line[1]);
            totalLength += distance;
            double angle = line[0].x < line[1].x ? Geometry::angleOfPoints(line[0], line[1]) : Geometry::angleOfPoints(line[1], line[0]);
            
            bool found = false;
            for (int j=0; j<angles.size() && !found; j++) {
                if (Geometry::lineAngleDifference(angles[j].angle, angle) < deltaAngle) {
                    angles[j].distance += distance;
                    found = true;
                }
            }
            if (!found) {
                angle_distance ad;
                ad.distance = distance;
                ad.angle = angle;
                angles.push_back(ad);
            }
        }
        
        //sort
        sort(angles.begin(), angles.end(), sortAngleDistances);
        
        return totalLength;
    }
    
    void LineProcessing::drawLines(cv::Mat &img, const std::vector<LineSegment> &lines, cv::Scalar color,
                                   int thickness, int confirmationMultiplier)
    {
        bool randomizeColor = color[0] == -1;
        
        for(unsigned int i=0; i<lines.size(); i++) {
            if (randomizeColor) {
                //random
                color = Coloring::randomColor();
            }
            
            cv::line(img, lines[i][0], lines[i][1], color, MIN(5, thickness + confirmationMultiplier * lines[i].confirmations));
            //cv::circle(img, lines[i][0], thickness + 1, color);
            //cv::circle(img, lines[i][1], thickness + 1, color);
        }
    }

    void LineProcessing::getLongLinesInContour(const std::vector<cv::Point> &contour, std::vector<LineSegment>&longLines, cv::Size imageSize, int minLength)
    {
        if (!minLength) {
            minLength = MAX(imageSize.width, imageSize.height) / 15;
        }
        
        for (int i=0; i<contour.size(); i++) {
            cv::Point pointA = contour[i];
            cv::Point pointB = contour[(i+1) % contour.size()];
            
            double length = Geometry::euclideanDistance(pointA, pointB);
            if (length >= minLength
                && !(pointA.x == pointB.x && (pointA.x <= 3 || pointA.x >= imageSize.width - 4))
                && !(pointA.y == pointB.y && (pointA.y <= 3 || pointA.y >= imageSize.height - 4))) {
                longLines.push_back(LineSegment(pointA, pointB));
            }
        }
    }
}
