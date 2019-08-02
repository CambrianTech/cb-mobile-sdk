//
//  Coloring.cpp
//
//  Created by Joel Teply on 6/14/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "Coloring.h"
#include "Imaging.h"
#include "ImageDefines.h"

using namespace std;

namespace imaging {
    
    cv::Scalar Coloring::_subtractColors(cv::Scalar colorA, cv::Scalar colorB, bool isHSV, cv::Scalar coefficient) {
        cv::Scalar distance;
        if (isHSV)
        {
            //scale is 0-180 for hue but circular so 0=180 and 1 is only 1 distance from 180
            
            //convert scale from -90 to 90
            double colorAHue = (colorA.val[0] < 90) ? colorA.val[0] : 180 - colorA.val[0];
            double colorBHue = (colorB.val[0] < 90) ? colorB.val[0] : 180 - colorB.val[0];
            
            //adjust scale to 0-255, like other values, so the color component has the same weight
            distance[0] = (255.0f / 180.0f) * (colorAHue - colorBHue);
        }
        else {
            distance[0] = colorA.val[0] - colorB.val[0];
        }
        
        distance[1] = colorA.val[1] - colorB.val[1];
        distance[2] = colorA.val[2] - colorB.val[2];
        
        distance = distance * coefficient;
        
        return distance;
    }
    
    double Coloring::euclideanDistance(cv::Scalar colorA, cv::Scalar colorB, bool isHSV, cv::Scalar coefficient)
    {
        return sqrt(euclideanDistanceSq(colorA, colorB, isHSV, coefficient));
    }
    
    double Coloring::euclideanDistanceSq(cv::Scalar colorA, cv::Scalar colorB, bool isHSV, cv::Scalar coefficient)
    {
        cv::Scalar distances = _subtractColors(colorA, colorB, isHSV, coefficient);
        
        return distances[0] * distances[0]
            + distances[1] * distances[1]
            + distances[2] * distances[2];
    }
    
