//
//  CBP_PlaneRansac.cpp
//  Cambrian
//
//  Created by Joel Teply on 3/18/19.
//  Copyright © 2019 Joel Teply. All rights reserved.
//

#include "CBP_PlaneRansac.hpp"

#include <cbar/pipeline/rendering/rendering.h>
#include <cbcommon/CB_Config.h>
#include <imaging/ImageProcessing.h>
#include <imaging/Coloring.h>
#include <cbar/pipeline/util/util.h>
#include <imaging/Imaging.h>

namespace cbpipe {
    
    struct CBP_PlaneRansac::Impl
    {
        Impl(CBP_PlaneRansac *parent, const cv::Size &workingSize, const std::vector<cv::Vec4f>&lines3d,
             std::vector<cv::Mat> &surfaces, std::vector<cv::Point2f> &directions, std::vector<cv::Scalar> &colors) : m_workingSize(workingSize), m_lines(lines3d) {
            
            for (int i=0; i<surfaces.size(); i++) {
                surface_plane plane;
                auto &surface = surfaces[i];
                plane.color = colors[i];
                plane.direction = directions[i];
                plane.workingBounds = _get_surface_contour(surface, workingSize, surface.size().area() / 20);
                
                cv::Vec4f planeLine = cv::Vec4f(0, 0, plane.direction.x, plane.direction.y);
                plane.validLines.reserve(m_lines.size());
                
                //now get valid baselines for this surface
                for (int j=0; j<m_lines.size(); j++) {
                    const auto &line = m_lines[j];
                    float perp = Geometry::linePerpendicularity(line, planeLine);
                    
                    if (perp > 0.3) continue; //30% gradient
                    
                    //check if inside boundary
                    cv::Point2f midpoint = cv::Point2f((line[0] + line[2]) * 0.5, (line[1] + line[3]) * 0.5);
                    int matchingContour = -1;
                    for (int k=0; k<plane.workingBounds.size(); k++) {
                        if (cv::pointPolygonTest(plane.workingBounds[k], midpoint, false) > 0.0f) {
                            matchingContour = k;
                            break;
                        }
                    }
                    if (matchingContour >= 0) {
                        cv::Vec4f extendedLine = Geometry::scaleLineSegment(line, 100.0);
                        auto intersections = CBP_AnalysisUtil::getLineContourIntersections(extendedLine, plane.workingBounds[matchingContour]);
                        
                        if (intersections.size() == 2) {
                            plane.validLines.push_back(cv::Vec4f(intersections[0].x, intersections[0].y, intersections[1].x, intersections[1].y));
                        }
                        
                    }
                }
                if (plane.validLines.size()) {
                    m_planes.push_back(plane);
                }
            }
        }
        
        ~Impl() {
            
        }
        
        struct surface_plane {
            cv::Point2f direction;
            cv::Scalar color;
            std::vector<std::vector<cv::Point>> workingBounds;
            std::vector<cv::Vec4f> validLines;
            cv::Point2f basepointA;
            cv::Point2f basepointB;
        };
        
        std::vector<surface_plane>m_planes;
        std::vector<cv::Vec4f>m_lines;
        cv::Size m_workingSize;
        
        std::vector<std::vector<cv::Point>> _get_surface_contour(cv::Mat &surface, cv::Size size, double minArea) {
            
            cv::erode(surface, surface, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3)));//get rid of clutter
            cv::dilate(surface, surface, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(10, 10)));
            
            std::vector<cv::Vec4i> hierarchy;
            std::vector<std::vector<cv::Point>> planeContours;
            cv::findContours(surface, planeContours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_TC89_KCOS);
            
            cv::Point2f scale = cv::Point2f(float(size.width) / float(surface.cols), float(size.height) / float(surface.rows));
            
            std::vector<std::vector<cv::Point>> boundaries;
            for (int i=0; i<planeContours.size(); i++) {
                const auto &contour = planeContours[i];
                if (contour.size() < 3) continue;
                
                double area = cv::contourArea(contour);
                
                if (area < minArea) continue;
                
                std::vector<cv::Point> hull;
                cv::convexHull(contour, hull);
                
                //expand
                for (auto &point : hull) {
                    point.x *= scale.x;
                    point.y *= scale.y;
                }
                
                boundaries.push_back(hull);
            }
            
            return boundaries;
        }

        void _solve(int numIterations, cv::Mat &debug, std::vector<cv::Vec4f> &basepoints) {
            
            basepoints.clear();
            
            if (!debug.empty()) {
                cv::resize(debug, debug, m_workingSize);
                for (const auto &plane : m_planes) {
                    for (int j=0; j<plane.workingBounds.size(); j++) {
                        cv::drawContours(debug, plane.workingBounds, j, plane.color, 3);
                    }
                    for (const auto &line : plane.validLines) {
                        cv::line(debug, cv::Point2f(line[0], line[1]), cv::Point2f(line[2], line[3]), plane.color, 3);
                    }
                }
            }
            
            // Iterate
            for (const auto &plane : m_planes) {
                float highestScore = 0.0f;
                cv::Vec4f highestLineA, highestLineB;
                cv::RNG rng;
                
                for (int iteration = 0; iteration < numIterations; iteration++) {
                    
                    int usedIndexA = rng.uniform(0, (int)plane.validLines.size());
                    int usedIndexB = rng.uniform(0, (int)plane.validLines.size());
                    if (usedIndexA == usedIndexB) continue;//ignore

                    const cv::Vec4f &lineA = plane.validLines[usedIndexA];
                    const cv::Vec4f &lineB = plane.validLines[usedIndexB];
                    
                    auto score = _compute_score(plane, lineA, lineB);
                    
                    if (score > highestScore) {
                        highestLineA = lineA;
                        highestLineB = lineB;
                        highestScore = score;
                    }
                }
                
                if (highestScore > 0) {
                    basepoints.push_back(highestLineA);
                    basepoints.push_back(highestLineB);
                }
            }
        }
        
        double _compute_score(const surface_plane &plane, const cv::Vec4f &baselineA, const cv::Vec4f &baselineB) {
            
            //auto len = norm(baseeline);
            cv::Point2f baselineAMidpoint = cv::Point2f((baselineA[0] + baselineA[2]) * 0.5, (baselineA[1] + baselineA[3]) * 0.5);
            cv::Point2f baselineBMidpoint = cv::Point2f((baselineB[0] + baselineB[2]) * 0.5, (baselineB[1] + baselineB[3]) * 0.5);
            
            float height = Geometry::euclideanDistance(baselineAMidpoint, baselineBMidpoint);
            
            float parallelism = Geometry::lineParallelism(baselineA, baselineB);
            
            int matches = 1;
            for (int i=0; i<plane.validLines.size(); i++) {
                const auto &line = plane.validLines[i];
                
                //cv::Point2f lineMidpoint = cv::Point2f((line[0] + line[2]) * 0.5, (line[1] + line[3]) * 0.5);
                
                float parallelismA = Geometry::lineParallelism(baselineA, line);
                float parallelismB = Geometry::lineParallelism(baselineB, line);
                
                if (parallelismA < 0.1) {
                    matches ++;
                }
                if (parallelismB < 0.1) {
                    matches ++;
                }
            }
            
            return sqrt(matches) * powf(height, 2.0) / (0.01 + parallelism);
        }
        
        //just demo:
        void _circle_ransac(const cv::Mat &image, std::vector<cv::Vec3f> &circles, double canny_threshold, double circle_threshold, int numIterations)
        {
            CV_Assert(image.type() == CV_8UC1 || image.type() == CV_8UC3);
            circles.clear();
            
            // Edge Detection
            cv::Mat edges;
            cv::Canny(image, edges, MAX(canny_threshold/2,1), canny_threshold, 3);
            
            // Create point set from Canny Output
            std::vector<cv::Point2d> points;
            for(int r = 0; r < edges.rows; r++)
            {
                for(int c = 0; c < edges.cols; c++)
                {
                    if(edges.at<unsigned char>(r,c) == 255)
                    {
                        points.push_back(cv::Point2d(c,r));
                    }
                }
            }
            
            // 4 point objects to hold the random samples
            cv::Point2d pointA;
            cv::Point2d pointB;
            cv::Point2d pointC;
            cv::Point2d pointD;
            
            // distances between points
            double AB;
            double BC;
            double CA;
            double DC;
            
            // varibales for line equations y = mx + b
            double m_AB;
            double b_AB;
            double m_BC;
            double b_BC;
            
            // varibles for line midpoints
            double XmidPoint_AB;
            double YmidPoint_AB;
            double XmidPoint_BC;
            double YmidPoint_BC;
            
            // variables for perpendicular bisectors
            double m2_AB;
            double m2_BC;
            double b2_AB;
            double b2_BC;
            
            // RANSAC
            cv::RNG rng;
            int min_point_separation = 10; // change to be relative to image size?
            int colinear_tolerance = 1; // make sure points are not on a line
            int radius_tolerance = 3; // change to be relative to image size?
            int points_threshold = 10; //should always be greater than 4
            //double min_circle_separation = 10; //reject a circle if it is too close to a previously found circle
            //double min_radius = 10.0; //minimum radius for a circle to not be rejected
            
            int x,y;
            cv::Point2d center;
            double radius;
            
            // Iterate
            for(int iteration = 0; iteration < numIterations; iteration++)
            {
                //std::cout << "RANSAC iteration: " << iteration << std::endl;
                
                // get 4 random points
                pointA = points[rng.uniform((int)0, (int)points.size())];
                pointB = points[rng.uniform((int)0, (int)points.size())];
                pointC = points[rng.uniform((int)0, (int)points.size())];
                pointD = points[rng.uniform((int)0, (int)points.size())];
                
                // calc lines
                AB = norm(pointA - pointB);
                BC = norm(pointB - pointC);
                CA = norm(pointC - pointA);
                DC = norm(pointD - pointC);
                
                // one or more random points are too close together
                if(AB < min_point_separation || BC < min_point_separation || CA < min_point_separation || DC < min_point_separation) continue;
                
                //find line equations for AB and BC
                //AB
                m_AB = (pointB.y - pointA.y) / (pointB.x - pointA.x + 0.000000001); //avoid divide by 0
                b_AB = pointB.y - m_AB*pointB.x;
                
                //BC
                m_BC = (pointC.y - pointB.y) / (pointC.x - pointB.x + 0.000000001); //avoid divide by 0
                b_BC = pointC.y - m_BC*pointC.x;
                
                
                //test colinearity (ie the points are not all on the same line)
                if(abs(pointC.y - (m_AB*pointC.x + b_AB + colinear_tolerance)) < colinear_tolerance) continue;
                
                //find perpendicular bisector
                //AB
                //midpoint
                XmidPoint_AB = (pointB.x + pointA.x) / 2.0;
                YmidPoint_AB = m_AB * XmidPoint_AB + b_AB;
                //perpendicular slope
                m2_AB = -1.0 / m_AB;
                //find b2
                b2_AB = YmidPoint_AB - m2_AB*XmidPoint_AB;
                
                //BC
                //midpoint
                XmidPoint_BC = (pointC.x + pointB.x) / 2.0;
                YmidPoint_BC = m_BC * XmidPoint_BC + b_BC;
                //perpendicular slope
                m2_BC = -1.0 / m_BC;
                //find b2
                b2_BC = YmidPoint_BC - m2_BC*XmidPoint_BC;
                
                //find intersection = circle center
                x = (b2_AB - b2_BC) / (m2_BC - m2_AB);
                y = m2_AB * x + b2_AB;
                center = cv::Point2d(x,y);
                radius = cv::norm(center - pointB);
                
                /// geometry debug image
                if (false)
                {
                    cv::Mat debug_image = edges.clone();
                    cv::cvtColor(debug_image, debug_image, CV_GRAY2RGB);
                    
                    cv::Scalar pink(255,0,255);
                    cv::Scalar blue(255,0,0);
                    cv::Scalar green(0,255,0);
                    cv::Scalar yellow(0,255,255);
                    cv::Scalar red(0,0,255);
                    
                    // the 3 points from which the circle is calculated in pink
                    cv::circle(debug_image, pointA, 3, pink);
                    cv::circle(debug_image, pointB, 3, pink);
                    cv::circle(debug_image, pointC, 3, pink);
                    
                    // the 2 lines (blue) and the perpendicular bisectors (green)
                    cv::line(debug_image,pointA,pointB,blue);
                    cv::line(debug_image,pointB,pointC,blue);
                    cv::line(debug_image,cv::Point(XmidPoint_AB,YmidPoint_AB),center,green);
                    cv::line(debug_image,cv::Point(XmidPoint_BC,YmidPoint_BC),center,green);
                    
                    cv::circle(debug_image, center, 3, yellow); // center
                    cv::circle(debug_image, center, radius, yellow);// circle
                    
                    // 4th point check
                    cv::circle(debug_image, pointD, 3, red);
                    
//                    cv::imshow("ransac debug", debug_image);
//                    waitKey(0);
                }
                
                //check if the 4 point is on the circle
                if(abs(cv::norm(pointD - center) - radius) > radius_tolerance) continue;
                
                // vote
                std::vector<int> votes;
                std::vector<int> no_votes;
                for(int i = 0; i < (int)points.size(); i++)
                {
                    double vote_radius = norm(points[i] - center);
                    
                    if(abs(vote_radius - radius) < radius_tolerance)
                    {
                        votes.push_back(i);
                    }
                    else
                    {
                        no_votes.push_back(i);
                    }
                }
                
                // check votes vs circle_threshold
                if( (float)votes.size() / (2.0*CV_PI*radius) >= circle_threshold )
                {
                    circles.push_back(cv::Vec3f(x,y,radius));
                    
                    // voting debug image
                    if (false)
                    {
                        cv::Mat debug_image2 = edges.clone();
                        cv::cvtColor(debug_image2, debug_image2, CV_GRAY2RGB);
                        
                        cv::Scalar yellow(0,255,255);
                        cv::Scalar green(0,255,0);
                        
                        cv::circle(debug_image2, center, 3, yellow); // center
                        cv::circle(debug_image2, center, radius, yellow);// circle
                        
                        // draw points that voted
                        for(int i = 0; i < (int)votes.size(); i++)
                        {
                            cv::circle(debug_image2, points[votes[i]], 1, green);
                        }
                        
//                        imshow("ransac debug", debug_image2);
//                        waitKey(0);
                    }
                    
                    // remove points from the set so they can't vote on multiple circles
                    std::vector<cv::Point2d> new_points;
                    for(int i = 0; i < (int)no_votes.size(); i++)
                    {
                        new_points.push_back(points[no_votes[i]]);
                    }
                    points.clear();
                    points = new_points;
                }
                
                // stop RANSAC if there are few points left
                if((int)points.size() < points_threshold)
                    break;
            }
            
            return;
        }

    };
    
    CBP_PlaneRansac::CBP_PlaneRansac(const cv::Size &workingSize,
                                     const std::vector<cv::Vec4f>&lines3d,
                                     std::vector<cv::Mat> &surfaces,
                                     std::vector<cv::Point2f> &directions,
                                     std::vector<cv::Scalar> &colors) {
        m_pImpl = std::unique_ptr<Impl>(new Impl(this, workingSize, lines3d, surfaces, directions, colors));
    }
    
    CBP_PlaneRansac::~CBP_PlaneRansac() {
        
    }
    
    void CBP_PlaneRansac::solve(int numIterations, cv::Mat &debug, std::vector<cv::Vec4f> &baselines) {
        m_pImpl->_solve(numIterations, debug, baselines);
    }
};