    //http://www.compuphase.com/cmetric.htm
    double Coloring::perceptiveDistance(cv::Scalar colorA, cv::Scalar colorB) {
        long rmean = ( (long)colorA[0] + (long)colorB[0] ) / 2;
        long r = (long)colorA[0] - (long)colorB[0];
        long g = (long)colorA[1] - (long)colorB[1];
        long b = (long)colorA[2] - (long)colorB[2];
        return sqrt((((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*b*b)>>8));
    }
    
    ColorClass Coloring::getColorClassification(const cv::Scalar &color) {
        
        auto hslColor = rgbToHSL(color);
        
        return getColorClassification(hslColor[0], hslColor[1], hslColor[2]);
    }
    
    //Colors are in HSL, on an H/S coordinate system with range 0-255 and X=Saturation, Y=Luminance
    //Hue has a range 0-360
    //X = Saturation
    //Y = Luminance
    //colors are above or below the line depending on where they are on Photoshops HSL picker (aka less than or greater)
    //rise over run lol https://www.desmos.com/calculator (zoom out and drag to 255x255ish)
#define WHITES_SLOPE 0.5f
#define WHITES_YIntercept 235.0f
#define WHITES_YThreshold (WHITES_SLOPE * saturation + WHITES_YIntercept)
    
#define GREYS_SLOPE -3.16f
#define GREYS_YIntercept 255.0f
#define GREYS_YThreshold (GREYS_SLOPE * saturation + GREYS_YIntercept)
    
#define BLACKS_SLOPE -0.04f
#define BLACKS_YIntercept 60.0f
#define BLACKS_YThreshold (BLACKS_SLOPE * saturation + BLACKS_YIntercept)
    
#define REDS_END_HUE 10
#define ORANGE_END_HUE 40

#define BROWNS_START_HUE 18
#define BROWNS_END_HUE 38
#define BROWNS_TOP_SLOPE -0.33f
#define BROWNS_TOP_YIntercept 204.0f
#define BROWNS_TOP_YThreshold (BROWNS_TOP_SLOPE * saturation + BROWNS_TOP_YIntercept)
    
#define YELLOW_END_HUE 65
#define GREEN_END_HUE 169
#define BLUE_END_HUE 225
#define PURPLE_END_HUE 340

    
    //     Luminance = 255 (Y axis)
    //     |    /
    //     |Wh /
    //     |  /
    //     | \   HUES
    //     |  \    (Browns some hues)
    //     |greys ____
    //     |    \ blacks
    //     O---------------- Saturation = 255 (X axis)
    
    ColorClass Coloring::getColorClassification(double hue, double saturation, double luminance) {
        
        //IF statement priority matters. Blacks are excluded from hues and browns. Whites excluded from greys
        if (luminance >= WHITES_YThreshold) {
            return ColorClass_Whites;
        }
        else if (luminance < BLACKS_YThreshold) {
            return ColorClass_Blacks;
        }
        else if (luminance < GREYS_YThreshold) {
            return ColorClass_Greys;
        }
        else {
            if (hue > BROWNS_START_HUE && hue <= BROWNS_END_HUE && luminance < BROWNS_TOP_YThreshold) {
                return ColorClass_Browns;
            }
            else {
                if (hue > PURPLE_END_HUE || hue <= REDS_END_HUE) {
                    //340 to 20
                    return ColorClass_Reds;
                } else if (hue <= ORANGE_END_HUE) {
                    return ColorClass_Oranges;
                } else if (hue <= YELLOW_END_HUE) {
                    return ColorClass_Yellows;
                } else if (hue <= GREEN_END_HUE) {
                    return ColorClass_Greens;
                } else if (hue <= BLUE_END_HUE) {
                    return ColorClass_Blues;
                } else {
                    return ColorClass_Purples;
                }
            }
        }
    }
    
    double Coloring::manhattanDistance(cv::Scalar colorA, cv::Scalar colorB, bool isHSV, cv::Scalar coefficient)
    {
        return sqrt(manhattanDistanceSq(colorA, colorB, isHSV, coefficient));
    }
    
    double Coloring::manhattanDistanceSq(cv::Scalar colorA, cv::Scalar colorB, bool isHSV, cv::Scalar coefficient)
    {
        cv::Scalar distances = _subtractColors(colorA, colorB, isHSV, coefficient);
        
        return abs(distances[0]) + abs(distances[1]) + abs(distances[2]);
    }
    
    cv::Scalar Coloring::rgbToBGR(cv::Scalar rgbColor)
    {
        return cv::Scalar(rgbColor.val[2], rgbColor.val[1], rgbColor.val[0], rgbColor.val[3]);
    }
    
    cv::Scalar Coloring::bgrToRGB(cv::Scalar bgrColor)
    {
        return cv::Scalar(bgrColor.val[2], bgrColor.val[1], bgrColor.val[0], bgrColor.val[3]);
    }
    
    cv::Scalar Coloring::bgrToHSV(cv::Scalar bgrColor, bool isHSV255)
    {
        cv::Scalar rgbColor = bgrToRGB(bgrColor);
        
        return Coloring::rgbToHSV(rgbColor, isHSV255);
    }
    
    cv::Scalar Coloring::hsvToBGR(cv::Scalar hsv, bool isHSV255)
    {
        cv::Scalar rgbColor = hsvToRGB(hsv, isHSV255);
        
        return Coloring::rgbToBGR(rgbColor);
    }
    
    cv::Scalar Coloring::rgbToHSV(cv::Scalar rgb, bool isHSV255)
    {
        double r = rgb.val[0] / 255.0;
        double g = rgb.val[1] / 255.0;
        double b = rgb.val[2] / 255.0;
        
        double hue;     /* Hue degree between 0.0 and 360.0 */
        double sat;     /* Saturation between 0.0 (gray) and 1.0 */
        double val;     /* Value between 0.0 (black) and 1.0 */
        
        //<<find min and max RGB value>>=
        double rgb_min, rgb_max;
        
        rgb_min = MIN3(r, g, b);
        rgb_max = MAX3(r, g, b);
        
        //<<compute value>>=
        val = rgb_max;
        if (val == 0)
        {
            //hue = sat = 0;
            return cv::Scalar(0, 0, 0, rgb.val[3]);
        }
        
        double d = rgb_max - rgb_min;
    
        if (d == 0){
            hue = sat = 0; // achromatic
        } else {
            sat = d / rgb_max;
            
            if (rgb_max == r) hue = (g - b) / d + (g < b ? 6 : 0);
            else if (rgb_max == g) hue = (b - r) / d + 2;
            else hue = (r - g) / d + 4;
            
            hue /= 6.0f;
        }
        
        return cv::Scalar(hue * (isHSV255 ? 255.f : 180.f), 255.0f * sat, 255.0f * val, rgb.val[3]);
    }
    
    cv::Scalar Coloring::hsvToRGB(cv::Scalar hsv, bool isHSV255)
    {
        double H = hsv.val[0] / (isHSV255 ? 255.0 : 180.0f);         //0-180
        double S = hsv.val[1] / 255.0f;;        //0-255
        
        double V = hsv.val[2] / 255.0f;;        //0-255
        double R, G, B;
        if ( S == 0 )                           //HSV from 0 to 1
        {
            R = V * 255.0;
            G = V * 255.0;
            B = V * 255.0;
        }
        else
        {
            double var_h = H * 6;
            if ( var_h == 6 )
                var_h = 0;                  //H must be < 1
            double var_i = floor( var_h );  //Or ... var_i = floor( var_h )
            double var_1 = V * ( 1 - S );
            double var_2 = V * ( 1 - S * ( var_h - var_i ) );
            double var_3 = V * ( 1 - S * ( 1 - ( var_h - var_i ) ) );
            
            double var_r, var_g, var_b;
            if ( var_i == 0 )
            {
                var_r = V; var_g = var_3; var_b = var_1;
            }
            else if ( var_i == 1 )
            {
                var_r = var_2; var_g = V; var_b = var_1;
            }
            else if ( var_i == 2 )
            {
                var_r = var_1; var_g = V; var_b = var_3;
            }
            else if ( var_i == 3 )
            {
                var_r = var_1; var_g = var_2; var_b = V;
            }
            else if ( var_i == 4 )
            {
                var_r = var_3; var_g = var_1; var_b = V;
            }
            else
            {
                var_r = V; var_g = var_1; var_b = var_2;
            }
            
            R = var_r * 255.0;  //RGB results from 0 to 255
            G = var_g * 255.0;
            B = var_b * 255.0;
        }
        return cv::Scalar(R, G, B, hsv.val[3]);
    }
    
    cv::Scalar Coloring::rgbToHSL(cv::Scalar rgb)
    {
        double r = rgb.val[0] / 255.0;
        double g = rgb.val[1] / 255.0;
        double b = rgb.val[2] / 255.0;
        
//        double hue;     /* Hue degree between 0.0 and 360.0 */
//        double sat;     /* Saturation between 0.0 (gray) and 1.0 */
//        double lum;     /* Value between 0.0 (black) and 1.0 */
        
        double max = MAX3(r, g, b), min = MIN3(r, g, b);
        double h, s, l = (max + min) / 2;
        
        if (max == min){
            h = s = 0; // achromatic
        } else {
            double d = max - min;
            s = l > 0.5 ? d / (2 - max - min) : d / (max + min);
            
            if (max == r) h = (g - b) / d + (g < b ? 6 : 0);
            else if (max == g) h = (b - r) / d + 2;
            else h = (r - g) / d + 4;
            
            h /= 6.0f;
        }
        
        return cv::Scalar(h * 180, s * 255.0f, l * 255.0f, rgb.val[3]);
    }
    
    double hue2rgb(double p, double q, double t) {
        if(t < 0) t += 1;
        if(t > 1) t -= 1;
        if(t < 1/6) return p + (q - p) * 6 * t;
        if(t < 1/2) return q;
        if(t < 2/3) return p + (q - p) * (2/3 - t) * 6;
        return p;
    }
    
    cv::Scalar Coloring::hslToRGB(cv::Scalar hsl) {
        double r,g,b;
        
        double h = hsl.val[0] / 180.0f;
        double s = hsl.val[1] / 255.0f;
        double l = hsl.val[2] / 255.0f;
        
        if(s == 0){
            r = g = b = l; // achromatic
        }else{
            double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
            double p = 2 * l - q;
            r = hue2rgb(p, q, h + 1/3);
            g = hue2rgb(p, q, h);
            b = hue2rgb(p, q, h - 1/3);
        }
     
        cv::Scalar result = cv::Scalar(r * 255.0f, g * 255.0f, b * 255.0f);
        LIMIT_SCALAR(result);
        return result;
    }
    
    cv::Scalar Coloring::rotateRGB(cv::Scalar rgb, double degrees)
    {
//    Rotate:147.00,101.00,144.00 by 140.000000 degrees: 129.47,147.00,101.00
        
//        Original Color:(147.000000,101.000000,144.000000) 214.526860, rotated 160.000000 degrees is 72.860194
        
        cv::Scalar hsv = rgbToHSV(rgb, true);
        cv::Scalar newHSV = rotateHSV(hsv, degrees, true);
        
//        printf("Original Color:(%f,%f,%f) %f, rotated %f degrees is %f\n",
//                   rgb.val[0], rgb.val[1], rgb.val[2],
//                   hsv.val[0], degrees, newHSV.val[0]);
        
        cv::Scalar rgbResult = hsvToRGB(newHSV, true);
        return rgbResult;
    }
    
    cv::Scalar Coloring::rotateHSV(const cv::Scalar hsv, double degrees, bool isHSV255)
    {
        cv::Scalar hsvResult = hsv;
        double max = isHSV255 ? 255.0f : 180.0f;
        double percentOf360 = degrees / 360.0f;
        hsvResult.val[0] = hsvResult.val[0] + percentOf360 * max;
        if (hsvResult.val[0] > max)
            hsvResult.val[0] = hsvResult.val[0] - max;
        else if (hsv.val[0] < 0.0f)
            hsvResult.val[0] = fabs(hsvResult.val[0]) + max;
        return hsvResult;
    }
    
    //http://www.tigercolor.com/color-lab/color-theory/color-harmonies.htm
    //formulas: http://www.easyrgb.com/index.php?X=WEEL
    std::vector<cv::Scalar> Coloring::triadicColors(cv::Scalar rgb)
    {
        vector<cv::Scalar> values;
        
        values.push_back(rotateRGB(rgb, 120));
        values.push_back(rotateRGB(rgb, -120));
        
        return values;
    }
    
    std::vector<cv::Scalar> Coloring::analogousColors(cv::Scalar rgb)
    {
        vector<cv::Scalar> values;
        
        values.push_back(rotateRGB(rgb, 30));
        values.push_back(rotateRGB(rgb, -30));
        
        return values;
    }
    
    std::vector<cv::Scalar> Coloring::complementary(cv::Scalar rgb, int imageCount, double angleRange)
    {
        vector<cv::Scalar> values;
        
        double startAngle = 180.0f - (angleRange / 2.0f);
        double angleIncrement = angleRange / (imageCount - 1);
        for (int i=0; i<imageCount; i++) {
            
            double angle = startAngle + angleIncrement * i;
            
            cv::Scalar result = rotateRGB(rgb, angle);
            values.push_back(result);
            
//            printf("Rotate:%.2f,%.2f,%.2f by %f degrees: %.2f,%.2f,%.2f\n",
//                   rgb[0], rgb[1], rgb[2],
//                   angle,
//                   result[0], result[1], result[2]);
        }
        return values;
    }
    
    std::vector<cv::Scalar> Coloring::sampleColors(cv::Mat img, int xSampleCount, int ySampleCount)
    {
        cv::Size segmentSize = cv::Size(img.cols / xSampleCount, img.rows / ySampleCount);
        
        std::vector<cv::Scalar> colors;
        
        for (int row = 0; row < ySampleCount; row++)
        {
            for (int col = 0; col < xSampleCount; col++)
            {
                cv::Point point = cv::Point((col + 0.5) * segmentSize.width, (row + 0.5) * segmentSize.height);
                cv::Scalar avg = Imaging::meanAtPoint(img, point);
                colors.push_back(avg);
            }
        }
        
        return colors;
    }
    
    static int closestMatch(std::vector<cv::Scalar> colors, cv::Scalar forColor, int indexToIgnore, double *distance)
    {
        double minDistance = 100000;
        int closestMatchIndex = 0;
        
        const size_t colorsSize = colors.size();
        const cv::Scalar* colorsPtr = (colorsSize != 0) ? &colors.front() : NULL;
        
        for (int i = 0; i < colorsSize; i++)
        {
            if (i == indexToIgnore)
                continue;
            
            cv::Scalar toColor = colorsPtr[i];
            bool isBlank = toColor.val[0] == 0 && toColor.val[1] && toColor.val[2] == 0;
            
            if (!isBlank)
            {
                
                double distance = Coloring::euclideanDistanceSq(forColor, toColor, true);
                if (distance < minDistance)
                {
                    minDistance = distance;
                    closestMatchIndex = i;
                }
            }
        }
        if (distance)
            *distance = minDistance;
        
        return closestMatchIndex;
    }
    
    
    static int leastUniqueColor(std::vector<cv::Scalar> colors, int *similarIndex)
    {
        double minDistance = 100000;
        int closestMatchIndex = 0;
        
        const size_t colorsSize = colors.size();
        const cv::Scalar* colorsPtr = (colorsSize != 0) ? &colors.front() : NULL;
        
        for (int i = 0; i < colorsSize; i++)
        {
            cv::Scalar color = colorsPtr[i];
            
            //black or white
            if (color.val[2] <= 50 || (color.val[1] <= 15 && color.val[2] <= 230))
                return i;
            
            double distance;
            
            int matchIndex = closestMatch(colors, color, i, &distance);
            
            if (distance < minDistance)
            {
                minDistance = distance;
                closestMatchIndex = i;
                *similarIndex = matchIndex;
            }
        }
        
        return closestMatchIndex;
    }

    
    float score(const cv::Scalar &hsvColor, const cv::Scalar &rgbColor, int pixelArea, const std::vector<cv::Scalar> &existingColors) {
        
        double distanceScore = 1.0;
        
        //get mean and standard deviation of distance from the other colors so far
        if (existingColors.size() > 0) {
            std::vector<cv::Scalar> distances;
            for (const auto &color : existingColors) {
                distances.push_back(Coloring::perceptiveDistance(color, rgbColor));
            }
            
            cv::Scalar mean, stddev;
            cv::meanStdDev(distances, mean, stddev);
            distanceScore = mean[0] + stddev[0];
        }
        
        return powf(hsvColor[1], 1.9) //saturation
            * powf(hsvColor[2], 2.5) //intensity
            * powf(distanceScore, 3.0)
            * pixelArea; //pixel area
    }
    
    std::vector<cv::Scalar> Coloring::getMostCommonColors(const cv::Mat &rgb, cv::Mat &reducedRGB, int count)
    {
        std::vector<cv::Point> scaledPositions(count);
        
        double scale;
        if (reducedRGB.empty()) {
            scale = fmax(100.0f / double(rgb.cols), 100.0f / double(rgb.rows));
            cv::resize(rgb, reducedRGB, cv::Size(scale * double(rgb.cols), scale * double(rgb.rows)), cv::INTER_CUBIC);
        } else {
            scale = double(reducedRGB.cols) / double(rgb.cols);
        }
        
        std::vector<cv::Mat> imgRGB;
        cv::split(reducedRGB,imgRGB);
        
        int n = reducedRGB.rows *reducedRGB.cols;
        cv::Mat img3xN(n,3,CV_8U);
        for(int i=0;i<3;++i) {
            imgRGB[i].reshape(1,n).copyTo(img3xN.col(i));
        }
        img3xN.convertTo(img3xN,CV_32F);
        cv::Mat bestLabels, centers;
        
        int K = count * 3;
        cv::kmeans(img3xN, K, bestLabels, cv::TermCriteria(),5,cv::KMEANS_RANDOM_CENTERS, centers);
        
        std::vector<int> counts(K);
        for (int j=0; j<bestLabels.rows; j++) {
            int label = bestLabels.data[j];
            counts[label] ++;
        }
        
        std::vector<cv::Scalar> bestColors;
        std::vector<cv::Scalar> colors(K);
        std::vector<cv::Scalar> hsvColors(K);
        std::vector<bool> takenColors(K);
        
        cv::Scalar color = cv::Scalar::all(255);
        for (int i=0; i<K; i++) {
            takenColors[i] = 0;
            
            color[0]=centers.at<float>(i, 0);
            color[1]=centers.at<float>(i, 1);
            color[2]=centers.at<float>(i, 2);
            colors[i] = color;
            hsvColors[i] = Coloring::rgbToHSV(color);
        }
        
        double bestScore;
        int bestIndex = 0;
        for (int i=0; bestColors.size()<count; i++) {
            bestScore = score(hsvColors[i], colors[i], counts[i], bestColors);
            bestIndex = i;
            
            for (int j=0; j<K; j++) {
                auto thisScore = score(hsvColors[j], color[i], counts[j], colors);
                if (!takenColors[j] && thisScore > bestScore) {
                    bestScore = thisScore;
                    bestIndex = j;
                }
            }
            
            takenColors[bestIndex] = true;
            bestColors.push_back(colors[bestIndex]);
        }
        
        //bestLabels = bestLabels.reshape(0,reducedSRC.rows);
        
        return bestColors;
    }
    
    std::vector<cv::Point> Coloring::getColorPositions(const cv::Mat &rgb, cv::Mat &reducedRGB, const std::vector<cv::Scalar> &colors, cv::Scalar coefficient)
    {
        std::vector<cv::Point> scaledPositions(colors.size());
        
        double scale;
        if (reducedRGB.empty()) {
            scale = fmax(100.0f / double(rgb.cols), 100.0f / double(rgb.rows));
            cv::resize(rgb, reducedRGB, cv::Size(scale * double(rgb.cols), scale * double(rgb.rows)), cv::INTER_AREA);
        } else {
            scale = double(reducedRGB.cols) / double(rgb.cols);
        }
        
        int offset = int(1.0f / scale);
        
        for (int i = 0; i < colors.size(); i++) {
            const auto &color = colors[i];
            
            double closestDistance = INT_MAX;
            for (int y = 0; y < reducedRGB.rows; y++) {
                for (int x = 0; x < reducedRGB.cols; x++) {
                    auto position = cv::Point(x,y);
                    
                    bool positionTaken = false;
                    //make sure not taken, by checking previous items
                    for (int j=0; j<i; j++) {
                        if (scaledPositions[j] == position) {
                            positionTaken = true;
                            break;
                        }
                    }
                    if (positionTaken) continue;
                    
                    const cv::Scalar imageColor = reducedRGB.at<cv::Vec3b>(position);
                    
                    double distance = perceptiveDistance(color, imageColor);
                    if (distance < closestDistance) {
                        closestDistance = distance;
                        scaledPositions[i] = position;
                    }
                }
            }
        }
        
        std::vector<cv::Point> positions(colors.size());
        
        for (int i = 0; i < scaledPositions.size(); i++) {
            positions[i] = cv::Point(offset + double(scaledPositions[i].x) / scale,
                                     offset + double(scaledPositions[i].y) / scale);
        }
        
        return positions;
    }
    
    std::vector<cv::Scalar> Coloring::mostCommonHSVColors(const cv::Mat &imgHsv, int numColors, bool sample,
                                                          cv::Scalar lowerBound, cv::Scalar upperBound,
                                                          bool boostBrights)
    {
        int sampleSize = 4 * numColors;
        
        int channels[3] = { 0, 1, 2 };
        
        // Build and fill the histogram
        int h_bins = 3 * numColors;
        int s_bins = numColors;
        int v_bins = numColors;
        int hist_size[] = { h_bins, s_bins, v_bins };
        
        //hue range from 0-180
        float h_ranges[] = { float(lowerBound.val[0]), float(upperBound.val[0]) };
        //saturation range from 0-255
        float s_ranges[] = { float(lowerBound.val[1]),  float(upperBound.val[1]) };
        //value range from 0-255
        float v_ranges[] = { float(lowerBound.val[2]), float(upperBound.val[2]) };
        const float* ranges[3] = { h_ranges, s_ranges, v_ranges };
        
        cv::MatND hist;
        
        // Compute histogram
        cv::calcHist(&imgHsv,
                     1,                 // histogram of 1 image only
                     channels,          // the channel used
                     cv::noArray(),     // binary mask
                     hist,              // the resulting histogram
                     3,                 // it is a 3D histogram
                     hist_size,         // number of bins
                     ranges             // pixel value range
                     );
        
        //cv::normalize(hist, hist, 0, 255, CV_MINMAX);
        
        
        //    printf("Getting colors from %i,%i,%i to %i,%i,%i\n",
        //           (int)lowerBound.val[0], (int)lowerBound.val[1], (int)lowerBound.val[2],
        //           (int)upperBound.val[0], (int)upperBound.val[1], (int)upperBound.val[2]);
        
        //compute the most used colors
        float topMagnitudes[sampleSize];
        double topHueValues[sampleSize];
        double topSatValues[sampleSize];
        double topValValues[sampleSize];
        
        for (int i = 0; i < sampleSize; i++) {
            topMagnitudes[i] = 0;
            topHueValues[sampleSize] = 0;
            topSatValues[sampleSize] = 0;
            topValValues[sampleSize] = 0;
        }
        
        double hueRange = upperBound.val[0] - lowerBound.val[0];
        double satRange = upperBound.val[1] - lowerBound.val[1];
        double valRange = upperBound.val[2] - lowerBound.val[2];
        
        //find the top sampleSize colors
        for (int h = 0; h < h_bins; h++ )
        {
            for ( int s = 0; s < s_bins; s++ )
            {
                for ( int v = 0; v < v_bins; v++ )
                {
                    float bin_val = hist.at<float>(h, s, v);
                    
                    double hue = lowerBound.val[0] + hueRange * (double(h) / double(h_bins));
                    double sat = lowerBound.val[1] + satRange * (double(s) / double(s_bins));
                    double val = lowerBound.val[2] + valRange * (double(v) / double(v_bins));
                    
                    //printf("value: %i: %f\n", (int)hue, bin_val);
                    for ( int i = 0; i < sampleSize; i++ )
                    {
                        double score = bin_val;
                        
                        if (boostBrights) {
                            if (sat > 220 && val > 100)
                                score *= pow(sat, 10);
                            else if (sat > 150 && val > 80)
                                score *= pow(sat, 4);
                        }
                        
                        if (bin_val > topMagnitudes[i])
                        {
                            //scoot the remaing down
                            for (int j = sampleSize - 1; j > i; j--)
                            {
                                topMagnitudes[j] = topMagnitudes[j - 1];
                                topHueValues[j] = topHueValues[j - 1];
                                topSatValues[j] = topSatValues[j - 1];
                                topValValues[j] = topValValues[j - 1];
                            }
                            topMagnitudes[i] = score;
                            topHueValues[i] = hue;
                            topSatValues[i] = sat;
                            topValValues[i] = val;
                            break;
                        }
                    }
                }
            }
        }
        
        vector<cv::Scalar> sampledColors = sampleColors(imgHsv);
        
        //make all the colors
        vector<cv::Scalar> colors;
        
        for (int i = 0; i < sampleSize; i++ )
        {
            double hValue = topHueValues[i];
            double sValue = topSatValues[i];
            double vValue = topValValues[i];
            //double mag = topMagnitudes[i];
            
            //printf("Color: (%i,%i,%i) %f\n", (int)hValue, (int)sValue, (int)vValue, mag);
            
            cv::Scalar hsvColor = cv::Scalar(hValue, sValue, vValue, 255);
            
            double distance = 0;
            
            int matchIndex = -1;
            
            if (sample)
                matchIndex = closestMatch(sampledColors, hsvColor, -1, &distance);
            
            if (matchIndex >= 0 && distance < 60)
                //printf("Using sampled color with distance %f\n", distance);
                colors.push_back(sampledColors[matchIndex]);
            else
                colors.push_back(hsvColor);
            
        }
        
        //Culling down to numColors by averaging, give up at sampleSize attempts
        
        for (int i = 0; colors.size() > numColors && i < sampleSize; i++)
        {
            int closestMatchIndex = -1;
            int colorIndex = leastUniqueColor(colors, &closestMatchIndex);
            
            colors.erase(colors.begin() + colorIndex);
        }
        
        bool printout = false;
        if (printout)
        {
            const size_t colorsSize = colors.size();
            const cv::Scalar* colorsPtr = (colorsSize != 0) ? &colors.front() : NULL;
            
            printf("Found %zu HSV colors:", colorsSize);
            for (int i = 0; i < colorsSize; i++)
            {
                cv::Scalar color = colorsPtr[i];
                cv::Scalar rgb = hsvToRGB(color);
                printf(" (%i,%i,%i):(%i,%i,%i)",
                       (int)color.val[0], (int)color.val[1], (int)color.val[2],
                       (int)rgb.val[0], (int)rgb.val[1], (int)rgb.val[2]);
            }
            printf("\n");
        }
        
        return colors;
    }
    
    static bool seededColor = false;
    cv::Scalar Coloring::randomColor(int minValue, int maxValue) {
        
        if (!seededColor) {
            srand ((int)time(NULL) );
            seededColor = true;
        }
        
        cv::Scalar color;
        
        int range = maxValue - minValue;
        color[0] = double(minValue + rand() % range);
        color[1] = double(minValue + rand() % range);
        color[2] = double(minValue + rand() % range);
        
        return color;
    }
}
